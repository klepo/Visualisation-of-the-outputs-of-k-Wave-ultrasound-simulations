/**
 * @file        mainwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with MainWindow class declaration.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include <QWidget>
#include <QScrollBar>
#include <QRadioButton>

#include "ui_dialog.h"
#include "ui_mainwindow.h"

#include "cvimagewidget.h"
#include "gwindow.h"
#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"
#include "hdf5readingthread.h"
#include "openedh5file.h"

#include <hdf5helper.h>

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

public slots:
    void repaintXYImage(QImage image, uint64_t index);
    void repaintXZImage(QImage image, uint64_t index);
    void repaintYZImage(QImage image, uint64_t index);

    void loaded3D(std::string datasetName);

private slots:
    void on_dockWidgetXY_visibilityChanged(bool);
    void on_dockWidgetXZ_visibilityChanged(bool);
    void on_dockWidgetYZ_visibilityChanged(bool);

    void on_dockWidgetInfo_visibilityChanged(bool);
    //void on_dockWidgetCT_visibilityChanged(bool);
    void on_dockWidgetDatasets_visibilityChanged(bool);
    void on_dockWidgetSelectedDataset_visibilityChanged(bool);

    void on_actionLoadHDF5File_triggered();

    void on_actionCloseHDF5File_triggered();
    void selectDataset();

    void on_horizontalSliderGlobalMin_valueChanged(int value);
    void on_horizontalSliderGlobalMax_valueChanged(int value);
    void on_doubleSpinBoxMinGlobal_valueChanged(double value);
    void on_doubleSpinBoxMaxGlobal_valueChanged(double value);

    void on_comboBoxColormap_currentIndexChanged(int index);

    void on_spinBoxSelectedDatasetStep_valueChanged(int step);
    void on_toolButtonPlay_clicked(bool checked);
    void updateStep();
    void on_toolButtonStart_clicked();
    void on_toolButtonEnd_clicked();

    void on_spinBoxTMInterval_valueChanged(int value);

    void on_imageWidgetXY_hoveredPointInImage(int x, int y);
    void on_imageWidgetXZ_hoveredPointInImage(int x, int y);
    void on_imageWidgetYZ_hoveredPointInImage(int x, int y);

    void on_actionVolumeRendering_toggled(bool arg1);

    void on_actionExportImageFrom3DScene_triggered();

    void on_horizontalSliderVRAlpha_valueChanged(int value);
    void on_horizontalSliderVRRed_valueChanged(int value);
    void on_horizontalSliderVRGreen_valueChanged(int value);
    void on_horizontalSliderVRBlue_valueChanged(int value);
    void on_doubleSpinBoxVRAlpha_valueChanged(double value);
    void on_doubleSpinBoxVRRed_valueChanged(double value);
    void on_doubleSpinBoxVRGreen_valueChanged(double value);
    void on_doubleSpinBoxVRBlue_valueChanged(double value);

    void on_verticalSliderXY_valueChanged(int value);
    void on_verticalSliderXZ_valueChanged(int value);
    void on_verticalSliderYZ_valueChanged(int value);

    void on_actionFillSpace_toggled(bool arg1);

    void on_actionAbout_triggered();

    void on_saveVideoButton_clicked();

public:
    Ui::MainWindow *ui;

private:
    void clearGUI();
    void clearRequestsAndWaitThreads();
    void initControls();
    void clearLayout(QLayout *layout);

    GWindow *gWindow;

    OpenedH5File *openedH5File;

    HDF5Helper::File *file;

    OpenedH5File::H5ObjectToVisualize *object;
    OpenedH5File::H5SubobjectToVisualize *subobject;

    bool flagDatasetInitialized;

    bool flagVRLoaded;

    QTimer *timer;

    bool playing = false;

    bool recording = false;

    QMovie *movie;

    QDialog *dialog;
};

#endif // MAINWINDOW_H
