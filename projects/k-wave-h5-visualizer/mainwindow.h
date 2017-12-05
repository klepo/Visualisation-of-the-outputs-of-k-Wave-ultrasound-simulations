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
#include <QTextEdit>

#include <ui_dialog.h>
#include <ui_mainwindow.h>

//#include <imagewidget.h>
//#include <opacitygroupbox.h>
#include <gwindow.h>
#include <h5objecttovisualize.h>
#include <h5readingthread.h>
#include <h5openedfile.h>

#include <k-wave-h5-helper.h>

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


private slots:
    void data3Dloaded();
    void slicesLoaded();
    void data3Dloading();

    void on_actionLoadHDF5File_triggered();

    void on_actionCloseHDF5File_triggered();
    void selectDataset(H5ObjectToVisualize *object);

    void on_spinBoxSelectedDatasetStep_valueChanged(int step);
    void on_toolButtonPlay_clicked(bool checked);
    void updateStep();
    void on_toolButtonStart_clicked();
    void on_toolButtonEnd_clicked();

    void on_spinBoxTMInterval_valueChanged(int value);

    void on_dockWidgetSliceXY_hoveredPointInImage(int x, int y);
    void on_dockWidgetSliceXZ_hoveredPointInImage(int x, int y);
    void on_dockWidgetSliceYZ_hoveredPointInImage(int x, int y);

    void on_actionAbout_triggered();

    void showFPS();

private:
    void clearGUIForDataset();
    void clearGUI();
    void clearRequestsAndWaitThreads();

    SliceDockWidget *sliceXYDockWidget;
    SliceDockWidget *sliceXZDockWidget;
    SliceDockWidget *sliceYZDockWidget;
    OpacityGroupBox *opacityGroupBox;
    DatasetInfoGroupBox *datasetInfoGroupBox;
    GWindow *gWindow;

    H5OpenedFile *openedH5File;
    H5ObjectToVisualize *object;
    QTimer *timer;
    QMovie *movie;
    QDialog *dialog;
    Ui::Dialog *dialogUi;
    bool playing = false;
};

#endif // MAINWINDOW_H
