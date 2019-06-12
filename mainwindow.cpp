#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QProcess"
#include "QDebug"
#include "QTimer"
#include "QThread"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sourceMDLpath_btn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "", ui->sourceMDL_loc->text(),
                                                    "All Supported Model Files (*.blend *.dae *.fbx *.obj);;"
                                                    "BLEND Model Files (*.blend);;"
                                                    "DAE Model Files (*.dae);;"
                                                    "FBX Model Files (*.fbx);;"
                                                    "OBJ Model Files (*.obj);;"
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

void MainWindow::nsbmd_readErrorsToCmd()
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
    destIMDpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/temp.imd\"";
    destNSBMDpath = "\"" + ui->saveNSBMD_loc->text() + "\"";

    ui->cmd_out->appendPlainText("Status: Generating IMD file, please wait...");

    QProcess *ass2imd = new QProcess(this);
    ass2imd->setProgram(QApplication::applicationDirPath() + "/bin/ass2imd/AssToImd");
    ass2imd->setNativeArguments(srcFBXpath + " -o " + destIMDpath);
    ass2imd->start();
    if(ass2imd->state() == QProcess::NotRunning)
    {
        ui->nsbtx_cmd_out->appendPlainText("Error: Could not start \"./bin/ass2imd/AssToImd\"");
    }
    connect(ass2imd, SIGNAL(readyRead()), this, SLOT(nsbmd_readErrorsToCmd()));
    connect(ass2imd, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(doIMD2NSBMD()));

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
    g3dcvtr->setProgram(QApplication::applicationDirPath() + "/bin/imd2nsbmd/g3dcvtr");
    g3dcvtr->setNativeArguments(destIMDpath + " -o " + destNSBMDpath);
    g3dcvtr->start();
    if(g3dcvtr->state() == QProcess::NotRunning)
    {
        ui->nsbtx_cmd_out->appendPlainText("Error: Could not start \"./bin/imd2nsbmd/g3dcvtr\"");
    }
    connect(g3dcvtr, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nsbmd_finishConvertion()));

    progressBarIncrementMax = 100;
    progressBarTimer->start(500);
    connect(progressBarTimer, &QTimer::timeout, this, &MainWindow::doProgressIncrementation);
}

void MainWindow::nsbmd_finishConvertion() //Convert Part 3
{
    bool bmd0Exists = QFileInfo::exists(destNSBMDpath.remove('"')) && QFileInfo(destNSBMDpath.remove('"')).isFile();
    if(!bmd0Exists) {
        ui->cmd_out->appendPlainText("Status: IMD to NSBMD file convertion failed!");
        return;
    }
    else {
        ui->cmd_out->appendPlainText("Status: IMD to NSBMD file convertion succeeded!");
    }

    ui->progressBar->setValue(100);
}

#define NSBTX_START {
void MainWindow::nsbtx_readErrorsToCmd()
{
    QProcess *myProcess = qobject_cast<QProcess*>(sender());

    QString line;
    do
    {
        line = myProcess->readLine();
        if (line.toLower().contains("error"))
            ui->nsbtx_cmd_out->appendPlainText(line.trimmed() + "!");
    } while (!line.isNull());
}

static QString nsbtx_srcTEXpath;
static QString nsbtx_srcMDLpath;
static QString nsbtx_genMDLpath;
static QString nsbtx_destIMDpath;
static QString nsbtx_genNSBMDpath;
static QString nsbtx_destNSBTXpath;

void MainWindow::on_nsbtx_convert_btn_clicked()
{
    //Clear CMD output
    ui->nsbtx_cmd_out->clear();

    //Setup paths
    nsbtx_srcTEXpath = "\"" + ui->nsbtx_image_source->text() + "\"";
    nsbtx_srcMDLpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/nsbtx.dae\"";
    nsbtx_genMDLpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/nsbtx2.dae\"";
    nsbtx_destIMDpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/nsbtx.imd\"";
    nsbtx_genNSBMDpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/nsbtx.nsbmd\"";
    nsbtx_destNSBTXpath = "\"" + ui->nsbtx_save_path->text() + "\"";

    //Replace "texture.png" in DAE with path to actual texture
    QByteArray fileData;
    QFile file(nsbtx_srcMDLpath.remove('"'));
    file.open(QIODevice::ReadOnly);
    fileData = file.readAll();
    file.close();

    QString text(fileData);
    text.replace(QString("texture.png"), QString(nsbtx_srcTEXpath.remove('"')));

    QFile file2(nsbtx_genMDLpath.remove('"'));
    file2.open(QIODevice::WriteOnly);
    file2.write(text.toUtf8());
    file2.close();

    //Start converting to IMD
    ui->nsbtx_cmd_out->appendPlainText("Status: Generating IMD file, please wait...");

    QProcess *ass2imd = new QProcess(this);
    ass2imd->setProgram(QApplication::applicationDirPath() + "/bin/ass2imd/AssToImd");
    ass2imd->setNativeArguments(nsbtx_genMDLpath + " -o " + nsbtx_destIMDpath);
    ass2imd->start();
    if(ass2imd->state() == QProcess::NotRunning)
    {
        ui->nsbtx_cmd_out->appendPlainText("Error: Could not start \"./bin/ass2imd/AssToImd\"");
    }
    connect(ass2imd, SIGNAL(readyRead()), this, SLOT(nsbtx_readErrorsToCmd()));
    connect(ass2imd, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nsbtx_imd_process_end()));
}

void MainWindow::nsbtx_imd_process_end()
{
    bool imdExists = QFileInfo::exists(nsbtx_destIMDpath.remove('"')) && QFileInfo(nsbtx_destIMDpath.remove('"')).isFile();
    if(!imdExists) {
        ui->nsbtx_cmd_out->appendPlainText("Status: IMD file generation failed!");
        return;
    }
    else {
        ui->nsbtx_cmd_out->appendPlainText("Status: IMD file generation succeeded!");
        ui->nsbtx_cmd_out->appendPlainText("Status: Converting IMD file to NSBMD!");
    }

    QProcess *g3dcvtr = new QProcess(this);
    g3dcvtr->setProgram(QApplication::applicationDirPath() + "/bin/imd2nsbmd/g3dcvtr");
    g3dcvtr->setNativeArguments(nsbtx_destIMDpath + " -o " + nsbtx_genNSBMDpath);
    g3dcvtr->start();
    if(g3dcvtr->state() == QProcess::NotRunning)
    {
        ui->nsbtx_cmd_out->appendPlainText("Error: Could not start \"./bin/imd2nsbmd/g3dcvtr\"");
    }
    connect(g3dcvtr, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nsbtx_g3dcvtr_process_end()));
}

void MainWindow::nsbtx_g3dcvtr_process_end()
{
    bool nsbmdExists = QFileInfo::exists(nsbtx_genNSBMDpath.remove('"')) && QFileInfo(nsbtx_genNSBMDpath.remove('"')).isFile();
    if(!nsbmdExists) {
        ui->nsbtx_cmd_out->appendPlainText("Status: IMD to NSBMD file convertion failed!");
        return;
    }
    else {
        ui->nsbtx_cmd_out->appendPlainText("Status: IMD to NSBMD file convertion succeeded!");
        ui->nsbtx_cmd_out->appendPlainText("Status: Converting NSBMD file to NSBTX!");
    }

    //Convert to NSBTX
    QByteArray nsbmdData;
    QFile nsbmdFile(nsbtx_genNSBMDpath.remove('"'));
    nsbmdFile.open(QIODevice::ReadOnly);
    nsbmdData = nsbmdFile.readAll();

    nsbmdData.replace(0, 4, "BTX0");

    int MLD0_blockOffset = 0;
    for (int i = 0; i < nsbmdData.size(); i++)
    {
        if(nsbmdData.at(i) == 0x4D && nsbmdData.at(i+1) == 0x44 && nsbmdData.at(i+2) == 0x4C && nsbmdData.at(i+3) == 0x30)
        {
            MLD0_blockOffset = i;
        }
    }

    int TEX0_blockOffset = 0;
    for (int i = 0; i < nsbmdData.size(); i++)
    {
        if(nsbmdData.at(i) == 0x54 && nsbmdData.at(i+1) == 0x45 && nsbmdData.at(i+2) == 0x58 && nsbmdData.at(i+3) == 0x30)
        {
            TEX0_blockOffset = i;
        }
    }

    nsbmdData.remove(MLD0_blockOffset, TEX0_blockOffset - MLD0_blockOffset);

    //Save to NSBTX file
    QFile nsbtxFile(nsbtx_destNSBTXpath.remove('"'));
    nsbtxFile.open(QIODevice::WriteOnly);
    nsbtxFile.write(nsbmdData);
    nsbtxFile.close();

    bool nsbtxExists = QFileInfo::exists(nsbtx_destNSBTXpath.remove('"')) && QFileInfo(nsbtx_destNSBTXpath.remove('"')).isFile();
    if(!nsbtxExists) {
        ui->nsbtx_cmd_out->appendPlainText("Status: NSBMD to NSBTX file convertion failed!");
        return;
    }
    else {
        ui->nsbtx_cmd_out->appendPlainText("Status: NSBMD to NSBTX file convertion succeeded!");
    }
}

void MainWindow::on_nsbtx_image_source_select_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "", ui->nsbtx_image_source->text(),
                                                    "All Supported Image Formats (*.png *.tga);;"
                                                    "PNG Files (*.png);;"
                                                    "TGA Files (*.tga);;"
                                                    "All files (*)");
    if(fileName == "")
        return;

    ui->nsbtx_image_source->setText(fileName);
}

void MainWindow::on_nsbtx_save_path_select_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "", ui->nsbtx_image_source->text().split('.')[0].append(".nsbtx"), "NSBTX Image Files (*.nsbtx);;All files (*)");
    if(fileName == "")
        return;

    ui->nsbtx_save_path->setText(fileName);
}
#define NSBTX_END }
