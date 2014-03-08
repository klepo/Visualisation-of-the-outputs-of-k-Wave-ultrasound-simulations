#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cvimagewidget.h"
#include "dataset.h"

#include <QFileDialog>
#include <QSlider>
#include <QDialog>
#include <QDebug>

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

void MainWindow::on_verticalSlider_Z_valueChanged(int value)
{
    ui->dockWidget_Z->setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
}

void MainWindow::on_verticalSlider_Y_valueChanged(int value)
{
    ui->dockWidget_Y->setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
}

void MainWindow::on_verticalSlider_X_valueChanged(int value)
{
    ui->dockWidget_X->setWindowTitle("YZ slice (X = " + QString::number(value) + ")");
}

void MainWindow::on_actionLoad_output_HDF5_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("HDF5 Files (*.h5)"));

    if (fileName != "") {

        Dataset *dataset = new Dataset(fileName.toStdString());
        std::cout << std::endl << "Opened " << fileName.toStdString() << std::endl;

        //CVImageWidget* imageWidgetZ = this->findChild<CVImageWidget*>("imageWidgetZ");
        //QSlider* verticalSlider_Z = this->findChild<QSlider*>("verticalSlider_Z");
        ui->imageWidget_Z->unsetDataset();
        ui->spinBox_Z->setMaximum(dataset->getMaximum(0));
        ui->verticalSlider_Z->setValue(0);
        ui->verticalSlider_Z->setMaximum(dataset->getMaximum(0));
        ui->imageWidget_Z->setDataset(dataset, 0);

        //CVImageWidget* imageWidgetY = this->findChild<CVImageWidget*>("imageWidgetY");
        //QSlider* verticalSlider_Y = this->findChild<QSlider*>("verticalSlider_Y");
        ui->imageWidget_Y->unsetDataset();
        ui->spinBox_Y->setMaximum(dataset->getMaximum(1));
        ui->verticalSlider_Y->setValue(0);
        ui->verticalSlider_Y->setMaximum(dataset->getMaximum(1));
        ui->imageWidget_Y->setDataset(dataset, 1);

        //CVImageWidget* imageWidgetX = this->findChild<CVImageWidget*>("imageWidgetX");
        //QSlider* verticalSlider_X = this->findChild<QSlider*>("verticalSlider_X");
        ui->imageWidget_X->unsetDataset();
        ui->spinBox_X->setMaximum(dataset->getMaximum(2));
        ui->verticalSlider_X->setValue(0);
        ui->verticalSlider_X->setMaximum(dataset->getMaximum(2));
        ui->imageWidget_X->setDataset(dataset, 2);

    }
}

void MainWindow::on_dockWidget_Z_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidget_Z->isVisible())
        ui->actionXY_slice->setChecked(true);
    else
        ui->actionXY_slice->setChecked(false);
}

void MainWindow::on_actionXY_Slice_triggered(bool checked)
{
    ui->dockWidget_Z->setVisible(checked);
}

void MainWindow::on_dockWidget_Y_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidget_Y->isVisible())
        ui->actionXZ_slice->setChecked(true);
    else
        ui->actionXZ_slice->setChecked(false);
}

void MainWindow::on_actionXZ_Slice_triggered(bool checked)
{
    ui->dockWidget_Y->setVisible(checked);
}

void MainWindow::on_dockWidget_X_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidget_X->isVisible())
        ui->actionYZ_slice->setChecked(true);
    else
        ui->actionYZ_slice->setChecked(false);
}

void MainWindow::on_actionYZ_Slice_triggered(bool checked)
{
    ui->dockWidget_X->setVisible(checked);
}
