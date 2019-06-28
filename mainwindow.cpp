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

#define NSBMD_START {
void MainWindow::on_nsbmd_sourceMDLpath_btn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "", ui->nsbmd_sourceMDL_loc->text(),
                                                    "All Supported Model Files (*.blend *.dae *.fbx *.obj);;"
                                                    "BLEND Model Files (*.blend);;"
                                                    "DAE Model Files (*.dae);;"
                                                    "FBX Model Files (*.fbx);;"
                                                    "OBJ Model Files (*.obj);;"
                                                    "All files (*)");
    if(fileName == "")
        return;

    ui->nsbmd_sourceMDL_loc->setText(fileName);
}

void MainWindow::on_nsbmd_saveNSBMDpath_btn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "", ui->nsbmd_sourceMDL_loc->text().split('.')[0].append(".nsbmd"), "NSBMD Model Files (*.nsbmd);;All files (*)");
    if(fileName == "")
        return;

    ui->nsbmd_saveNSBMD_loc->setText(fileName);
}

void MainWindow::nsbmd_readErrorsToCmd()
{
    QProcess *myProcess = qobject_cast<QProcess*>(sender());

    QString line;
    do
    {
        line = myProcess->readLine();
        if (line.toLower().contains("error:") || line.toLower().contains("warning:"))
            ui->nsbmd_cmd_out->appendPlainText(line.trimmed() + "!");
    } while (!line.isNull());
}

static QString nsbmd_srcFBXpath;
static QString nsbmd_genIMDpath;
static QString nsbmd_destNSBMDpath;
static QTimer* nsbmd_progressBarTimer;
static int nsbmd_progressBarIncrementMax;

void MainWindow::nsbmd_doProgressIncrementation()
{
    if(ui->nsbmd_progressBar->value() < nsbmd_progressBarIncrementMax)
        ui->nsbmd_progressBar->setValue(ui->nsbmd_progressBar->value() + 1);
}

void MainWindow::on_nsbmd_convert_btn_clicked() //Convert Part 1
{
    ui->nsbmd_cmd_out->clear();

    while(ui->nsbmd_progressBar->value() != 0)
    {
        QThread::msleep(1);
        ui->nsbmd_progressBar->setValue(ui->nsbmd_progressBar->value() - 1);
    }

    nsbmd_srcFBXpath = "\"" + ui->nsbmd_sourceMDL_loc->text() + "\"";
    nsbmd_genIMDpath = "\"" + QApplication::applicationDirPath() + "/bin/ass2imd/temp.imd\"";
    nsbmd_destNSBMDpath = "\"" + ui->nsbmd_saveNSBMD_loc->text() + "\"";

    ui->nsbmd_cmd_out->appendPlainText("Status: Generating IMD file, please wait...");

    QProcess *ass2imd = new QProcess(this);
    ass2imd->setProgram(QApplication::applicationDirPath() + "/bin/ass2imd/AssToImd");
    ass2imd->setNativeArguments(nsbmd_srcFBXpath + " -o " + nsbmd_genIMDpath);
    ass2imd->start();
    if(ass2imd->state() == QProcess::NotRunning)
    {
        ui->nsbmd_cmd_out->appendPlainText("Error: Could not start \"./bin/ass2imd/AssToImd\"");
        return;
    }
    connect(ass2imd, SIGNAL(readyRead()), this, SLOT(nsbmd_readErrorsToCmd()));
    connect(ass2imd, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nsbmd_imd_process_end()));

    nsbmd_progressBarTimer = new QTimer();
    nsbmd_progressBarTimer->setSingleShot(false);
    nsbmd_progressBarTimer->start(500);
    nsbmd_progressBarIncrementMax = 75;
    connect(nsbmd_progressBarTimer, &QTimer::timeout, this, &MainWindow::nsbmd_doProgressIncrementation);
}

void MainWindow::nsbmd_imd_process_end() //Convert Part 2
{
    nsbmd_progressBarTimer->stop();

    bool imdExists = QFileInfo::exists(nsbmd_genIMDpath.remove('"')) && QFileInfo(nsbmd_genIMDpath.remove('"')).isFile();
    if(!imdExists) {
        ui->nsbmd_cmd_out->appendPlainText("Status: IMD file generation failed!");
        return;
    }
    else {
        ui->nsbmd_cmd_out->appendPlainText("Status: IMD file generation succeeded!");
        ui->nsbmd_cmd_out->appendPlainText("Status: Converting IMD file to NSBMD!");
    }

    ui->nsbmd_progressBar->setValue(75);

    QProcess *g3dcvtr = new QProcess(this);
    g3dcvtr->setProgram(QApplication::applicationDirPath() + "/bin/imd2nsbmd/g3dcvtr");
    g3dcvtr->setNativeArguments(nsbmd_genIMDpath + " -o " + nsbmd_destNSBMDpath);
    g3dcvtr->start();
    if(g3dcvtr->state() == QProcess::NotRunning)
    {
        ui->nsbmd_cmd_out->appendPlainText("Error: Could not start \"./bin/imd2nsbmd/g3dcvtr\"");
        nsbmd_progressBarTimer->stop();
        return;
    }
    connect(g3dcvtr, SIGNAL(readyRead()), this, SLOT(nsbmd_readErrorsToCmd()));
    connect(g3dcvtr, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nsbmd_g3dcvtr_process_end()));

    nsbmd_progressBarIncrementMax = 100;
    nsbmd_progressBarTimer->start(500);
    connect(nsbmd_progressBarTimer, &QTimer::timeout, this, &MainWindow::nsbmd_doProgressIncrementation);
}

void MainWindow::nsbmd_g3dcvtr_process_end() //Convert Part 3
{
    bool nsbmdExists = QFileInfo::exists(nsbmd_destNSBMDpath.remove('"')) && QFileInfo(nsbmd_destNSBMDpath.remove('"')).isFile();
    if(!nsbmdExists) {
        ui->nsbmd_cmd_out->appendPlainText("Status: IMD to NSBMD file convertion failed!");
        return;
    }
    else {
        ui->nsbmd_cmd_out->appendPlainText("Status: IMD to NSBMD file convertion succeeded!");
    }

    ui->nsbmd_progressBar->setValue(100);
}
#define NSBMD_END }

#define NSBTX_START {
void MainWindow::nsbtx_readErrorsToCmd()
{
    QProcess *myProcess = qobject_cast<QProcess*>(sender());

    QString line;
    do
    {
        line = myProcess->readLine();
        if (line.toLower().contains("error:") || line.toLower().contains("warning:"))
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

    QFile().remove(nsbtx_genMDLpath.remove('"'));
    QFile().remove(nsbtx_destIMDpath.remove('"'));
    QFile().remove(nsbtx_genNSBMDpath.remove('"'));
    QFile().remove(nsbtx_destNSBTXpath.remove('"'));

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
        return;
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
        return;
    }
    connect(g3dcvtr, SIGNAL(readyRead()), this, SLOT(nsbtx_readErrorsToCmd()));
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
            break;
        }
    }

    int TEX0_blockOffset = 0;
    for (int i = 0; i < nsbmdData.size(); i++)
    {
        if(nsbmdData.at(i) == 0x54 && nsbmdData.at(i+1) == 0x45 && nsbmdData.at(i+2) == 0x58 && nsbmdData.at(i+3) == 0x30)
        {
            TEX0_blockOffset = i;
            break;
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
