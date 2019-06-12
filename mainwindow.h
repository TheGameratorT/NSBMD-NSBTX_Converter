#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sourceMDLpath_btn_clicked();

    void on_saveNSBMDpath_btn_clicked();

    void on_convert_btn_clicked();

    void nsbmd_readErrorsToCmd();

    void doIMD2NSBMD();

    void doProgressIncrementation();

    void nsbmd_finishConvertion();

    void nsbtx_readErrorsToCmd();

    void on_nsbtx_convert_btn_clicked();

    void on_nsbtx_image_source_select_clicked();

    void on_nsbtx_save_path_select_clicked();

    void nsbtx_imd_process_end();

    void nsbtx_g3dcvtr_process_end();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
