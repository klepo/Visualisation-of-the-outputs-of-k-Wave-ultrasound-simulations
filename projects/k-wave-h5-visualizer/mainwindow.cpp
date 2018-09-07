/**
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
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
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
    , gWindow(0)
    , openedH5File(0)
    , object(0)
{
    ui->setupUi(this);

    sliceXYDockWidget = ui->dockWidgetSliceXY;
    sliceXZDockWidget = ui->dockWidgetSliceXZ;
    sliceYZDockWidget = ui->dockWidgetSliceYZ;
    opacityGroupBox = ui->groupBoxOpacity;
    datasetInfoGroupBox = ui->groupBoxSelectedDatasetInfo;

    // Docking
    centralWidget()->setParent(0);
    splitDockWidget(sliceXYDockWidget, ui->dockWidgetWindow3D, Qt::Horizontal);
    splitDockWidget(sliceXYDockWidget, sliceXZDockWidget, Qt::Vertical);
    splitDockWidget(sliceXZDockWidget, sliceYZDockWidget, Qt::Vertical);

    // Set slices types
    sliceXYDockWidget->setSliceType(SliceDockWidget::SliceType::XY);
    sliceXZDockWidget->setSliceType(SliceDockWidget::SliceType::XZ);
    sliceYZDockWidget->setSliceType(SliceDockWidget::SliceType::YZ);

    connect(ui->dockWidgetDatasets, SIGNAL(datasetSelected(H5ObjectToVisualize *)), this, SLOT(selectDataset(H5ObjectToVisualize *)));

    // Create timer for animation of data series and connect it
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));

    elapsedTimer = new QElapsedTimer();

    // Create OpenGL window
    gWindow = new GWindow(this);
    // Widget from QWindow
    QWidget *widget = createWindowContainer(gWindow);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *qVBoxLayout = (QVBoxLayout *) ui->dockWidgetContents3D->layout();
    qVBoxLayout->insertWidget(0, widget);

    // Connect signals from gWindow
    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));

    connect(gWindow, SIGNAL(rendered()), this, SLOT(showFPS()));

    // Connect signals to gWindow
    // Enable/disable VR
    connect(ui->actionVolumeRendering, SIGNAL(toggled(bool)), gWindow, SLOT(setViewVolumeRendering(bool)));

    // Show/hide 3D slices
    connect(ui->action3DXY, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXYSlice(bool)));
    connect(ui->action3DXZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXZSlice(bool)));
    connect(ui->action3DYZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewYZSlice(bool)));
    gWindow->setViewXYSlice(ui->action3DXY->isChecked());
    gWindow->setViewXZSlice(ui->action3DXZ->isChecked());
    gWindow->setViewYZSlice(ui->action3DYZ->isChecked());

    // Change VR slices count
    connect(ui->horizontalSliderVRSlices, SIGNAL(valueChanged(int)), gWindow, SLOT(setSlicesCount(int)));

    // Show/hide 3D frame
    connect(ui->actionViewFrame, SIGNAL(toggled(bool)), gWindow, SLOT(setViewFrame(bool)));

    connect(ui->actionFillSpace, SIGNAL(toggled(bool)), gWindow, SLOT(setFillSpace(bool)));

    // Align to basic views
    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));
    connect(ui->actionAlignToXYFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXYFromBack()));
    connect(ui->actionAlignToXZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXZFromBack()));
    connect(ui->actionAlignToYZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToYZFromBack()));

    // Enable/disable trim of values
    connect(ui->groupBoxMinMax, SIGNAL(setTrim(bool)), gWindow, SLOT(setTrim(bool)));

    // Orthogonal projection
    connect(ui->actionOrthogonal, SIGNAL(toggled(bool)), gWindow, SLOT(setOrthogonal(bool)));

    connect(ui->comboBoxInterpolationMode, SIGNAL(currentIndexChanged(int)), gWindow, SLOT(setInterpolationMode(int)));
    connect(ui->comboBoxVolumeRenderingMode, SIGNAL(currentIndexChanged(int)), gWindow, SLOT(setVolumeRenderingMode(int)));

    connect(ui->actionExportImageFrom3DScene, SIGNAL(triggered()), gWindow, SLOT(saveImage()));

    ui->dockWidgetInfo->setVisible(false);

    // Create loading animation
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();

    // Clear GUI
    clearGUI();

    // Create dialog to wait for the file is closed
    dialog = new QDialog(this, Qt::CustomizeWindowHint);
    //dialog->setModal(true);
    dialogUi = new Ui::Dialog;
    dialogUi->setupUi(dialog);
    dialogUi->label->setText("Waiting for completion of reading the file...");
}

/**
 * @brief Destructor of MainWindow object
 *
 * Deletes ui, timer, gWindow, etc.
 */
MainWindow::~MainWindow()
{
    // Close file
    on_actionCloseHDF5File_triggered();

    delete timer;
    delete ui;
    delete gWindow;
    delete dialogUi;
    delete dialog;
}

/**
 * @brief Action on load HDF5 file
 */
void MainWindow::on_actionLoadHDF5File_triggered()
{
    // Create a dialog for opening a file
    QSettings settings("VUT FIT Brno", "k-Wave H5 Visualizer");
    QString fileName = QFileDialog::getOpenFileName(0, "Open File", settings.value("hdf5datafile", QDir::homePath()).toString(), "HDF5 Files (*.h5)");
    settings.setValue("hdf5datafile", QFileInfo(fileName).absolutePath());

    if (!fileName.isEmpty()) {
        // Close file
        on_actionCloseHDF5File_triggered();

        try {
            // Enable closing button
            ui->actionCloseHDF5File->setEnabled(true);

            // Create OpenedH5File
            openedH5File = new H5OpenedFile(fileName);

            // Change the name of the window by file name
            setWindowTitle(QCoreApplication::applicationName() + " - " + openedH5File->getFilename());

            // Set info from HDF5 file to GUI (dock panel)
            ui->dockWidgetInfo->setFile(openedH5File);

            // Clear list of datasets (dock panel)
            ui->dockWidgetDatasets->setFile(openedH5File);

            if (ui->dockWidgetDatasets->getFile()->getObjects().size() == 0) {
                on_actionCloseHDF5File_triggered();
                QMessageBox messageBox;
                messageBox.warning(0, "Notification", "Empty HDF5 file!");
                return;
            }
        } catch (std::exception e) {
            // Wrong HDF5 file
            std::cerr << e.what() << std::endl;
            on_actionCloseHDF5File_triggered();
            QMessageBox messageBox;
            messageBox.critical(0, "Error", "Wrong HDF5 file!");
        }
    }
}

/**
 * @brief Action on close file
 */
void MainWindow::on_actionCloseHDF5File_triggered()
{
    // Show waiting dialog for closing file
    //dialog->show();
    //QApplication::processEvents();

    // Clear pointers
    delete openedH5File;
    openedH5File = 0;

    //dialog->hide();

    object = 0;

    clearGUI();

    // Disable closing button
    ui->actionCloseHDF5File->setEnabled(false);
}

// Show FPS

/**
 * @brief Shows FPS
 */
void MainWindow::showFPS()
{
    QString framesPerSecond, ms;
    framesPerSecond.setNum(1000.0 / gWindow->getElapsedMs(), 'f', 2);
    ms.setNum(gWindow->getElapsedMs(), 'f', 2);
    ui->dockWidgetWindow3D->setWindowTitle("3D view, last render time: " + ms + "ms (" + framesPerSecond + " fps)");
}

void MainWindow::clearGUIForDataset() {
    object = 0;

    sliceXYDockWidget->clear();
    sliceXZDockWidget->clear();
    sliceYZDockWidget->clear();
    opacityGroupBox->clear();
    datasetInfoGroupBox->clear();
    gWindow->clear();

    // Reset colormap
    disconnect(ui->comboBoxColormap, SIGNAL(currentIndexChanged(int)), 0, 0);
    ui->comboBoxColormap->setCurrentIndex(ColorMap::Type::JET);

    // Stop animation
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);

    // Init min and max controls
    ui->groupBoxMinMax->clear();

    ui->spinBoxSelectedDatasetStep->setMaximum(0);
    ui->spinBoxSelectedDatasetStep->setValue(0);
    ui->horizontalSliderSelectedDatasetStep->setMaximum(0);
    ui->horizontalSliderSelectedDatasetStep->setValue(0);
    ui->spinBoxTMIncrement->setMaximum(1);
    ui->spinBoxTMIncrement->setValue(1);
    ui->spinBoxTMInterval->setMaximum(5000);
    ui->spinBoxTMInterval->setMinimum(1);
    ui->spinBoxTMInterval->setValue(1);
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

    clearGUIForDataset();
}

/**
 * @brief Action on select dataset
 */
void MainWindow::selectDataset(H5ObjectToVisualize *object)
{
    clearGUIForDataset();

    if (object != 0) {
        qDebug() << "--> Selected dataset" << object->getName();

        // Init GUI controls
        // TM series control
        ui->spinBoxSelectedDatasetStep->setMaximum(object->getSteps() - 1);
        ui->spinBoxSelectedDatasetStep->setValue(object->getCurrentStep());
        ui->horizontalSliderSelectedDatasetStep->setMaximum(object->getSteps() - 1);
        ui->horizontalSliderSelectedDatasetStep->setValue(object->getCurrentStep());
        ui->spinBoxTMIncrement->setMaximum(object->getSteps() - 1);
        ui->spinBoxTMIncrement->setValue(1);

        this->object = object;

        // Set object to slice widgets
        sliceXYDockWidget->setObject(object);
        sliceXZDockWidget->setObject(object);
        sliceYZDockWidget->setObject(object);

        // Set object to opacity widget
        opacityGroupBox->setObject(object);

        // Set object min/max widget
        ui->groupBoxMinMax->setObject(object);

        // Set dataset info
        datasetInfoGroupBox->setObject(object);

        // Set object to gWindow
        gWindow->setObject(object);

        ui->comboBoxColormap->setCurrentIndex(object->getColormap());
        connect(ui->comboBoxColormap, SIGNAL(currentIndexChanged(int)), object, SLOT(setColormap(int)));

        connect(object, SIGNAL(slicesLoaded()), this, SLOT(slicesLoaded()));
        connect(object, SIGNAL(data3DLoaded(float *)), this, SLOT(data3DLoaded()));
        connect(object, SIGNAL(data3DLoading()), this, SLOT(data3DLoading()));
    }
}

void MainWindow::data3DLoaded()
{
    // Stop loading icon
    ui->label3DLoading->clear();

    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        // If animation is running...
        if (playing && object->areCurrentSlicesLoaded())
            timer->start(ui->spinBoxTMInterval->value());
    }
}

void MainWindow::slicesLoaded()
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        // If animation is running...
        if (playing && object->areCurrentData3DLoaded())
            timer->start(ui->spinBoxTMInterval->value());
    }
}

void MainWindow::data3DLoading()
{
    // Start loading icon
    ui->label3DLoading->setMovie(movie);
}

// Time series controls

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        // Set step in object structure
        //elapsedTimer->restart();
        object->setCurrentStep(hsize_t(step));
    }
}

void MainWindow::updateStep()
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        //qDebug() << double(elapsedTimer->nsecsElapsed()) / 1000000 << "ms";
        // Get current step
        hsize_t step = object->getCurrentStep();
        // Increment of step
        step += ui->spinBoxTMIncrement->value();
        // End of time series
        if (step >= object->getSteps()) {
            // Stop timer
            timer->stop();
            // Stop playing
            playing = false;
            ui->toolButtonPlay->setChecked(false);
            // Set first dataset
            ui->horizontalSliderSelectedDatasetStep->setValue(0);
        } else {
            // Stop timer and update step
            timer->stop();
            ui->horizontalSliderSelectedDatasetStep->setValue(step);
        }
    }
}

void MainWindow::on_toolButtonPlay_clicked(bool checked)
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        if (checked) {
            timer->start(ui->spinBoxTMInterval->value());
            playing = true;
        } else {
            timer->stop();
            playing = false;
        }
    }
}

void MainWindow::on_toolButtonStart_clicked()
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        timer->stop();
        playing = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(0);
    }
}

void MainWindow::on_toolButtonEnd_clicked()
{
    if (object && object->getType() == H5OpenedFile::DATASET_4D) {
        timer->stop();
        playing = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(object->getSteps() - 1);
    }
}

void MainWindow::on_spinBoxTMInterval_valueChanged(int value)
{
    timer->setInterval(value);
}

// Select point on slices, show value

void MainWindow::on_dockWidgetSliceXY_hoveredPointInImage(int x, int y)
{
    if (object != 0) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(object->getValueAtPointFromXY(x, y), 'f', 4), 3000);
    }
}

void MainWindow::on_dockWidgetSliceXZ_hoveredPointInImage(int x, int z)
{
    if (object != 0) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(object->getValueAtPointFromXZ(x, z), 'f', 4), 3000);
    }
}

void MainWindow::on_dockWidgetSliceYZ_hoveredPointInImage(int y, int z)
{
    if (object != 0) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(object->getValueAtPointFromYZ(y, z), 'f', 4), 3000);
    }
}


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
