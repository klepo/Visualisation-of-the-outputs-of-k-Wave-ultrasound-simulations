/**
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *              13 February  2017 (updated)
 *
 * @brief       The implementation file containing the MainWindow class.
 *              Main window of application.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "mainwindow.h"

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

    opacity = QVector<float>(5, 1);

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

    connect(gWindow, SIGNAL(rendered()), this, SLOT(showFPS()));

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

    // Align to basic views
    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));
    connect(ui->actionAlignToXYFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXYFromBack()));
    connect(ui->actionAlignToXZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXZFromBack()));
    connect(ui->actionAlignToYZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToYZFromBack()));

    // Enable/disable trim of values
    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), gWindow, SLOT(setTrim(bool)));

    connect(ui->actionOrthogonal, SIGNAL(toggled(bool)), gWindow, SLOT(setOrthogonal(bool)));

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
    QSettings settings("VUT FIT Brno", "k-Wave H5 Visualizer");

    // Create a dialog for opening a file
    QString fileName = QFileDialog::getOpenFileName(0, "Open File", settings.value("hdf5datafile", QDir::homePath()).toString(), "HDF5 Files (*.h5)");

    settings.setValue("hdf5datafile", QFileInfo(fileName).absolutePath());

    if (!fileName.isEmpty()) {
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

            QRadioButton *lastRadioButton = 0;
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

            if (i == 0) {
                on_actionCloseHDF5File_triggered();
                QMessageBox messageBox;
                messageBox.warning(0, "Notification", "Empty HDF5 file!");
                return;
            }

            file = openedH5File->getFile();

            // Enable buttons and dock panels
            ui->actionCloseHDF5File->setEnabled(true);
            ui->dockWidgetDatasets->setEnabled(true);
            ui->dockWidgetInfo->setEnabled(true);

            // Select last dataset
            lastRadioButton->click();

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
 * @brief MainWindow::on_actionCloseHDF5File_triggered Action on close file
 */
void MainWindow::on_actionCloseHDF5File_triggered()
{
    // Show waiting dialog for closing file
    dialog->show();
    QApplication::processEvents();

    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);

    // Clear requets
    clearRequestsAndWaitThreads();

    // Clear pointers
    delete openedH5File;
    openedH5File = 0;

    dialog->hide();

    object = 0;
    subobject = 0;

    clearGUI();
}

// Show FPS

/**
 * @brief MainWindow::showFPS
 */
void MainWindow::showFPS()
{
    QString framesPerSecond, ms;
    framesPerSecond.setNum(1000.0 / gWindow->getElapsedMs(), 'f', 2);
    ms.setNum(gWindow->getElapsedMs(), 'f', 2);
    ui->dockWidget3D->setWindowTitle("3D view, last render time: " + ms + "ms (" + framesPerSecond + " fps)");
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
    ui->comboBoxColormap->setCurrentIndex(ColorMap::Type::JET);
    // Stop animation
    playing = false;
    // Disable fill space of 3D scene by sensor mask
    ui->actionFillSpace->setEnabled(false);
    ui->actionFillSpace->setChecked(false);
    // Disable dock panels
    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);
    // Reset sliders and spin boxes
    ui->verticalSliderXY->setMaximum(0);
    ui->verticalSliderXY->setValue(0);
    ui->spinBoxXY->setMaximum(0);
    ui->spinBoxXY->setValue(0);
    ui->verticalSliderXZ->setMaximum(0);
    ui->verticalSliderXZ->setValue(0);
    ui->spinBoxXZ->setMaximum(0);
    ui->spinBoxXZ->setValue(0);
    ui->verticalSliderYZ->setMaximum(0);
    ui->verticalSliderYZ->setValue(0);
    ui->spinBoxYZ->setMaximum(0);
    ui->spinBoxYZ->setValue(0);
    // Disable other settings
    ui->actionInfo->setChecked(false);
    ui->dockWidgetInfo->setVisible(false);
    // Clear data from image widgets
    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();
    // Clear 3D scene
    if (gWindow != 0) {
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
    ui->doubleSpinBoxMinGlobal->setRange(0, 0);
    ui->doubleSpinBoxMaxGlobal->setRange(0, 0);
    ui->doubleSpinBoxMinGlobal->setValue(0);
    ui->doubleSpinBoxMaxGlobal->setValue(0);
    ui->doubleSpinBoxMinGlobal->setSingleStep(0.1);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(0.1);

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
    if (gWindow != 0) {
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
    if (gWindow != 0)
        gWindow->getThread()->clearRequests();

    // Clear images
    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    // Clear data form 3D scene
    if (gWindow != 0) {
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
    playing = false;

    // Clear dataset info
    clearLayout(ui->formLayoutSelectedDatasetInfo);

    // Disconnect all last subobjects from image loading
    foreach (OpenedH5File::H5ObjectToVisualize *object, openedH5File->getObjects()) {
        foreach (OpenedH5File::H5SubobjectToVisualize *subobject, object->getSubobjects()) {
            disconnect(subobject, SIGNAL(imageXYChanged(QImage, uint64_t)), 0, 0);
            disconnect(subobject, SIGNAL(imageXZChanged(QImage, uint64_t)), 0, 0);
            disconnect(subobject, SIGNAL(imageYZChanged(QImage, uint64_t)), 0, 0);
        }
    }

    object = 0;
    subobject = 0;

    // No selected name
    if (selectedObjectName == "")
        return;

    // Set object and subobject
    object = openedH5File->getObject(selectedObjectName);

    if (object != 0 && object->getSelectedSubobject()) {
        subobject = object->getSelectedSubobject();
        // Set selected name
        selectedSubobjectName = subobject->getName();
        openedH5File->setSelectedSubobject(selectedSubobjectName);

        if (subobject != 0) {

        qDebug() << "--> Selected dataset" << subobject->getName();

        subobject->setGUIInitialized(false);

        // Set dataset info
        QList<QPair<QString, QString>> info = subobject->getInfo();
        for (int i = 0; i < info.count(); ++i)
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel(info[i].first + ":"), new QLabel(info[i].second));

        // Init GUI controls
        initControls();

        //initSlices();
        subobject->setXIndex(subobject->getXIndex());
        subobject->setYIndex(subobject->getYIndex());
        subobject->setZIndex(subobject->getZIndex());
        // Connect repainting image
        connect(subobject, SIGNAL(imageXYChanged(QImage, uint64_t)), this, SLOT(repaintXYImage(QImage, uint64_t)));
        connect(subobject, SIGNAL(imageXZChanged(QImage, uint64_t)), this, SLOT(repaintXZImage(QImage, uint64_t)));
        connect(subobject, SIGNAL(imageYZChanged(QImage, uint64_t)), this, SLOT(repaintYZImage(QImage, uint64_t)));
        // Enable controls
        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);


        // Group of datasets (time series) is selected
        if (subobject->getType() == OpenedH5File::dataset4D_t) {
            ui->groupBoxSelectedDatasetTMSeries->setEnabled(true);
            ui->actionFillSpace->setEnabled(true);
        } else {
            // One dataset
            ui->groupBoxSelectedDatasetTMSeries->setEnabled(false);
            ui->actionFillSpace->setEnabled(false);
        }

        if (gWindow != 0) {
            // Reset values in 3D scene (min, max, colormap, size, ...)
            gWindow->changeMinValue(subobject->getMinValue());
            gWindow->changeMaxValue(subobject->getMaxValue());
            gWindow->changeColormap(subobject->getColormap());
            gWindow->setMainSize(subobject->getFrameSize());
            gWindow->setSize(subobject->getSize());
            gWindow->setPosition(subobject->getPos());
            // Fill space by mask?
            if (ui->actionFillSpace->isChecked()) {
                gWindow->setMainSize(subobject->getSize());
                gWindow->setPosition(HDF5Helper::HDF5Vector3D(0, 0, 0));
            }
        }

        /*if (subobject->getSize().z() > 600 || subobject->getSize().y() > 600 || subobject->getSize().x() > 600) {
            ui->actionVolumeRendering->setEnabled(false);
            ui->actionVolumeRendering->setChecked(false);
            ui->groupBoxVolumeRendering->setEnabled(false);
            qDebug() << "Dataset is too big for Volume Rendering (> 600).";
            QMessageBox messageBox;
            messageBox.information(0, "Info", "Dataset is too big for Volume Rendering (> 600).");
        } else */if (gWindow != 0 && ui->actionVolumeRendering->isChecked()) {
            // Set Volume rendering
            ui->actionVolumeRendering->setEnabled(true);
            ui->groupBoxVolumeRendering->setEnabled(true);
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            //if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
            // Start loading 3D texture
            gWindow->load3DTexture(subobject->getDataset(), subobject->getCurrentStep());
        } else {
            // Disable VR
            ui->actionVolumeRendering->setEnabled(true);
            ui->groupBoxVolumeRendering->setEnabled(true);
        }
        // Set init GUI flag
        subobject->setGUIInitialized(true);
    }
    }
}

/**
 * @brief MainWindow::repaintXYImage Set new XY image if it is loaded
 * @param image Image data of XY slice
 * @param index Index of XY slice
 */
void MainWindow::repaintXYImage(QImage image, uint64_t index)
{
    if (subobject != 0) {
        // Send data to 3D scene
        if (gWindow != 0) {
            if (subobject->getSize().z() == 1) // Index -> 0
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize().x(), subobject->getSize().y(), (float) 0);
            else
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize().x(), subobject->getSize().y(), (float) ui->verticalSliderXY->value() / (subobject->getSize().z() - 1));
        }
        // Point for positioning of sensor mask image
        QPoint p = QPoint(subobject->getPos().x(), subobject->getPos().y());
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetXY->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(index) + ")");

        // Continue playing animation if it is possible
        if (playing && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());
        // Hide loading animation
        if (subobject->isCurrentXYLoaded())
            ui->labelXYLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::repaintXZImage Set new XZ image if it is loaded
 * @param image Image data of XZ slice
 * @param index Index of XZ slice
 */
void MainWindow::repaintXZImage(QImage image, uint64_t index)
{
    if (subobject != 0) {
        // Send data to 3D scene
        if (gWindow != 0) {
            if (subobject->getSize().y() == 1) // Index -> 0
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize().x(), subobject->getSize().z(), (float) 0);
            else
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize().x(), subobject->getSize().z(), (float) ui->verticalSliderXZ->value() / (subobject->getSize().y() - 1));
        }
        // Point for positioning of sensor mask image
        QPoint p = QPoint(subobject->getPos().x(), subobject->getPos().z());
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetXZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(index) + ")");

        // Continue playing animation if it is possible
        if (playing && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        // Hide loading animation
        if (subobject->isCurrentXZLoaded())
            ui->labelXZLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::repaintYZImage Set new YZ image if it is loaded
 * @param image Image data of YZ slice
 * @param index Index of YZ slice
 */
void MainWindow::repaintYZImage(QImage image, uint64_t index)
{
    if (subobject != 0) {
        // Send data to 3D scene
        if (gWindow != 0) {
            if (subobject->getSize().x() == 1) // Index -> 0
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize().y(), subobject->getSize().z(), (float) 0);
            else
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize().y(), subobject->getSize().z(), (float) ui->verticalSliderYZ->value() / (subobject->getSize().x() - 1));
        }
        QPoint p = QPoint(subobject->getPos().y(), subobject->getPos().z());
        p = QPoint(0, 0); // TODO Disabled
        // Set image data to image widget
        ui->imageWidgetYZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        // Set title for dock panel
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(index) + ")");

        // Continue playing animation if it is possible
        if (playing && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        // Hide loading animation
        if (subobject->isCurrentYZLoaded())
            ui->labelYZLoading->setVisible(false);
    }
}

/**
 * @brief MainWindow::initControls Initialization of selected dataset controls
 */
void MainWindow::initControls()
{
    if (subobject != 0) {
        // Init min and max controls
        ui->doubleSpinBoxMinGlobal->setRange(subobject->getOriginalMinValue(), subobject->getOriginalMaxValue());
        ui->doubleSpinBoxMaxGlobal->setRange(subobject->getOriginalMinValue(), subobject->getOriginalMaxValue());
        ui->doubleSpinBoxMinGlobal->setValue(subobject->getMinValue());
        ui->doubleSpinBoxMaxGlobal->setValue(subobject->getMaxValue());
        ui->doubleSpinBoxMinGlobal->setSingleStep((subobject->getOriginalMaxValue() - subobject->getOriginalMinValue()) / 1000);
        ui->doubleSpinBoxMaxGlobal->setSingleStep((subobject->getOriginalMaxValue() - subobject->getOriginalMinValue()) / 1000);

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

        ui->verticalSliderXY->setMaximum(subobject->getSize().z() - 1);
        ui->verticalSliderXY->setValue(subobject->getZIndex());
        ui->spinBoxXY->setMaximum(subobject->getSize().z() - 1);
        ui->spinBoxXY->setValue(subobject->getZIndex());

        ui->verticalSliderXZ->setMaximum(subobject->getSize().y() - 1);
        ui->verticalSliderXZ->setValue(subobject->getYIndex());
        ui->spinBoxXZ->setMaximum(subobject->getSize().y() - 1);
        ui->spinBoxXZ->setValue(subobject->getYIndex());

        ui->verticalSliderYZ->setMaximum(subobject->getSize().x() - 1);
        ui->verticalSliderYZ->setValue(subobject->getXIndex());
        ui->spinBoxYZ->setMaximum(subobject->getSize().x() - 1);
        ui->spinBoxYZ->setValue(subobject->getXIndex());

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

    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
        // If animation is running...
        if (playing && subobject->areCurrentSlicesLoaded() && datasetName == subobject->getDataset()->getName())
            timer->start(ui->spinBoxTMInterval->value());
    }
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
    if (subobject != 0 && subobject->isGUIInitialized()) {
        subobject->setMinValue(value);
    }
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);

    if (gWindow != 0)
        gWindow->changeMinValue(value);
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        subobject->setMaxValue(value);
    }
    ui->horizontalSliderGlobalMax->setTracking(false);
    ui->horizontalSliderGlobalMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMax->setTracking(true);

    if (gWindow != 0)
        gWindow->changeMaxValue(value);
}

// Colormap change

void MainWindow::on_comboBoxColormap_currentIndexChanged(int index)
{
    if (gWindow != 0)
        gWindow->changeColormap(static_cast<ColorMap::Type>(index));
    if (subobject != 0 && subobject->isGUIInitialized()) {
        subobject->setColormap(static_cast<ColorMap::Type>(index));
    }
}

// Time series controls

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
        // Set step in subobject structure
        subobject->setCurrentStep(step/*, gWindow->getThread()*/);
        // For VR
        if (gWindow && ui->actionVolumeRendering->isChecked()) {
            // Enable loading animation
            ui->label3DLoading->setMovie(movie);
            // Load 3D data
            gWindow->unloadDataset();
            gWindow->load3DTexture(subobject->getDataset(), step);
        }
    }
}

void MainWindow::updateStep()
{
    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
        // Get current step
        uint64_t step = subobject->getCurrentStep();
        // Increment of step
        step += ui->spinBoxTMIncrement->value();
        // End of time series
        if (step >= subobject->getSteps()) {
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
    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
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
    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
        timer->stop();
        playing = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(0);
    }
}

void MainWindow::on_toolButtonEnd_clicked()
{
    if (subobject && subobject->isGUIInitialized() && subobject->getType() == OpenedH5File::dataset4D_t) {
        timer->stop();
        playing = false;
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
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromXY(x, y), 'f', 4), 3000);
    }
}

void MainWindow::on_imageWidgetXZ_hoveredPointInImage(int x, int z)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromXZ(x, z), 'f', 4), 3000);
    }
}

void MainWindow::on_imageWidgetYZ_hoveredPointInImage(int y, int z)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(subobject->getValueAtPointFromYZ(y, z), 'f', 4), 3000);
    }
}

// Load 3D texture on enable

void MainWindow::on_actionVolumeRendering_toggled(bool value)
{
    if (gWindow != 0 && subobject != 0 && subobject->isGUIInitialized() && value/*&& !gWindow->isTexture3DInitialized()*/) {
        gWindow->load3DTexture(subobject->getDataset(), subobject->getCurrentStep());
        if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
    }
}

// Save 3D scene image

void MainWindow::on_actionExportImageFrom3DScene_triggered()
{
    if (gWindow != 0) {
        QString name = "no_name";
        if (subobject != 0 && subobject->isGUIInitialized()) {
            QString fileName = subobject->getOpenedH5File()->getFilename();
            QString objectName = subobject->getH5ObjectToVisualize()->getName();
            QString subobjectName = subobject->getOnlyName();
            name = fileName + "_" + objectName + "_" + subobjectName + "_3Dscene.png";
        }
        QString imagefileName = QFileDialog::getSaveFileName(this, "Save image", name, "Image (*.png)");
        if (imagefileName != 0)
            gWindow->saveImage(imagefileName);
    }
}

// Index sliders

void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->labelXYLoading->setVisible(true);
        subobject->setZIndex(value);
    }
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->labelXZLoading->setVisible(true);
        subobject->setYIndex(value);
    }
}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        ui->labelYZLoading->setVisible(true);
        subobject->setXIndex(value);
    }
}

// Fill 3D space

void MainWindow::on_actionFillSpace_toggled(bool value)
{
    if (subobject != 0 && subobject->isGUIInitialized()) {
        if (value == true) {
            gWindow->setMainSize(subobject->getSize());
            gWindow->setPosition(HDF5Helper::HDF5Vector3D(0, 0, 0));
            gWindow->renderLater();
        } else {
            gWindow->setMainSize(subobject->getFrameSize());
            gWindow->setPosition(subobject->getPos());
            gWindow->renderLater();
        }
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

void MainWindow::on_comboBoxMode_currentIndexChanged(int index)
{
    if (gWindow != 0)
        gWindow->changeMode(index);
    //if (subobject != 0 && subobject->isGUIInitialized()) {
    //    subobject->setColormap(static_cast<ColorMap::Type>(index));
    //}
}

void MainWindow::on_doubleSpinBox_0_valueChanged(double value)
{
    ui->verticalSlider_0->setValue(int(value * 1000));
    opacity[0] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void MainWindow::on_verticalSlider_0_valueChanged(int value)
{
    ui->doubleSpinBox_0->setValue(double(value) / 1000);
}

void MainWindow::on_doubleSpinBox_1_valueChanged(double value)
{
    ui->verticalSlider_1->setValue(int(value * 1000));
    opacity[1] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void MainWindow::on_verticalSlider_1_valueChanged(int value)
{
    ui->doubleSpinBox_1->setValue(double(value) / 1000);
}

void MainWindow::on_doubleSpinBox_2_valueChanged(double value)
{
    ui->verticalSlider_2->setValue(int(value * 1000));
    opacity[2] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void MainWindow::on_verticalSlider_2_valueChanged(int value)
{
    ui->doubleSpinBox_2->setValue(double(value) / 1000);
}

void MainWindow::on_doubleSpinBox_3_valueChanged(double value)
{
    ui->verticalSlider_3->setValue(int(value * 1000));
    opacity[3] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void MainWindow::on_verticalSlider_3_valueChanged(int value)
{
    ui->doubleSpinBox_3->setValue(double(value) / 1000);
}

void MainWindow::on_doubleSpinBox_4_valueChanged(double value)
{
    int nv = int(value * 1000);
    ui->verticalSlider_4->setValue(nv);
    opacity[4] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void MainWindow::on_verticalSlider_4_valueChanged(int value)
{
    double nv = double(value) / 1000;
    ui->doubleSpinBox_4->setValue(nv);
}
