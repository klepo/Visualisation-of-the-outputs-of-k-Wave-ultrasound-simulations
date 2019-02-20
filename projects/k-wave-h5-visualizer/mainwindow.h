/**
 * @file        mainwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The header file with MainWindow class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include <QTextEdit>

#include "ui_mainwindow.h"

#include "h5openedfile.h"

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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionLoadHDF5File_triggered();
    void on_actionCloseHDF5File_triggered();
    void showFPS(qint64 elapsedNs);
    void showOpeningFile();
    void hideOpeningFile();
    void showHoveredPointValue(float value);
    void datasetSelectionChanged(int id, bool selected);
    void on_actionAbout_triggered();

    void on_dockWidgetSliceXY_visibilityChanged(bool visible);

    void on_dockWidgetSliceXZ_visibilityChanged(bool visible);

    void on_dockWidgetSliceYZ_visibilityChanged(bool visible);

private:
    Q_DISABLE_COPY(MainWindow)

    /// Last 3D XY view flag
    bool last3DXY = true;
    /// Last 3D XZ view flag
    bool last3DXZ = true;
    /// Last 3D YZ view flag
    bool last3DYZ = true;

    void connectGWindowActions(GWindow *gWindow);
    void clearGUIForDataset();
    void clearGUI();

    /// User interface
    Ui::MainWindow *ui;
    /// Opened H5 file
    H5OpenedFile *openedH5File = nullptr;

    /// Loading animation
    QMovie *movie;
    /// Loading label for animation
    QLabel *labelLoading;
};

#endif // MAINWINDOW_H
