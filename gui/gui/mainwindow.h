#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_verticalSlider_Z_valueChanged(int value);
    void on_verticalSlider_Y_valueChanged(int value);
    void on_verticalSlider_X_valueChanged(int value);
    void on_actionLoad_output_HDF5_file_triggered();

    void on_dockWidget_Z_visibilityChanged(bool visible);
    void on_actionXY_Slice_triggered(bool checked);
    void on_dockWidget_Y_visibilityChanged(bool);
    void on_actionXZ_Slice_triggered(bool checked);
    void on_dockWidget_X_visibilityChanged(bool);
    void on_actionYZ_Slice_triggered(bool checked);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
