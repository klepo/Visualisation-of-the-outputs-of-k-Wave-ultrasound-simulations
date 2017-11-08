/**
 * @file        mainwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with MainWindow class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
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

#include "imagewidget.h"
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

/**
 * @brief The MainWindow class represents wrapper for the main application window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /// Window user interface
    Ui::MainWindow *ui;

public slots:
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

    void on_dockWidgetXY_hoveredPointInImage(int x, int y);
    void on_dockWidgetXZ_hoveredPointInImage(int x, int y);
    void on_dockWidgetYZ_hoveredPointInImage(int x, int y);

    void on_actionVolumeRendering_toggled(bool arg1);

    void on_actionExportImageFrom3DScene_triggered();

    void on_actionFillSpace_toggled(bool arg1);

    void on_actionAbout_triggered();

    void on_comboBoxMode_currentIndexChanged(int index);

    void showFPS();

    void on_comboBox_currentIndexChanged(int index);

private:
    void clearGUI();
    void clearRequestsAndWaitThreads();
    void initControls();
    void clearLayout(QLayout *layout);

    GWindow *gWindow;
    OpenedH5File *openedH5File;
    OpenedH5File::H5ObjectToVisualize *object;
    OpenedH5File::H5SubobjectToVisualize *subobject;
    QTimer *timer;
    QMovie *movie;
    QDialog *dialog;
    Ui::Dialog *dialogUi;
    bool flagVRLoaded = false;
    bool playing = false;
};

#endif // MAINWINDOW_H
