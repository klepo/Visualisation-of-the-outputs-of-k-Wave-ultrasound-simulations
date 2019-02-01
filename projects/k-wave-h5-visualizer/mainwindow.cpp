/**
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              22 November  2018 (updated)
 *
 * @brief       The implementation file containing MainWindow class definition.
 *
 * Main window of application.
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

#include "mainwindow.h"

/**
 * @brief Creates MainWindow
 * @param[in] parent Parent (optional)
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Docking
    centralWidget()->setParent(nullptr);
    splitDockWidget(ui->dockWidgetSliceXY, ui->dockWidgetWindow3D, Qt::Horizontal);
    splitDockWidget(ui->dockWidgetSliceXY, ui->dockWidgetSliceXZ, Qt::Vertical);
    splitDockWidget(ui->dockWidgetSliceXZ, ui->dockWidgetSliceYZ, Qt::Vertical);

    // Set slices types
    ui->dockWidgetSliceXY->setSliceType(SliceDockWidget::SliceType::XY);
    ui->dockWidgetSliceXZ->setSliceType(SliceDockWidget::SliceType::XZ);
    ui->dockWidgetSliceYZ->setSliceType(SliceDockWidget::SliceType::YZ);

    // Action on dataset selection
    connect(ui->dockWidgetDatasets, SIGNAL(datasetSelectionChanged(int, bool)), this, SLOT(datasetSelectionChanged(int, bool)));

    // Connect gWindow with actions
    connectGWindowActions(ui->dockWidgetWindow3D->getGWindow());

    // Hide HDF5 info widget
    ui->dockWidgetInfo->setVisible(false);

    labelLoading = new QLabel;
    statusBar()->setStyleSheet("QStatusBar::item { border: 0px solid black }");
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();
    labelLoading->setMinimumSize(20, 20);
    statusBar()->addPermanentWidget(labelLoading);

    // Clear GUI
    clearGUI();
}

/**
 * @brief Destructor of MainWindow object
 *
 * Deletes ui, timer, gWindow, etc.
 */
MainWindow::~MainWindow()
{
    // Close file
    //on_actionCloseHDF5File_triggered();
    if (openedH5File) {
        delete openedH5File;
        openedH5File = nullptr;
    }

    delete ui;
}

/**
 * @brief Action on load HDF5 file
 */
void MainWindow::on_actionLoadHDF5File_triggered()
{
    showOpeningFile();
    // Create a dialog for opening a file
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    QString filename = QFileDialog::getOpenFileName(nullptr, "Open File", settings.value("hdf5datafile", QDir::homePath()).toString(), "HDF5 Files (*.h5)");
    settings.setValue("hdf5datafile", QFileInfo(filename).absolutePath());

    if (!filename.isEmpty()) {
        // Close file
        on_actionCloseHDF5File_triggered();

        try {
            // Enable closing button
            ui->actionCloseHDF5File->setEnabled(true);

            // Create OpenedH5File
            openedH5File = new H5OpenedFile(filename);

            // Change the name of the window by file name
            setWindowTitle(QCoreApplication::applicationName() + " - " + openedH5File->getFilename());

            // Set info from HDF5 file to GUI (dock panel)
            ui->dockWidgetInfo->setFile(openedH5File);

            // Clear list of datasets (dock panel)
            ui->dockWidgetDatasets->setFile(openedH5File);

            if (openedH5File->getObjects().size() == 0) {
                on_actionCloseHDF5File_triggered();
                QMessageBox messageBox;
                messageBox.warning(nullptr, "Notification", "Empty HDF5 file!");
                hideOpeningFile();
                return;
            }
        } catch (std::exception e) {
            // Wrong HDF5 file
            std::cerr << e.what() << std::endl;
            on_actionCloseHDF5File_triggered();
            QMessageBox messageBox;
            messageBox.critical(nullptr, "Error", "Wrong HDF5 file!");
        }
    }
    hideOpeningFile();
}

/**
 * @brief Action on close file
 */
void MainWindow::on_actionCloseHDF5File_triggered()
{
    clearGUI();

    // Delete H5OpenedFile and H5ObjectToVisualize objects
    // Because of the missing smart pointers it must be done after clearing widgets
    if (openedH5File) {
        //delete openedH5File;
        openedH5File->deleteLater();
        openedH5File = nullptr;
    }

    // Disable closing button
    ui->actionCloseHDF5File->setEnabled(false);
}

/**
 * @brief Shows FPS
 * @param[in] time Render time in ms
 */
void MainWindow::showFPS(qint64 elapsedNs)
{
    QString framesPerSecond, ms;
    double elapsedMs = double(elapsedNs / 1000000.0);
    framesPerSecond.setNum(1000.0 / elapsedMs, 'f', 2);
    ms.setNum(elapsedMs, 'f', 2);
    ui->dockWidgetWindow3D->setWindowTitle("3D view, last render time: " + ms + "ms (" + framesPerSecond + " fps)");
}

void MainWindow::showOpeningFile()
{
    statusBar()->showMessage("Opening file ...");
    labelLoading->setMovie(movie);
}

void MainWindow::hideOpeningFile()
{
    labelLoading->clear();
    statusBar()->clearMessage();
}

/**
 * @brief Shows hovered point value
 * @param[in] value Hovered point value
 */
void MainWindow::showHoveredPointValue(float value)
{
    ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
}

/**
 * @brief Action on dataset selection changed
 * @param[in] id Dataset index in table of datasets
 * @param[in] selected Selected flag
 */
void MainWindow::datasetSelectionChanged(int id, bool selected)
{
    H5ObjectToVisualize *object = openedH5File->getObjects().at(id);
    if (object) {
        if (selected) {
            object->setSelected(true);
            clearGUIForDataset();

            qDebug() << "--> Selected dataset" << object->getName();

            connect(object, SIGNAL(hoveredPointInImage(float)), this, SLOT(showHoveredPointValue(float)));

            // Set object to selected dataset widget
            ui->dockWidgetSelectedDataset->setObject(object);

            // Set object to slice widgets
            ui->dockWidgetSliceXY->setObject(object);
            ui->dockWidgetSliceXZ->setObject(object);
            ui->dockWidgetSliceYZ->setObject(object);

            // Set object to 3D window
            ui->dockWidgetWindow3D->setObject(object);
        } else {
            object->setSelected(false);
            object->disconnectSignals();
            clearGUIForDataset();
        }
    }
}

/**
 * @brief Shows about message in window
 */
void MainWindow::on_actionAbout_triggered()
{
    QTextEdit* help = new QTextEdit();
    help->setWindowFlags(Qt::Window); //or Qt::Tool, Qt::Dialog if you like
    help->setReadOnly(true);
    help->setWindowModality(Qt::ApplicationModal);
    help->setFixedSize(400, 200);
    QFile file(":/html/help.html");
    file.open(QIODevice::ReadOnly);
    QByteArray dump = file.readAll();
    help->setHtml(dump);
    help->show();
}

/**
 * @brief Connects GWindow actions
 * @param[in] gWindow GWindow object
 */
void MainWindow::connectGWindowActions(GWindow *gWindow)
{
    // Connect signals from gWindow
    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    connect(gWindow, SIGNAL(rendered(qint64)), this, SLOT(showFPS(qint64)));

    // Connect signals to gWindow
    // Enable/disable VR
    connect(ui->actionVolumeRendering, SIGNAL(toggled(bool)), gWindow, SLOT(setViewVolumeRendering(bool)));
    gWindow->setViewVolumeRendering(ui->actionVolumeRendering->isChecked());

    // Show/hide 3D slices
    connect(ui->action3DXY, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXYSlice(bool)));
    connect(ui->action3DXZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXZSlice(bool)));
    connect(ui->action3DYZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewYZSlice(bool)));

    gWindow->setViewXYSlice(ui->action3DXY->isChecked());
    gWindow->setViewXZSlice(ui->action3DXZ->isChecked());
    gWindow->setViewYZSlice(ui->action3DYZ->isChecked());
    last3DXY = ui->action3DXY->isChecked();
    last3DXZ = ui->action3DXZ->isChecked();
    last3DYZ = ui->action3DYZ->isChecked();

    // Show/hide 3D frame
    connect(ui->actionViewFrame, SIGNAL(toggled(bool)), gWindow, SLOT(setViewFrame(bool)));
    gWindow->setViewFrame(ui->actionViewFrame->isChecked());

    // Enable/disable fill space
    connect(ui->actionFillSpace, SIGNAL(toggled(bool)), gWindow, SLOT(setFillSpace(bool)));
    gWindow->setFillSpace(ui->actionFillSpace->isChecked());

    // Enable/disable orthogonal projection
    connect(ui->actionOrthogonal, SIGNAL(toggled(bool)), gWindow, SLOT(setOrthogonal(bool)));
    gWindow->setOrthogonal(ui->actionOrthogonal->isChecked());

    // Align to basic views
    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));
    connect(ui->actionAlignToXYFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXYFromBack()));
    connect(ui->actionAlignToXZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXZFromBack()));
    connect(ui->actionAlignToYZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToYZFromBack()));

    // Export image form 3D scene
    connect(ui->actionExportImageFrom3DScene, SIGNAL(triggered()), gWindow, SLOT(saveImage()));

    // Connect 3D settings
    // 3D interpolation mode
    gWindow->setInterpolationMode(ui->dockWidgetSettings3D->getInterpolationMode());
    connect(ui->dockWidgetSettings3D, SIGNAL(interpolationModeChanged(int)), gWindow, SLOT(setInterpolationMode(int)));
    // VR mode
    gWindow->setVolumeRenderingMode(ui->dockWidgetSettings3D->getVolumeRenderingMode());
    connect(ui->dockWidgetSettings3D, SIGNAL(volumeRenderingModeChanged(int)), gWindow, SLOT(setVolumeRenderingMode(int)));
    // VR slices count
    gWindow->setSlicesCount(ui->dockWidgetSettings3D->getVolumeRenderingSlices());
    connect(ui->dockWidgetSettings3D, SIGNAL(volumeRenderingSlicesChanged(int)), gWindow, SLOT(setSlicesCount(int)));
}

/**
 * @brief Sets GUI for selected dataset to default values
 */
void MainWindow::clearGUIForDataset() {
    // Clear selected dataset widget
    ui->dockWidgetSelectedDataset->clear();

    // Clear slices widgets
    ui->dockWidgetSliceXY->clear();
    ui->dockWidgetSliceXZ->clear();
    ui->dockWidgetSliceYZ->clear();

    // Clear Window 3D dataset
    ui->dockWidgetWindow3D->clear();
}

/**
 * @brief Sets GUI to default values
 */
void MainWindow::clearGUI()
{
    // Reset title of application
    setWindowTitle(QCoreApplication::applicationName());

    // Clear datasets
    ui->dockWidgetDatasets->clear();

    // Clear info
    ui->dockWidgetInfo->clear();

    // Clear GUI for selected dataset
    clearGUIForDataset();
}

void MainWindow::on_dockWidgetSliceXY_visibilityChanged(bool visible)
{
    if (visible) {
        ui->action3DXY->setEnabled(true);
        ui->action3DXY->setChecked(last3DXY);
    } else {
        last3DXY = ui->action3DXY->isChecked();
        ui->action3DXY->setChecked(false);
        ui->action3DXY->setEnabled(false);
    }
}

void MainWindow::on_dockWidgetSliceXZ_visibilityChanged(bool visible)
{
    if (visible) {
        ui->action3DXZ->setEnabled(true);
        ui->action3DXZ->setChecked(last3DXZ);
    } else {
        last3DXZ = ui->action3DXZ->isChecked();
        ui->action3DXZ->setChecked(false);
        ui->action3DXZ->setEnabled(false);
    }
}

void MainWindow::on_dockWidgetSliceYZ_visibilityChanged(bool visible)
{
    if (visible) {
        ui->action3DYZ->setEnabled(true);
        ui->action3DYZ->setChecked(last3DYZ);
    } else {
        last3DYZ = ui->action3DYZ->isChecked();
        ui->action3DYZ->setChecked(false);
        ui->action3DYZ->setEnabled(false);
    }
}
