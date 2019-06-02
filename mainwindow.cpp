#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QProcess"
#include "QDebug"
#include "QTimer"
#include "QThread"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sourceMDLpath_btn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "", ui->sourceMDL_loc->text(),
                                                    "BLEND Model Files (*.blend);;"
                                                    "DAE Model Files (*.dae);;"
                                                    "FBX Model Files (*.fbx);;"
                                                    "OBJ Model Files (*.obj);;"
                                                    "All Supported Model Files (*.blend *.dae *.fbx *.obj);;"
                                                    "All files (*)");
    if(fileName == "")
        return;

    ui->sourceMDL_loc->setText(fileName);
}

void MainWindow::on_saveNSBMDpath_btn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "", ui->sourceMDL_loc->text().split('.')[0].append(".nsbmd"), "NSBMD Model Files (*.nsbmd);;All files (*)");
    if(fileName == "")
        return;

    ui->saveNSBMD_loc->setText(fileName);
}

void MainWindow::readErrorsToCmd()
{
    QProcess *myProcess = qobject_cast<QProcess*>(sender());

    QString line;
    do
    {
        line = myProcess->readLine();
        if (line.toLower().contains("error"))
            ui->cmd_out->appendPlainText(line.trimmed() + "!");
    } while (!line.isNull());
}

static QString srcFBXpath;
static QString destIMDpath;
static QString destNSBMDpath;
static QTimer* progressBarTimer;
static int progressBarIncrementMax;

void MainWindow::doProgressIncrementation()
{
    if(ui->progressBar->value() < progressBarIncrementMax)
        ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void MainWindow::on_convert_btn_clicked() //Convert Part 1
{
    ui->cmd_out->clear();

    while(ui->progressBar->value() != 0)
    {
        QThread::msleep(1);
        ui->progressBar->setValue(ui->progressBar->value() - 1);
    }

    srcFBXpath = "\"" + ui->sourceMDL_loc->text() + "\"";
    destIMDpath = "\"" + QApplication::applicationDirPath() + "/ass2imd/temp.imd\"";
    destNSBMDpath = "\"" + ui->saveNSBMD_loc->text() + "\"";

    ui->cmd_out->appendPlainText("Status: Generating IMD file, please wait...");

    QProcess *myProcess = new QProcess(this);
    myProcess->setProgram(QApplication::applicationDirPath() + "/ass2imd/AssToImd");
    myProcess->setNativeArguments(srcFBXpath + " -o " + destIMDpath);
    myProcess->start();
    connect(myProcess, SIGNAL(readyRead()), this, SLOT(readErrorsToCmd()));
    connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(doIMD2NSBMD()));

    progressBarTimer = new QTimer();
    progressBarTimer->setSingleShot(false);
    progressBarTimer->start(500);
    progressBarIncrementMax = 75;
    connect(progressBarTimer, &QTimer::timeout, this, &MainWindow::doProgressIncrementation);
}

void MainWindow::doIMD2NSBMD() //Convert Part 2
{
    progressBarTimer->stop();

    bool imdExists = QFileInfo::exists(destIMDpath.remove('"')) && QFileInfo(destIMDpath.remove('"')).isFile();
    if(!imdExists) {
        ui->cmd_out->appendPlainText("Status: IMD file generation failed!");
        return;
    }
    else {
        ui->cmd_out->appendPlainText("Status: IMD file generation succeeded!");
        ui->cmd_out->appendPlainText("Status: Converting IMD file to NSBMD!");
    }

    ui->progressBar->setValue(75);

    QProcess *g3dcvtr = new QProcess(this);
    g3dcvtr->setProgram(QApplication::applicationDirPath() + "/imd2nsbmd/g3dcvtr");
    g3dcvtr->setNativeArguments(destIMDpath + " -o " + destNSBMDpath);
    g3dcvtr->start();
    //connect(g3dcvtr, SIGNAL(readyRead()), this, SLOT(readErrorsToCmd()));
    connect(g3dcvtr, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishConvertion()));

    progressBarIncrementMax = 100;
    progressBarTimer->start(500);
    connect(progressBarTimer, &QTimer::timeout, this, &MainWindow::doProgressIncrementation);
}

void MainWindow::finishConvertion() //Convert Part 3
{
    bool btx0Exists = QFileInfo::exists(destNSBMDpath.remove('"')) && QFileInfo(destNSBMDpath.remove('"')).isFile();
    if(!btx0Exists) {
        ui->cmd_out->appendPlainText("Status: IMD to NSBMD file convertion failed!");
        return;
    }
    else {
        ui->cmd_out->appendPlainText("Status: IMD to NSBMD file convertion succeeded!");
    }

    ui->progressBar->setValue(100);
}
