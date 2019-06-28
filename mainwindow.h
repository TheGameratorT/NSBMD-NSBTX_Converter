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
    void on_nsbmd_sourceMDLpath_btn_clicked();

    void on_nsbmd_saveNSBMDpath_btn_clicked();

    void on_nsbmd_convert_btn_clicked();

    void nsbmd_readErrorsToCmd();

    void nsbmd_imd_process_end();

    void nsbmd_doProgressIncrementation();

    void nsbmd_g3dcvtr_process_end();

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
