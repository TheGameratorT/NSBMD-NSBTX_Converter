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

    void readErrorsToCmd();

    void doIMD2NSBMD();

    void doProgressIncrementation();

    void finishConvertion();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
