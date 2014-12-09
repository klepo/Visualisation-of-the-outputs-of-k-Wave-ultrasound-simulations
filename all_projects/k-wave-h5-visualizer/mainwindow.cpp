/*
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing the MainWindow class.
 *              Main window of application.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cvimagewidget.h"
#include "hdf5readingthread.h"
#include "gwindow.h"
#include "ui_dialog.h"

#include <HDF5File.h>
#include <HDF5Dataset.h>
#include <HDF5Group.h>
#include <HDF5Attribute.h>

#include <QFileDialog>
#include <QSlider>
#include <QDialog>
#include <QDebug>
#include <QRadioButton>
#include <QScrollBar>
#include <QTimer>
#include <QToolTip>
#include <QLocale>
#include <QMovie>
#include <QSignalMapper>
#include <QMessageBox>

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gWindow(0)
    , openedH5File(0)
    , file(0)
    , object(0)
    , subobject(0)
{
    ui->setupUi(this);

    // Create timer for animation of data series and connect it
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));

    // Create OpenGL window
    gWindow = new GWindow(this);

    // Widget from QWindow
    QWidget *widget3D = createWindowContainer(gWindow);
    widget3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = (QVBoxLayout *) ui->dockWidgetContents3D->layout();
    l->insertWidget(0, widget3D);

    // Connect signals from gWindow
    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    connect(gWindow, SIGNAL(loaded(std::string)), this, SLOT(loaded3D(std::string)));

    // Connect signals to gWindow
    // Enable/disable VR
    connect(ui->actionVolumeRendering, SIGNAL(toggled(bool)), gWindow, SLOT(setViewVR(bool)));

    // Show/hide 3D slices
    connect(ui->action3DXY, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXYSlice(bool)));
    connect(ui->action3DXZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXZSlice(bool)));
    connect(ui->action3DYZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewYZSlice(bool)));

    // Change VR slices count
    connect(ui->horizontalSliderVRSlices, SIGNAL(valueChanged(int)), gWindow, SLOT(setSlicesCount(int)));

    // Show/hide 3D frame
    connect(ui->actionViewFrame, SIGNAL(toggled(bool)), gWindow, SLOT(setViewFrame(bool)));

    // VR colors and alpha settings
    connect(ui->horizontalSliderVRAlpha, SIGNAL(valueChanged(int)), gWindow, SLOT(setAlpha(int)));
    connect(ui->horizontalSliderVRRed, SIGNAL(valueChanged(int)), gWindow, SLOT(setRed(int)));
    connect(ui->horizontalSliderVRGreen, SIGNAL(valueChanged(int)), gWindow, SLOT(setGreen(int)));
    connect(ui->horizontalSliderVRBlue, SIGNAL(valueChanged(int)), gWindow, SLOT(setBlue(int)));

    // Align to basic views
    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));
    connect(ui->actionAlignToXYFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXYFromBack()));
    connect(ui->actionAlignToXZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXZFromBack()));
    connect(ui->actionAlignToYZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToYZFromBack()));

    // Enable/disable trim of values
    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), gWindow, SLOT(setTrim(bool)));

    // Create loading animation
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();

    // Init visibility of loading
    ui->labelXYLoading->setVisible(false);
    ui->labelXZLoading->setVisible(false);
    ui->labelYZLoading->setVisible(false);
    ui->labelXYLoading->setMovie(movie);
    ui->labelXZLoading->setMovie(movie);
    ui->labelYZLoading->setMovie(movie);

    // Clear GUI
    clearGUI();

    // Create dialog to wait for the file is closed
    dialog = new QDialog(this, Qt::CustomizeWindowHint);
    //dialog->setModal(true);
    Ui::Dialog *dialogUi = new Ui::Dialog;
    dialogUi->setupUi(dialog);
    dialogUi->label->setText("Waiting for completion of reading the file...");
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    // Close file
    on_actionCloseHDF5File_triggered();

    delete timer;
    delete ui;
    delete gWindow;
}

/**
 * @brief MainWindow::on_actionLoadHDF5File_triggered Action on load HDF5 file
 */
void MainWindow::on_actionLoadHDF5File_triggered()
{
    // Create a dialog for opening a file
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "HDF5 Files (*.h5)");

    if (fileName != "") {
        // Close file
        on_actionCloseHDF5File_triggered();

        try {
            // Create OpenedH5File
            openedH5File = new OpenedH5File(fileName);

            // Set info from HDF5 file to GUI (dock panel)
            ui->textBrowserInfo->clear();
            foreach (QString key, openedH5File->getInfo().keys())
                ui->textBrowserInfo->append("<strong>" + key + "</strong><br>" + openedH5File->getInfo().value(key) + "<br>");
            QScrollBar *v = ui->textBrowserInfo->verticalScrollBar();
            // Scroll up
            v->setValue(v->minimum());

            // Change the name of the window by file name
            setWindowTitle(QCoreApplication::applicationName() + " - " + openedH5File->getFilename());

            // Clear list of datasets (dock panel)
            clearLayout(ui->gridLayoutDatasets);

            QRadioButton *lastRadioButton = NULL;
            int i = 0;
            // Load objects to visualize from file
            foreach (QString key, openedH5File->getObjects().keys()) {
                //qDebug() << key;
                // Get GUI pointers
                // Layout
                QGridLayout *gridLayout = ui->gridLayoutDatasets;
                QRadioButton *radioButton = new QRadioButton(key);
                // TODO
                //QCheckBox *checkBox = new QCheckBox();
                //checkBox->setAccessibleName(key);
                // ComboBox for all resolutions of the dataset
                QComboBox *comboBox = new QComboBox();
                // Set actions
                connect(radioButton, SIGNAL(clicked()), this, SLOT(selectDataset()));
                //connect(checkBox, SIGNAL(clicked()), this, SLOT(selectDataset()));

                radioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                //checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

                gridLayout->addWidget(radioButton, i, 0);
                lastRadioButton = radioButton;
                //gridLayout->addWidget(checkBox, i, 1); // TODO multiselected visualization
                gridLayout->addWidget(comboBox, i, 1);

                i++;
                // Fill ComboBox by all resolutions of the dataset
                foreach (QString item, openedH5File->getObjects().value(key)->getSubobjectNames()) {
                    //qDebug() << "->" << item;
                    comboBox->addItem(item);
                    comboBox->setCurrentText(item);
                }
                // Connect change with selection
                connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectDataset()));
            }

            file = openedH5File->getFile();

            // Enable buttons and dock panels
            ui->actionCloseHDF5File->setEnabled(true);
            ui->dockWidgetDatasets->setEnabled(true);
            ui->dockWidgetInfo->setEnabled(true);

            // Select last dataset
            lastRadioButton->click();

        } catch (std::exception &e) {
            // Wrong HDF5 file
            std::cerr << e.what() << std::endl;
            QMessageBox messageBox;
            messageBox.critical(0, "Error", "Wrong HDF5 file!");
        }

    }
}

/**
 * @brief MainWindow::on_actionCloseHDF5File_triggered Action on close file
 */
void MainWindow::on_actionCloseHDF5File_triggered()
{
    // Show waiting dialog for closing file
    dialog->show();
    QApplication::processEvents();

    timer->stop();

    // Clear requets
    clearRequestsAndWaitThreads();

    // Clear pointers
    delete openedH5File;
    openedH5File = NULL;

    dialog->hide();

    object = NULL;
    subobject = NULL;

    clearGUI();
}

/**
 * @brief MainWindow::clearGUI Set GUI to default values
 */
void MainWindow::clearGUI()
{
    // Reset title of application
    setWindowTitle(QCoreApplication::applicationName());
    // Volume rendering flag
    flagVRLoaded = false;
    // Reset colormap
    ui->comboBoxColormap->setCurrentIndex(cv::COLORMAP_JET);
    // Stop animation
    play = false;
    // Disable fill space of 3D scene by sensor mask
    ui->actionFillSpace->setEnabled(false);
    ui->actionFillSpace->setChecked(false);
    // Disable dock panels
    /*ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);*/
    // Reset sliders and spin boxes
    ui->verticalSliderXY->setValue(0);
    ui->verticalSliderXY->setMaximum(99);
    ui->spinBoxXY->setValue(0);
    ui->spinBoxXY->setMaximum(99);
    ui->verticalSliderXZ->setValue(0);
    ui->verticalSliderXZ->setMaximum(99);
    ui->spinBoxXZ->setValue(0);
    ui->spinBoxXZ->setMaximum(99);
    ui->verticalSliderYZ->setValue(0);
    ui->verticalSliderYZ->setMaximum(99);
    ui->spinBoxYZ->setValue(0);
    ui->spinBoxYZ->setMaximum(99);
    // Disable other settings
    ui->checkBoxUseGlobal->setChecked(true);
    ui->toolButtonLocalValues->setChecked(false);
    ui->actionInfo->setChecked(false);
    ui->dockWidgetInfo->setVisible(false);
    // Clear data from image widgets
    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();
    // Clear 3D scene
    if (gWindow != NULL) {
        gWindow->clearData();
    }
    // Clear dataset info
    clearLayout(ui->formLayoutSelectedDatasetInfo);
    // Clear datasets
    clearLayout(ui->gridLayoutDatasets);
    ui->dockWidgetContentsDatasets->adjustSize();
    // Clear info
    ui->textBrowserInfo->clear();
    // Disable closing button
    ui->actionCloseHDF5File->setEnabled(false);

    // Init min and max controls
    ui->doubleSpinBoxMinGlobal->setRange(0, 1);
    ui->doubleSpinBoxMaxGlobal->setRange(0, 1);
    ui->doubleSpinBoxMinGlobal->setValue(0);
    ui->doubleSpinBoxMaxGlobal->setValue(0);
    ui->doubleSpinBoxMinGlobal->setSingleStep(0.1);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(0.1);

    ui->doubleSpinBoxXYMin->setRange(0, 1);
    ui->doubleSpinBoxXYMax->setRange(0, 1);
    ui->doubleSpinBoxXYMin->setValue(0);
    ui->doubleSpinBoxXYMax->setValue(0);
    ui->doubleSpinBoxXYMin->setSingleStep(0.1);
    ui->doubleSpinBoxXYMax->setSingleStep(0.1);

    ui->doubleSpinBoxXZMin->setRange(0, 1);
    ui->doubleSpinBoxXZMax->setRange(0, 1);
    ui->doubleSpinBoxXZMin->setValue(0);
    ui->doubleSpinBoxXZMax->setValue(0);
    ui->doubleSpinBoxXZMin->setSingleStep(0.1);
    ui->doubleSpinBoxXZMax->setSingleStep(0.1);

    ui->doubleSpinBoxYZMin->setRange(0, 1);
    ui->doubleSpinBoxYZMax->setRange(0, 1);
    ui->doubleSpinBoxYZMin->setValue(0);
    ui->doubleSpinBoxYZMax->setValue(0);
    ui->doubleSpinBoxYZMin->setSingleStep(0.1);
    ui->doubleSpinBoxYZMax->setSingleStep(0.1);

    ui->spinBoxSelectedDatasetStep->setMaximum(0);
    ui->spinBoxSelectedDatasetStep->setValue(0);
    ui->horizontalSliderSelectedDatasetStep->setMaximum(0);
    ui->horizontalSliderSelectedDatasetStep->setValue(0);
    ui->spinBoxTMIncrement->setMaximum(1);
    ui->spinBoxTMIncrement->setValue(1);
    ui->spinBoxTMInterval->setMaximum(1000);
    ui->spinBoxTMInterval->setMinimum(16);
    ui->spinBoxTMInterval->setValue(16);
}

/**
 * @brief MainWindow::clearLayout Delete all items from layout
 * @param layout
 */
void MainWindow::clearLayout(QLayout *layout)
{
    QLayout *layout0 = static_cast< QFormLayout* >(layout);
    int count = layout0->count();
    while (--count >= 0) {
        QLayoutItem *child = layout->itemAt(0);
        layout0->removeItem(child);
        delete child->widget();
        delete child;
    }
}

/**
 * @brief MainWindow::clearRequestsAndWaitThreads Clear request in gWindow thread and wait for terminate
 */
void MainWindow::clearRequestsAndWaitThreads()
{
    if (gWindow != NULL) {
        gWindow->getThread()->clearRequests();
        gWindow->getThread()->wait();
        //gWindow->getThread()->clearDoneRequests();
    }
}

/**
 * @brief MainWindow::selectDataset Action on select dataset
 */
void MainWindow::selectDataset()
{
    QString selectedObjectName = "";
    QString selectedSubobjectName = "";

    // Find selected datasets or groups
    QList<QCheckBox *> checkBoxes = ui->dockWidgetContentsDatasets->findChildren<QCheckBox *>();
    foreach (QCheckBox *checkBox, checkBoxes) {
        openedH5File->setObjectSelected(checkBox->accessibleName(), checkBox->isChecked());
    }

    // Select one object
    QList<QRadioButton *> radioButtons = ui->dockWidgetContentsDatasets->findChildren<QRadioButton *>();
    foreach (QRadioButton *radioButton, radioButtons) {
        if (radioButton->isChecked()) {
            selectedObjectName = radioButton->text();
            break;
        }
    }

    // Select one subobject
    QList<QComboBox *> comboBoxes = ui->dockWidgetContentsDatasets->findChildren<QComboBox *>();
    foreach (QComboBox *comboBox, comboBoxes) {
        openedH5File->setSelectedSubobject(comboBox->currentText());
    }

    // Clear gWindow requests
    if (gWindow != NULL)
        gWindow->getThread()->clearRequests();

    // Clear images
    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    // Clear data form 3D scene
    if (gWindow != NULL) {
        gWindow->clearData();
    }

    // Reser flags
    flagDatasetInitialized = false;
    flagVRLoaded = false;

    // Disable dock widgets for selected dataset control
    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    // Stop animation
    play = false;

    // Clear dataset info
    clearLayout(ui->formLayoutSelectedDatasetInfo);

    // Disconnect all last subobjects from image loading
    foreach (OpenedH5File::H5ObjectToVisualize *object, openedH5File->getObjects()) {
        foreach (OpenedH5File::H5SubobjectToVisualize *subobject, object->getSubobjects()) {
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
        }
    }

    object = NULL;
    subobject = NULL;

    // No selected name
    if (selectedObjectName == "")
        return;

    // Set selected name
    selectedSubobjectName = openedH5File->getObject(selectedObjectName)->getSelectedSubobject()->getName();

    // Set object and subobject
    object = openedH5File->getObject(selectedObjectName);
    openedH5File->setSelectedSubobject(selectedSubobjectName);
    if (object != NULL)
        subobject = object->getSelectedSubobject();

    if (subobject != NULL) {

        qDebug() << "--> Selected dataset" << subobject->getName();

        subobject->setGUIInitialized(false);

        // Set dataset info
        QList<QPair<QString, QString>> info = subobject->getInfo();
        for(int i = 0; i < info.count(); ++i)
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel(info[i].first + ":"), new QLabel(info[i].second));

        // Init GUI controls
        initControls();

        //initSlices();
        subobject->setXIndex(subobject->getXIndex());
        subobject->setYIndex(subobject->getYIndex());
        subobject->setZIndex(subobject->getZIndex());
        // Connect repainting image
        connect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), this, SLOT(repaintXYImage(cv::Mat, int)));
        connect(subobject, SIGNAL(imageXZChanged(cv::Mat, int)), this, SLOT(repaintXZImage(cv::Mat, int)));
        connect(subobject, SIGNAL(imageYZChanged(cv::Mat, int)), this, SLOT(repaintYZImage(cv::Mat, int)));
        // Enable controls
        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);

        // TODO
        //connect(ui->horizontalSliderVRAlpha, SIGNAL(valueChanged(int)), subobject, SLOT(setAlpha(int)));


        // Group of datasets (time series) is selected
        if (subobject->getGroup() != NULL) {
            ui->groupBoxSelectedDatasetTMSeries->setEnabled(true);
            ui->actionFillSpace->setEnabled(true);
        } else {
            // One dataset
            ui->groupBoxSelectedDatasetTMSeries->setEnabled(false);
            ui->actionFillSpace->setEnabled(false);
        }

        if (gWindow != NULL) {
            // Reset values in 3D scene (min, max, colormap, size, ...)
            gWindow->changeMinValue(subobject->getMinVG());
            gWindow->changeMaxValue(subobject->getMaxVG());
            gWindow->changeColormap(subobject->getColormap());
            gWindow->setMainSize(subobject->getFrameSize()[0], subobject->getFrameSize()[1], subobject->getFrameSize()[2]);
            gWindow->setSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
            gWindow->setPosition(subobject->getPos()[0], subobject->getPos()[1], subobject->getPos()[2]);
            // Fill space by mask?
            if (ui->actionFillSpace->isChecked()) {
                gWindow->setMainSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
                gWindow->setPosition(0, 0, 0);
            }
        }

        /*if (subobject->getSize()[0] > 600 || subobject->getSize()[1] > 600 || subobject->getSize()[2] > 600) {
            ui->actionVolumeRendering->setEnabled(false);
            ui->actionVolumeRendering->setChecked(false);
            ui->groupBoxVolumeRendering->setEnabled(false);
            qDebug() << "Dataset is too big for Volume Rendering (> 600).";
            QMessageBox messageBox;
            messageBox.information(0, "Info", "Dataset is too big for Volume Rendering (> 600).");
        } else */if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            // Set Volume rendering
            ui->actionVolumeRendering->setEnabled(true);
            ui->groupBoxVolumeRendering->setEnabled(true);
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            //if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
            // Start loading 3D texture
            gWindow->load3DTexture(subobject->getDataset());
        } else {
            // Disable VR
            ui->actionVolumeRendering->setEnabled(true);
            ui->groupBoxVolumeRendering->setEnabled(true);
        }
        // Set init GUI flag
        subobject->setGUIInitialized(true);
    }
}

/**
 * @brief MainWindow::repaintXYImage Set new XY image if it is loaded
 * @param image Image data of XY slice
 * @param index Index of XY slice
 */
void MainWindow::repaintXYImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        // Send data to 3D scene
        if (gWindow != NULL) {
            if (subobject->getSize()[0] == 1) // Index -> 0
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) 0);
            else
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) ui->verticalSliderXY->value() / (subobject->getSize()[0] - 1));
        }
        // Point for positioning of sensor mask image
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[1]);
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetXY->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(index) + ")");

        // Set local minimal and maximal values for controls
        subobject->setGUIXYInitialized(false);
        ui->doubleSpinBoxXYMin->setRange(subobject->getOriginalMinVXY(), subobject->getOriginalMaxVXY());
        ui->doubleSpinBoxXYMax->setRange(subobject->getOriginalMinVXY(), subobject->getOriginalMaxVXY());
        ui->doubleSpinBoxXYMin->setValue(subobject->getMinVXY());
        ui->doubleSpinBoxXYMax->setValue(subobject->getMaxVXY());
        ui->doubleSpinBoxXYMin->setSingleStep((subobject->getOriginalMaxVXY() - subobject->getOriginalMinVXY()) / 1000);
        ui->doubleSpinBoxXYMax->setSingleStep((subobject->getOriginalMaxVXY() - subobject->getOriginalMinVXY()) / 1000);
        subobject->setGUIXYInitialized(true);

        // Continue playing animation if it is possible
        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());
        // Hide loading animation
        if (subobject->isCurrentXYLoaded()) ui->labelXYLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::repaintXZImage Set new XZ image if it is loaded
 * @param image Image data of XZ slice
 * @param index Index of XZ slice
 */
void MainWindow::repaintXZImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        // Send data to 3D scene
        if (gWindow != NULL) {
            if (subobject->getSize()[1] == 1) // Index -> 0
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) 0);
            else
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) ui->verticalSliderXZ->value() / (subobject->getSize()[1] - 1));
        }
        // Point for positioning of sensor mask image
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[0]);
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetXZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(index) + ")");

        // Set local minimal and maximal values for controls
        subobject->setGUIXZInitialized(false);
        ui->doubleSpinBoxXZMin->setRange(subobject->getOriginalMinVXZ(), subobject->getOriginalMaxVXZ());
        ui->doubleSpinBoxXZMax->setRange(subobject->getOriginalMinVXZ(), subobject->getOriginalMaxVXZ());
        ui->doubleSpinBoxXZMin->setValue(subobject->getMinVXZ());
        ui->doubleSpinBoxXZMax->setValue(subobject->getMaxVXZ());
        ui->doubleSpinBoxXZMin->setSingleStep((subobject->getOriginalMaxVXZ() - subobject->getOriginalMinVXZ()) / 1000);
        ui->doubleSpinBoxXZMax->setSingleStep((subobject->getOriginalMaxVXZ() - subobject->getOriginalMinVXZ()) / 1000);
        subobject->setGUIXZInitialized(true);

        // Continue playing animation if it is possible
        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        // Hide loading animation
        if (subobject->isCurrentXZLoaded()) ui->labelXZLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::repaintYZImage Set new YZ image if it is loaded
 * @param image Image data of YZ slice
 * @param index Index of YZ slice
 */
void MainWindow::repaintYZImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        // Send data to 3D scene
        if (gWindow != NULL) {
            if (subobject->getSize()[2] == 1) // Index -> 0
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) 0);
            else
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) ui->verticalSliderYZ->value() / (subobject->getSize()[2] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[1], subobject->getPos()[0]);
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetYZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(index) + ")");

        // Set local minimal and maximal values for controls
        subobject->setGUIYZInitialized(false);
        ui->doubleSpinBoxYZMin->setRange(subobject->getOriginalMinVYZ(), subobject->getOriginalMaxVYZ());
        ui->doubleSpinBoxYZMax->setRange(subobject->getOriginalMinVYZ(), subobject->getOriginalMaxVYZ());
        ui->doubleSpinBoxYZMin->setValue(subobject->getMinVYZ());
        ui->doubleSpinBoxYZMax->setValue(subobject->getMaxVYZ());
        ui->doubleSpinBoxYZMin->setSingleStep((subobject->getOriginalMaxVYZ() - subobject->getOriginalMinVYZ()) / 1000);
        ui->doubleSpinBoxYZMax->setSingleStep((subobject->getOriginalMaxVYZ() - subobject->getOriginalMinVYZ()) / 1000);
        subobject->setGUIYZInitialized(true);

        // Continue playing animation if it is possible
        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        // Hide loading animation
        if (subobject->isCurrentYZLoaded()) ui->labelYZLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::initControls Initialization of selected dataset controls
 */
void MainWindow::initControls()
{
    if (subobject != NULL) {
        // Init min and max controls
        ui->doubleSpinBoxMinGlobal->setRange(subobject->getOriginalMinVG(), subobject->getOriginalMaxVG());
        ui->doubleSpinBoxMaxGlobal->setRange(subobject->getOriginalMinVG(), subobject->getOriginalMaxVG());
        ui->doubleSpinBoxMinGlobal->setValue(subobject->getMinVG());
        ui->doubleSpinBoxMaxGlobal->setValue(subobject->getMaxVG());
        ui->doubleSpinBoxMinGlobal->setSingleStep((subobject->getOriginalMaxVG() - subobject->getOriginalMinVG()) / 1000);
        ui->doubleSpinBoxMaxGlobal->setSingleStep((subobject->getOriginalMaxVG() - subobject->getOriginalMinVG()) / 1000);

        // TM series control
        ui->spinBoxSelectedDatasetStep->setMaximum(subobject->getSteps()-1);
        ui->spinBoxSelectedDatasetStep->setValue(subobject->getCurrentStep());
        ui->horizontalSliderSelectedDatasetStep->setMaximum(subobject->getSteps()-1);
        ui->horizontalSliderSelectedDatasetStep->setValue(subobject->getCurrentStep());
        ui->spinBoxTMIncrement->setMaximum(subobject->getSteps()-1);
        ui->spinBoxTMIncrement->setValue(1);

        // Slices control
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(subobject->getZIndex()) + ")");
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(subobject->getYIndex()) + ")");
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(subobject->getXIndex()) + ")");

        ui->verticalSliderXY->setMaximum(subobject->getSize()[0] - 1);
        ui->verticalSliderXY->setValue(subobject->getZIndex());
        ui->spinBoxXY->setMaximum(subobject->getSize()[0] - 1);
        ui->spinBoxXY->setValue(subobject->getZIndex());

        ui->verticalSliderXZ->setMaximum(subobject->getSize()[1] - 1);
        ui->verticalSliderXZ->setValue(subobject->getYIndex());
        ui->spinBoxXZ->setMaximum(subobject->getSize()[1] - 1);
        ui->spinBoxXZ->setValue(subobject->getYIndex());

        ui->verticalSliderYZ->setMaximum(subobject->getSize()[2] - 1);
        ui->verticalSliderYZ->setValue(subobject->getXIndex());
        ui->spinBoxYZ->setMaximum(subobject->getSize()[2] - 1);
        ui->spinBoxYZ->setValue(subobject->getXIndex());

        ui->checkBoxUseGlobal->setChecked(subobject->getUseGlobal());

        ui->comboBoxColormap->setCurrentIndex(subobject->getColormap());

        // Loading aminmation
        ui->labelXYLoading->setVisible(true);
        ui->labelXZLoading->setVisible(true);
        ui->labelYZLoading->setVisible(true);
    }
}

/**
 * @brief MainWindow::loaded3D 3D data loaded event
 * @param datasetName Name of loaded dataset
 */
void MainWindow::loaded3D(std::string datasetName)
{
    // Set flag
    flagVRLoaded = true;
    ui->label3DLoading->clear();

    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL)
        // If animation is running...
        if (play && subobject->areCurrentSlicesLoaded() && datasetName == subobject->getDataset()->getName())
            timer->start(ui->spinBoxTMInterval->value());
}

// Visibility of docked panels

void MainWindow::on_dockWidgetXY_visibilityChanged(bool)
{
    if (ui->dockWidgetXY->isVisible())
        ui->actionXY->setChecked(true);
    else
        ui->actionXY->setChecked(false);
}

void MainWindow::on_dockWidgetXZ_visibilityChanged(bool)
{
    if (ui->dockWidgetXZ->isVisible())
        ui->actionXZ->setChecked(true);
    else
        ui->actionXZ->setChecked(false);
}

void MainWindow::on_dockWidgetYZ_visibilityChanged(bool)
{
    if (ui->dockWidgetYZ->isVisible())
        ui->actionYZ->setChecked(true);
    else
        ui->actionYZ->setChecked(false);
}

void MainWindow::on_dockWidgetInfo_visibilityChanged(bool)
{
    if (ui->dockWidgetInfo->isVisible())
        ui->actionInfo->setChecked(true);
    else
        ui->actionInfo->setChecked(false);
}

void MainWindow::on_dockWidgetDatasets_visibilityChanged(bool)
{
    if (ui->dockWidgetDatasets->isVisible())
        ui->actionDatasets->setChecked(true);
    else
        ui->actionDatasets->setChecked(false);
}

/*void MainWindow::on_dockWidgetCT_visibilityChanged(bool)
{
    if (ui->dockWidgetCT->isVisible())
        ui->actionCT->setChecked(true);
    else
        ui->actionCT->setChecked(false);
}*/

void MainWindow::on_dockWidgetSelectedDataset_visibilityChanged(bool)
{
    if (ui->dockWidgetSelectedDataset->isVisible())
        ui->actionSelectedDataset->setChecked(true);
    else
        ui->actionSelectedDataset->setChecked(false);
}

// Use global values settings

void MainWindow::on_checkBoxUseGlobal_clicked(bool checked)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        subobject->setUseGlobal(checked);
    }
}

// Global min and max values change

void MainWindow::on_horizontalSliderGlobalMin_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMinGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    ui->doubleSpinBoxMaxGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        subobject->setMinVG(value);
    }
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);

    if (gWindow != NULL)
        gWindow->changeMinValue(value);
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        subobject->setMaxVG(value);
    }
    ui->horizontalSliderGlobalMax->setTracking(false);
    ui->horizontalSliderGlobalMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMax->setTracking(true);

    if (gWindow != NULL)
        gWindow->changeMaxValue(value);
}

// Local min and max values change

void MainWindow::on_horizontalSliderXYMin_valueChanged(int value)
{
    ui->doubleSpinBoxXYMin->setValue((double) value / 1000 * (ui->doubleSpinBoxXYMin->maximum() - ui->doubleSpinBoxXYMin->minimum()) + ui->doubleSpinBoxXYMin->minimum());
}

void MainWindow::on_doubleSpinBoxXYMin_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIXYInitialized()) {
        subobject->setMinVXY(value);
    }
    ui->horizontalSliderXYMin->setTracking(false);
    ui->horizontalSliderXYMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXYMin->minimum()) / (ui->doubleSpinBoxXYMin->maximum() - ui->doubleSpinBoxXYMin->minimum())));
    ui->horizontalSliderXYMin->setTracking(true);
}

void MainWindow::on_horizontalSliderXYMax_valueChanged(int value)
{
    ui->doubleSpinBoxXYMax->setValue((double) value / 1000 * (ui->doubleSpinBoxXYMax->maximum() - ui->doubleSpinBoxXYMax->minimum()) + ui->doubleSpinBoxXYMax->minimum());
}

void MainWindow::on_doubleSpinBoxXYMax_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIXYInitialized()) {
        subobject->setMaxVXY(value);
    }
    ui->horizontalSliderXYMax->setTracking(false);
    ui->horizontalSliderXYMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXYMax->minimum()) / (ui->doubleSpinBoxXYMax->maximum() - ui->doubleSpinBoxXYMax->minimum())));
    ui->horizontalSliderXYMax->setTracking(true);
}

void MainWindow::on_horizontalSliderXZMin_valueChanged(int value)
{
    ui->doubleSpinBoxXZMin->setValue((double) value / 1000 * (ui->doubleSpinBoxXZMin->maximum() - ui->doubleSpinBoxXZMin->minimum()) + ui->doubleSpinBoxXZMin->minimum());
}

void MainWindow::on_doubleSpinBoxXZMin_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIXZInitialized()) {
        subobject->setMinVXZ(value);
    }
    ui->horizontalSliderXZMin->setTracking(false);
    ui->horizontalSliderXZMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXZMin->minimum()) / (ui->doubleSpinBoxXZMin->maximum() - ui->doubleSpinBoxXZMin->minimum())));
    ui->horizontalSliderXZMin->setTracking(true);
}

void MainWindow::on_horizontalSliderXZMax_valueChanged(int value)
{
    ui->doubleSpinBoxXZMax->setValue((double) value / 1000 * (ui->doubleSpinBoxXZMax->maximum() - ui->doubleSpinBoxXZMax->minimum()) + ui->doubleSpinBoxXZMax->minimum());
}

void MainWindow::on_doubleSpinBoxXZMax_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIXZInitialized()) {
        subobject->setMaxVXZ(value);
    }
    ui->horizontalSliderXZMax->setTracking(false);
    ui->horizontalSliderXZMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXZMax->minimum()) / (ui->doubleSpinBoxXZMax->maximum() - ui->doubleSpinBoxXZMax->minimum())));
    ui->horizontalSliderXZMax->setTracking(true);
}

void MainWindow::on_horizontalSliderYZMin_valueChanged(int value)
{
    ui->doubleSpinBoxYZMin->setValue((double) value / 1000 * (ui->doubleSpinBoxYZMin->maximum() - ui->doubleSpinBoxYZMin->minimum()) + ui->doubleSpinBoxYZMin->minimum());
}

void MainWindow::on_doubleSpinBoxYZMin_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIYZInitialized()) {
        subobject->setMinVYZ(value);
    }
    ui->horizontalSliderYZMin->setTracking(false);
    ui->horizontalSliderYZMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxYZMin->minimum()) / (ui->doubleSpinBoxYZMin->maximum() - ui->doubleSpinBoxYZMin->minimum())));
    ui->horizontalSliderYZMin->setTracking(true);
}

void MainWindow::on_horizontalSliderYZMax_valueChanged(int value)
{
    ui->doubleSpinBoxYZMax->setValue((double) value / 1000 * (ui->doubleSpinBoxYZMax->maximum() - ui->doubleSpinBoxYZMax->minimum()) + ui->doubleSpinBoxYZMax->minimum());
}

void MainWindow::on_doubleSpinBoxYZMax_valueChanged(double value)
{
    if (subobject != NULL && subobject->isGUIYZInitialized()) {
        subobject->setMaxVYZ(value);
    }
    ui->horizontalSliderYZMax->setTracking(false);
    ui->horizontalSliderYZMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxYZMax->minimum()) / (ui->doubleSpinBoxYZMax->maximum() - ui->doubleSpinBoxYZMax->minimum())));
    ui->horizontalSliderYZMax->setTracking(true);
}

// Colormap change

void MainWindow::on_comboBoxColormap_currentIndexChanged(int index)
{
    if (gWindow != NULL)
        gWindow->changeColormap(index);
    if (subobject != NULL && subobject->isGUIInitialized()) {
        subobject->setColormap(index);
    }
}

// Time series controls

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL) {
        // Set step in subobject structure
        subobject->setCurrentStep(step, gWindow->getThread());
        // For VR
        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            // Enable loading animation
            ui->label3DLoading->setMovie(movie);
            // Load 3D data
            gWindow->unloadDataset();
            gWindow->load3DTexture(subobject->getDataset());
        }
    }
}

void MainWindow::updateStep()
{
    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL) {
        // Get current step
        uint64_t step = subobject->getCurrentStep();
        // Increment of step
        step += ui->spinBoxTMIncrement->value();
        // End of time series
        if (step >= subobject->getSteps()) {
            // Stop timer
            timer->stop();
            // Stop playing
            play = false;
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
    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL) {
        if (checked) {
            timer->start(ui->spinBoxTMInterval->value());
            play = true;
        } else {
            timer->stop();
            play = false;
        }
    }
}

void MainWindow::on_toolButtonStart_clicked()
{
    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL) {
        timer->stop();
        play = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(0);
    }
}

void MainWindow::on_toolButtonEnd_clicked()
{
    if (subobject != NULL && subobject->isGUIInitialized() && subobject->getGroup() != NULL) {
        timer->stop();
        play = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(subobject->getSteps() - 1);
    }
}

void MainWindow::on_spinBoxTMInterval_valueChanged(int value)
{
    timer->setInterval(value);
}

// Select point on slices, show value

void MainWindow::on_imageWidgetXY_hoveredPointInImage(int x, int y)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromXY(x, y), 'f', 4), 3000);
    }
}

void MainWindow::on_imageWidgetXZ_hoveredPointInImage(int x, int z)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromXZ(x, z), 'f', 4), 3000);
    }
}

void MainWindow::on_imageWidgetYZ_hoveredPointInImage(int y, int z)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromYZ(y, z), 'f', 4), 3000);
    }
}

// Load 3D texture on enable

void MainWindow::on_actionVolumeRendering_toggled(bool value)
{
    if (gWindow != NULL && subobject != NULL && subobject->isGUIInitialized() && value/*&& !gWindow->isTexture3DInitialized()*/) {
        gWindow->load3DTexture(subobject->getDataset());
        if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
    }
}

// Save 3D scene image

void MainWindow::on_actionExportImageFrom3DScene_triggered()
{
    if (gWindow != NULL) {
        QString name = "no_name";
        if (subobject != NULL && subobject->isGUIInitialized()) name = subobject->getName();
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", name + "_3Dscene.png", "Image (*.png)");
        if (fileName != NULL)
            gWindow->saveImage(fileName);
    }
}

// Alpha, r, g, b sliders recomputing, connected together

void MainWindow::on_horizontalSliderVRAlpha_valueChanged(int value)
{
    ui->doubleSpinBoxVRAlpha->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRRed_valueChanged(int value)
{
    ui->doubleSpinBoxVRRed->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRGreen_valueChanged(int value)
{
    ui->doubleSpinBoxVRGreen->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRBlue_valueChanged(int value)
{
    ui->doubleSpinBoxVRBlue->setValue((double) value / 1000);
}

void MainWindow::on_doubleSpinBoxVRAlpha_valueChanged(double value)
{
    ui->horizontalSliderVRAlpha->setValue((int) (value * 1000));
}

void MainWindow::on_doubleSpinBoxVRRed_valueChanged(double value)
{
    ui->horizontalSliderVRRed->setValue((int) (value * 1000));
}

void MainWindow::on_doubleSpinBoxVRGreen_valueChanged(double value)
{
    ui->horizontalSliderVRGreen->setValue((int) (value * 1000));
}

void MainWindow::on_doubleSpinBoxVRBlue_valueChanged(double value)
{
    ui->horizontalSliderVRBlue->setValue((int) (value * 1000));
}

// Index sliders

void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->labelXYLoading->setVisible(true);
        subobject->setZIndex(value);
    }
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->labelXZLoading->setVisible(true);
        subobject->setYIndex(value);
    }
}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        ui->labelYZLoading->setVisible(true);
        subobject->setXIndex(value);
    }
}

// Fill 3D space

void MainWindow::on_actionFillSpace_toggled(bool value)
{
    if (subobject != NULL && subobject->isGUIInitialized()) {
        if (value == true) {
            gWindow->setMainSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
            gWindow->setPosition(0, 0, 0);
            gWindow->renderLater();
        } else {
            gWindow->setMainSize(subobject->getFrameSize()[0], subobject->getFrameSize()[1], subobject->getFrameSize()[2]);
            gWindow->setPosition(subobject->getPos()[0], subobject->getPos()[1], subobject->getPos()[2]);
            gWindow->renderLater();
        }
    }
}

/**
 * @brief MainWindow::on_actionAbout_triggered Action for help window
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
