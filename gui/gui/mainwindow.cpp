#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cvimagewidget.h"
#include "hdf5readingthread.h"
#include "gwindow.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    file = NULL;
    dataXY = NULL;
    dataXZ = NULL;
    dataYZ = NULL;

    windowTitle = "k-Wave HDF5 visualizer";

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));

    qRegisterMetaType<hsize_t>("hsize_t");
    threadXY = new HDF5ReadingThread();
    connect(threadXY, SIGNAL(dataLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setXYLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));
    threadXZ = new HDF5ReadingThread();
    connect(threadXZ, SIGNAL(dataLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setXZLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));
    threadYZ = new HDF5ReadingThread();
    connect(threadYZ, SIGNAL(dataLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setYZLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));

    gWindow = NULL;
    gWindow = new GWindow();
    QWidget *widget3D = createWindowContainer(gWindow);
    widget3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = (QVBoxLayout *) ui->dockWidgetContents3D->layout();
    l->insertWidget(0, widget3D);
    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    connect(gWindow, SIGNAL(loaded(std::string)), this, SLOT(loaded3D(std::string)));

    //connect(gWindow, SIGNAL(partLoaded(int)), ui->progressBar3D, SLOT(setValue(int)));

    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));

    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), gWindow, SLOT(setTrim(bool)));

    //ui->progressBar3D->setVisible(false);

    ui->toolButtonPositionXY->setVisible(false);
    ui->toolButtonPositionXZ->setVisible(false);
    ui->toolButtonPositionYZ->setVisible(false);

    movie = new QMovie(":/icons/icons/loading.gif");
    movie->start();

    clearGUI();
}

MainWindow::~MainWindow()
{
    on_actionCloseHDF5File_triggered();
    delete ui;
    delete timer;
    delete gWindow;
    threadXY->deleteLater();
    threadXZ->deleteLater();
    threadYZ->deleteLater();
}

std::string replaceString(std::string subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void MainWindow::on_actionLoadOutputHDF5File_triggered()
{
    QString _fileName = QFileDialog::getOpenFileName(this, "Open File", "", "HDF5 Files (*.h5)");

    if (_fileName != "") {
        try {
            on_actionCloseHDF5File_triggered();

            file = new HDF5File(_fileName.toStdString());

            fileName = _fileName.toStdString();
            size_t lastindex = fileName.find_last_of(".");
            std::string rawname = fileName.substr(0, lastindex);
            fileName = rawname;

            setWindowTitle(windowTitle + " - " + _fileName);

            ui->actionCloseHDF5File->setEnabled(true);
            ui->dockWidgetDatasets->setEnabled(true);
            ui->dockWidgetInfo->setEnabled(true);

            // Load info
            try {
                HDF5File::HDF5Group *group = file->openGroup("/");
                for (int i = 0; i < group->getNumAttrs(); i++) {
                    std::string value((char *) group->getAttribute(i)->getData(), group->getAttribute(i)->getSize());
                    value = replaceString(value, "\n", "<br>");
                    ui->textBrowserInfo->append(QString::fromStdString("<strong>" + group->getAttribute(i)->getName() + "</strong><br>" + value + "<br>"));
                }
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::exit(EXIT_FAILURE);
            }
            QScrollBar *v = ui->textBrowserInfo->verticalScrollBar();
            v->setValue(v->minimum());

            // Load dimensions
            nT = file->getNT();
            nX = file->getNX();
            nY = file->getNY();
            nZ = file->getNZ();

            // Clear datasets
            QLayoutItem* item;
            while ((item = ui->verticalLayoutDatasets->takeAt(0)) != NULL)
            {
                delete item->widget();
                delete item;
            }

            int count = 0;

            // Find datasets for visualization
            for (hsize_t i = 0; i < file->getNumObjs(); i++) {
                try {
                    HDF5File::HDF5Dataset *dataset = file->openDataset(i);
                    hsize_t *size = dataset->getDims();

                    // 3D type
                    if (dataset->getDataType() == H5T_FLOAT && dataset->getRank() == 3 && size[0] == nZ && size[1] == nY && size[2] == nX) {
                        std::cout << "----> 3D type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                        count++;
                        QRadioButton *rB = new QRadioButton(QString::fromStdString(dataset->getName()));
                        connect(rB, SIGNAL(clicked()), this, SLOT(selectDataset()));
                        ui->verticalLayoutDatasets->addWidget(rB, count , 0);
                    }

                    file->closeDataset(dataset->getName());

                } catch(std::exception &) {
                    std::cout << "Object " << i << " is not dataset" << std::endl;
                    // Reshaped mask type to group
                    try {
                        HDF5File::HDF5Group *group = file->openGroup(i);
                        uint64_t count = group->readAttributeI("count");
                        uint64_t posX = group->readAttributeI("positionX");
                        uint64_t posY = group->readAttributeI("positionY");
                        uint64_t posZ = group->readAttributeI("positionZ");
                        std::cout << "----> Reshaped mask type group: "<< group->getName() << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl;
                        count++;
                        QRadioButton *rB = new QRadioButton(QString::fromStdString(group->getName()));
                        connect(rB, SIGNAL(clicked()), this, SLOT(selectDataset()));
                        ui->verticalLayoutDatasets->addWidget(rB, count , 0);
                        file->closeGroup(group->getName());
                    } catch(std::exception &) {
                        std::cout << "Object " << i << " is not original reshaped group" << std::endl;
                    }
                }
            }

        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            // TODO error dialog
        }
    }
}

void MainWindow::clearGUI()
{
    fileName = "";

    nT = 0;
    nX = 0;
    nY = 0;
    nZ = 0;

    setWindowTitle(windowTitle);
    selectedDataset = NULL;
    selectedGroup = NULL;
    selectedName = "";
    datasetName = "";

    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;
    flagVRLoaded = false;

    ui->comboBoxColormap->setCurrentIndex(cv::COLORMAP_JET);

    posZ = 0;
    posY = 0;
    posX = 0;

    steps = 0;

    play = false;

    ui->dockWidgetSelectedDataset->setEnabled(false);
    //ui->dockWidget3D->setEnabled(false);
    ui->dockWidgetCT->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    ui->verticalSliderXY->setValue(0);
    ui->spinBoxXY->setValue(0);

    ui->verticalSliderXZ->setValue(0);
    ui->spinBoxXZ->setValue(0);

    ui->verticalSliderYZ->setValue(0);
    ui->spinBoxYZ->setValue(0);

    ui->checkBoxUseGlobal->setChecked(true);
    ui->toolButtonLocalValues->setChecked(false);
    ui->actionCT->setChecked(false);
    ui->actionInfo->setChecked(false);
    ui->dockWidgetCT->setVisible(false);
    ui->dockWidgetInfo->setVisible(false);

    ((CVImageWidget *) ui->imageWidgetXY)->clearImage();
    ((CVImageWidget *) ui->imageWidgetXZ)->clearImage();
    ((CVImageWidget *) ui->imageWidgetYZ)->clearImage();

    // TODO clear 3D scene
    if (gWindow != NULL) {
        gWindow->unload3DTexture();
        gWindow->clearSlices();
    }

    //ui->progressBar3D->setVisible(false);

    // Clear dataset info
    QLayoutItem* item;
    while ((item = ui->formLayoutSelectedDatasetInfo->takeAt(0)) != NULL)
    {
        delete item->widget();
        delete item;
    }
    //ui->groupBoxSelectedDatasetInfo->adjustSize();

    // Clear datasets
    while ((item = ui->verticalLayoutDatasets->takeAt(0)) != NULL)
    {
        delete item->widget();
        delete item;
    }
    ui->dockWidgetContentsDatasets->adjustSize();
    // Clear info
    ui->textBrowserInfo->clear();

    ui->actionCloseHDF5File->setEnabled(false);

}

void MainWindow::on_actionCloseHDF5File_triggered()
{
    threadXY->clearRequests();
    threadXY->wait();
    threadXZ->clearRequests();
    threadXZ->wait();
    threadYZ->clearRequests();
    threadYZ->wait();

    if (gWindow != NULL)
    {
        gWindow->getThread()->clearRequests();
        gWindow->getThread()->wait();
    }

    if (file != NULL) {
        delete file;
        file = NULL;
    }
    if (dataXY != NULL) {
        delete [] dataXY;
        dataXY = NULL;
    }
    if (dataXZ != NULL) {
        delete [] dataXZ;
        dataXZ = NULL;
    }
    if (dataYZ != NULL) {
        delete [] dataYZ;
        dataYZ = NULL;
    }

    clearGUI();

}

void MainWindow::selectDataset()
{
    threadXY->clearRequests();
    threadXY->wait();
    threadXZ->clearRequests();
    threadXZ->wait();
    threadYZ->clearRequests();
    threadYZ->wait();

    ((CVImageWidget *) ui->imageWidgetXY)->clearImage();
    ((CVImageWidget *) ui->imageWidgetXZ)->clearImage();
    ((CVImageWidget *) ui->imageWidgetYZ)->clearImage();

    if (gWindow != NULL)
    {
        gWindow->getThread()->clearRequests();
        gWindow->getThread()->wait();
        gWindow->unload3DTexture();
        gWindow->clearSlices();
    }

    //ui->progressBar3D->setVisible(false);

    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;
    flagVRLoaded = false;

    //ui->groupBoxVolumeRendering->setEnabled(false);
    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->groupBoxSelectedDatasetTMSeries->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    datasetName = "";

    posZ = 0;
    posY = 0;
    posX = 0;

    steps = 0;
    play = false;
    currentStep = 0;

    // Close old dataset or group
    try {
        selectedDataset = NULL;
        file->closeDataset(selectedName);
    } catch(std::exception &) {
        try {
            selectedGroup = NULL;
            file->closeGroup(selectedName);
        } catch(std::exception &) {

        }
    }

    // Clear dataset info
    QLayoutItem* item;
    while ((item = ui->formLayoutSelectedDatasetInfo->takeAt(0)) != NULL)
    {
        delete item->widget();
        delete item;
    }


    // Find selected dataset or group
    QList<QRadioButton *> list = ui->dockWidgetContentsDatasets->findChildren<QRadioButton *>();
    foreach(QRadioButton *rB, list) {
        if (rB->isChecked()) {
            selectedName = rB->text().toStdString();
            break;
        }
    }

    // Try open dataset
    try {
        selectedDataset = file->openDataset(selectedName);
        datasetName = selectedName;

        std::cout << "--> Selected dataset " << selectedName << std::endl;

        minVG = selectedDataset->getGlobalMinValueF();
        maxVG = selectedDataset->getGlobalMaxValueF();

        std::cout << "minVG: " << minVG << " maxVG: " << maxVG << std::endl;

        // Set dataset info
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Name:"), new QLabel(QString::fromStdString(selectedName)));
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Type:"), new QLabel(QString("3D dataset")));
        //ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Original size:"), new QLabel(QString::number(nZ) + " x " + QString::number(nY) + " x " + QString::number(nX)));
        hsize_t *size = selectedDataset->getDims();
        sizeZ = size[0];
        sizeY = size[1];
        sizeX = size[2];
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Size:"), new QLabel(QString::number(sizeZ) + " x " + QString::number(sizeY) + " x " + QString::number(sizeX)));
        size = selectedDataset->getChunkDims();
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Chunk size:"), new QLabel(QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));

        // Init min and max controls
        ui->doubleSpinBoxMinGlobal->setRange((double) minVG, (double) maxVG);
        ui->doubleSpinBoxMaxGlobal->setRange((double) minVG, (double) maxVG);
        ui->doubleSpinBoxMinGlobal->setValue((double) minVG);
        ui->doubleSpinBoxMaxGlobal->setValue((double) maxVG);
        ui->doubleSpinBoxMinGlobal->setSingleStep((maxVG - minVG) / 1000);
        ui->doubleSpinBoxMaxGlobal->setSingleStep((maxVG - minVG) / 1000);
        ui->horizontalSliderGlobalMin->setValue(ui->horizontalSliderGlobalMin->minimum());
        ui->horizontalSliderGlobalMax->setValue(ui->horizontalSliderGlobalMax->maximum());

        initSlices();

        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);

    } catch(std::exception &) {
        // Try open group
        try {
            selectedGroup = file->openGroup(selectedName);
            selectedDataset = file->openDataset(selectedName + "/" + std::to_string(0));
            datasetName = selectedDataset->getName();

            std::cout << "--> Selected group " << selectedName << std::endl;

            minVG = selectedGroup->readAttributeF("min");
            maxVG = selectedGroup->readAttributeF("max");

            std::cout << "minVG: " << minVG << " maxVG: " << maxVG << std::endl;

            steps = selectedGroup->readAttributeI("count");
            // Set series controls
            ui->spinBoxSelectedDatasetStep->setMaximum(steps-1);
            ui->spinBoxSelectedDatasetStep->setValue(0);
            ui->horizontalSliderSelectedDatasetStep->setMaximum(steps-1);
            ui->horizontalSliderSelectedDatasetStep->setValue(0);

            // Set info
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Name:"), new QLabel(QString::fromStdString(selectedName)));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Type:"), new QLabel(QString("Mask type")));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Steps:"), new QLabel(QString::number(steps)));
            sizeZ = selectedGroup->readAttributeI("sizeZ");
            sizeY = selectedGroup->readAttributeI("sizeY");
            sizeX = selectedGroup->readAttributeI("sizeX");
            posZ = selectedGroup->readAttributeI("positionZ");
            posY = selectedGroup->readAttributeI("positionY");
            posX = selectedGroup->readAttributeI("positionX");
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Size:"), new QLabel(QString::number(nZ) + " x " + QString::number(nY) + " x " + QString::number(nX)));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Mask size:"), new QLabel(QString::number(sizeZ) + " x " + QString::number(sizeY) + " x " + QString::number(sizeX)));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Position:"), new QLabel(QString::number(posZ) + " x " + QString::number(posY) + " x " + QString::number(posX)));
            hsize_t *size;
            size = selectedDataset->getChunkDims();
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Chunk size:"), new QLabel(QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));

            // Init min and max controls
            ui->doubleSpinBoxMinGlobal->setRange((double) minVG, (double) maxVG);
            ui->doubleSpinBoxMaxGlobal->setRange((double) minVG, (double) maxVG);
            ui->doubleSpinBoxMinGlobal->setValue((double) minVG);
            ui->doubleSpinBoxMaxGlobal->setValue((double) maxVG);
            ui->doubleSpinBoxMinGlobal->setSingleStep((maxVG - minVG) / 100);
            ui->doubleSpinBoxMaxGlobal->setSingleStep((maxVG - minVG) / 100);
            ui->horizontalSliderGlobalMin->setValue(ui->horizontalSliderGlobalMin->minimum());
            ui->horizontalSliderGlobalMax->setValue(ui->horizontalSliderGlobalMax->maximum());

            initSlices();

            ui->dockWidgetSelectedDataset->setEnabled(true);
            ui->groupBoxSelectedDatasetTMSeries->setEnabled(true);
            ui->dockWidgetXY->setEnabled(true);
            ui->dockWidgetXZ->setEnabled(true);
            ui->dockWidgetYZ->setEnabled(true);

        } catch(std::exception &) {
            std::cerr << "Object " << selectedName << " is wrong" << std::endl;
        }
    }
}

void MainWindow::initSlices()
{
    if (selectedDataset != NULL) {
        //ui->dockWidgetXY->setWindowTitle("XY slice (Z = 0)");
        //ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = 0)");
        //ui->dockWidgetYZ->setWindowTitle("YZ slice (X = 0)");

        // Set vertical sliders controls
        ui->verticalSliderXY->setMaximum(sizeZ-1);
        //ui->verticalSliderXY->setValue(0);
        ui->spinBoxXY->setMaximum(sizeZ-1);
        //ui->spinBoxXY->setValue(0);

        ui->verticalSliderXZ->setMaximum(sizeY-1);
        //ui->verticalSliderXZ->setValue(0);
        ui->spinBoxXZ->setMaximum(sizeY-1);
        //ui->spinBoxXZ->setValue(0);

        ui->verticalSliderYZ->setMaximum(sizeX-1);
        //ui->verticalSliderYZ->setValue(0);
        ui->spinBoxYZ->setMaximum(sizeX-1);
        //ui->spinBoxYZ->setValue(0);

        flagDatasetInitialized = true;
        loadXYSlice(ui->verticalSliderXY->value());
        loadXZSlice(ui->verticalSliderXZ->value());
        loadYZSlice(ui->verticalSliderYZ->value());

        if (gWindow != NULL) {
            gWindow->changeMinValue(ui->doubleSpinBoxMinGlobal->value());
            gWindow->changeMaxValue(ui->doubleSpinBoxMaxGlobal->value());
            gWindow->changeColormap(ui->comboBoxColormap->currentIndex());
            gWindow->setMainSize(nZ, nY, nX);
            gWindow->setSize(sizeZ, sizeY, sizeX);
            gWindow->setPosition(posZ, posY, posX);
        }

        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            gWindow->load3DTexture(selectedDataset, minVG, maxVG, ui->comboBoxColormap->currentIndex());
        }
    }
}

// Loading slices

void MainWindow::loadXYSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        ui->labelXYLoading->setMovie(movie);
        threadXY->setParams(selectedDataset, index, 0, 0, 1, sizeY, sizeX);
        //if (!threadXY->isRunning())
        threadXY->start();
    }
}

void MainWindow::setXYLoaded(hsize_t zO, hsize_t, hsize_t, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{
    ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(zO) + ")");
    flagXYloaded = false;
    delete [] dataXY;
    dataXY = NULL;

    if (zO == ui->verticalSliderXY->value()) ui->labelXYLoading->clear();

    // TODO mutex
    hsize_t size = zC * yC * xC;
    dataXY = new float[size];
    memcpy(dataXY, data, size * sizeof(float));
    delete [] data;
    data = NULL;

    minVXY = min;
    maxVXY = max;

    // Init local controls
    ui->doubleSpinBoxXYMin->setRange((double) minVXY, (double) maxVXY);
    ui->doubleSpinBoxXYMax->setRange((double) minVXY, (double) maxVXY);
    ui->doubleSpinBoxXYMin->setValue((double) minVXY);
    ui->doubleSpinBoxXYMax->setValue((double) maxVXY);
    ui->doubleSpinBoxXYMin->setSingleStep((maxVXY - minVXY) / 1000);
    ui->doubleSpinBoxXYMax->setSingleStep((maxVXY - minVXY) / 1000);

    ui->horizontalSliderXYMin->setValue(ui->horizontalSliderXYMin->minimum());
    ui->horizontalSliderXYMax->setValue(ui->horizontalSliderXYMax->maximum());

    flagXYloaded = true;
    setImageXYFromData();
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
        timer->start(ui->spinBoxTMInterval->value());
}

void MainWindow::loadXZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        ui->labelXZLoading->setMovie(movie);
        threadXZ->setParams(selectedDataset, 0, index, 0, sizeZ, 1, sizeX);
        //if (!threadXZ->isRunning())
        threadXZ->start();
    }
}

void MainWindow::setXZLoaded(hsize_t, hsize_t yO, hsize_t, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{
    ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(yO) + ")");
    flagXZloaded = false;
    delete [] dataXZ;
    dataXZ = NULL;

    if (yO == ui->verticalSliderXZ->value()) ui->labelXZLoading->clear();

    hsize_t size = zC * yC * xC;
    dataXZ = new float[size];
    std::copy(data, data + size, dataXZ);
    delete [] data;
    data = NULL;

    minVXZ = min;
    maxVXZ = max;

    ui->doubleSpinBoxXZMin->setRange((double) minVXZ, (double) maxVXZ);
    ui->doubleSpinBoxXZMax->setRange((double) minVXZ, (double) maxVXZ);
    ui->doubleSpinBoxXZMin->setValue((double) minVXZ);
    ui->doubleSpinBoxXZMax->setValue((double) maxVXZ);
    ui->doubleSpinBoxXZMin->setSingleStep((maxVXZ - minVXZ) / 1000);
    ui->doubleSpinBoxXZMax->setSingleStep((maxVXZ - minVXZ) / 1000);

    ui->horizontalSliderXZMin->setValue(ui->horizontalSliderXZMin->minimum());
    ui->horizontalSliderXZMax->setValue(ui->horizontalSliderXZMax->maximum());

    flagXZloaded = true;
    setImageXZFromData();
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
        timer->start(ui->spinBoxTMInterval->value());
}

void MainWindow::loadYZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        ui->labelYZLoading->setMovie(movie);
        threadYZ->setParams(selectedDataset, 0, 0, index, sizeZ, sizeY, 1);
        //if (!threadYZ->isRunning())
        threadYZ->start();
    }
}

void MainWindow::setYZLoaded(hsize_t, hsize_t, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{
    ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(xO) + ")");
    flagYZloaded = false;
    delete [] dataYZ;
    dataYZ = NULL;

    if (xO == ui->verticalSliderYZ->value()) ui->labelYZLoading->clear();

    hsize_t size = zC * yC * xC;
    dataYZ = new float[size];
    std::copy(data, data + size, dataYZ);
    delete [] data;
    data = NULL;

    minVYZ = min;
    maxVYZ = max;

    ui->doubleSpinBoxYZMin->setRange((double) minVYZ, (double) maxVYZ);
    ui->doubleSpinBoxYZMax->setRange((double) minVYZ, (double) maxVYZ);
    ui->doubleSpinBoxYZMin->setValue((double) minVYZ);
    ui->doubleSpinBoxYZMax->setValue((double) maxVYZ);
    ui->doubleSpinBoxYZMin->setSingleStep((maxVYZ - minVYZ) / 1000);
    ui->doubleSpinBoxYZMax->setSingleStep((maxVYZ - minVYZ) / 1000);

    ui->horizontalSliderYZMin->setValue(ui->horizontalSliderYZMin->minimum());
    ui->horizontalSliderYZMax->setValue(ui->horizontalSliderYZMax->maximum());

    flagYZloaded = true;
    setImageYZFromData();
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
        timer->start(ui->spinBoxTMInterval->value());
}

// 3D data loaded event

void MainWindow::loaded3D(std::string _datasetName)
{
    flagVRLoaded = true;
    //ui->progressBar3D->setVisible(false);
    //ui->progressBar3D->setValue(0);
    // If animation is running...
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && flagVRLoaded && _datasetName == datasetName)
        timer->start(ui->spinBoxTMInterval->value());
}

// Set new images from loaded slices

void MainWindow::setImageXYFromData()
{
    if (flagXYloaded) {
        if (gWindow != NULL) {
            if (sizeZ == 1)
                gWindow->setXYSlice(dataXY, sizeX, sizeY, (float) 0.0);
            else
                gWindow->setXYSlice(dataXY, sizeX, sizeY, (float) ui->verticalSliderXY->value() / (sizeZ - 1));
        }
        cv::Mat image = cv::Mat(sizeY, sizeX, CV_32FC1, dataXY); // rows, cols (height, width)
        if (ui->checkBoxUseGlobal->isChecked())
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXY - minVXY), - minVXY * 255.0 /(maxVXY - minVXY));
        cv::applyColorMap(image, image, ui->comboBoxColormap->currentIndex());
        cv::flip(image, image, 0);
        QPoint p = QPoint(posX, posY);
        if (!ui->toolButtonPositionXY->isChecked())
            p = QPoint(0, 0);
        ((CVImageWidget *) ui->imageWidgetXY)->showImage(image, p, ui->toolButtonFillXY->isChecked(), fileName + "_-_" + replaceString(datasetName, "/", "-") + "_-_XY_" + std::to_string(ui->verticalSliderXY->value()));
    }
}

void MainWindow::setImageXZFromData()
{
    if (flagXZloaded) {
        if (gWindow != NULL) {
            if (sizeY == 1)
                gWindow->setXZSlice(dataXZ, sizeX, sizeZ, (float) 0);
            else
                gWindow->setXZSlice(dataXZ, sizeX, sizeZ, (float) ui->verticalSliderXZ->value() / (sizeY - 1));
        }
        cv::Mat image = cv::Mat(sizeZ, sizeX, CV_32FC1, dataXZ); // rows, cols (height, width)
        if (ui->checkBoxUseGlobal->isChecked())
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXZ - minVXZ), - minVXZ * 255.0 /(maxVXZ - minVXZ));
        cv::applyColorMap(image, image, ui->comboBoxColormap->currentIndex());
        QPoint p = QPoint(posX, posZ);
        if (!ui->toolButtonPositionXZ->isChecked())
            p = QPoint(0, 0);
        ((CVImageWidget *) ui->imageWidgetXZ)->showImage(image, p, ui->toolButtonFillXZ->isChecked(), fileName + "_-_" + replaceString(datasetName, "/", "-") + "_-_XZ_" + std::to_string(ui->verticalSliderXZ->value()));
    }
}

void MainWindow::setImageYZFromData()
{
    if (flagYZloaded) {
        if (gWindow != NULL) {
            if (sizeX == 1)
                gWindow->setYZSlice(dataYZ, sizeY, sizeZ, (float) 0);
            else
                gWindow->setYZSlice(dataYZ, sizeY, sizeZ, (float) ui->verticalSliderYZ->value() / (sizeX - 1));

        }
        cv::Mat image = cv::Mat(sizeZ, sizeY, CV_32FC1, dataYZ); // rows, cols (height, width)
        if (ui->checkBoxUseGlobal->isChecked())
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVYZ - minVYZ), - minVYZ * 255.0 /(maxVYZ - minVYZ));
        cv::applyColorMap(image, image, ui->comboBoxColormap->currentIndex());
        cv::transpose(image, image);
        cv::flip(image, image, 0);
        cv::flip(image, image, 1);
        QPoint p = QPoint(posY, posZ);
        if (!ui->toolButtonPositionYZ->isChecked())
            p = QPoint(0, 0);
        ((CVImageWidget *) ui->imageWidgetYZ)->showImage(image, p, ui->toolButtonFillYZ->isChecked(), fileName + "_-_" + replaceString(datasetName, "/", "-") + "_-_YZ_" + std::to_string(ui->verticalSliderYZ->value()));
    }
}

// Repaint slices

void MainWindow::repaintSlices()
{
    setImageXYFromData();
    setImageXZFromData();
    setImageYZFromData();
    // TODO
}

// Slices indices change

void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    loadXYSlice(value);
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    loadXZSlice(value);
}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    loadYZSlice(value);
}



void MainWindow::on_horizontalSliderCTAlpha_valueChanged(int value)
{
    ui->doubleSpinBoxCTAlpha->setValue((double) value / 1000);
}

void MainWindow::on_doubleSpinBoxCTAlpha_valueChanged(double value)
{
    ui->horizontalSliderCTAlpha->setValue(value * 1000);
}

// Visibility of docked panels

void MainWindow::on_dockWidgetXY_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetXY->isVisible())
        ui->actionXY->setChecked(true);
    else
        ui->actionXY->setChecked(false);
}

void MainWindow::on_dockWidgetXZ_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetXZ->isVisible())
        ui->actionXZ->setChecked(true);
    else
        ui->actionXZ->setChecked(false);
}

void MainWindow::on_dockWidgetYZ_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetYZ->isVisible())
        ui->actionYZ->setChecked(true);
    else
        ui->actionYZ->setChecked(false);
}

void MainWindow::on_dockWidgetInfo_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetInfo->isVisible())
        ui->actionInfo->setChecked(true);
    else
        ui->actionInfo->setChecked(false);
}

void MainWindow::on_dockWidgetDatasets_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetDatasets->isVisible())
        ui->actionDatasets->setChecked(true);
    else
        ui->actionDatasets->setChecked(false);
}

void MainWindow::on_dockWidgetCT_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetCT->isVisible())
        ui->actionCT->setChecked(true);
    else
        ui->actionCT->setChecked(false);
}

void MainWindow::on_dockWidgetSelectedDataset_visibilityChanged(bool /*visible*/)
{
    if (ui->dockWidgetSelectedDataset->isVisible())
        ui->actionSelectedDataset->setChecked(true);
    else
        ui->actionSelectedDataset->setChecked(false);
}

// Use global values settings

void MainWindow::on_checkBoxUseGlobal_clicked(bool /*checked*/)
{
    //flagUseGlobalValues = checked;
    repaintSlices();
}

// Global min and max values change

void MainWindow::on_horizontalSliderGlobalMin_valueChanged(int value)
{
    if (flagDatasetInitialized)
        ui->doubleSpinBoxMinGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    if (flagDatasetInitialized)
        ui->doubleSpinBoxMaxGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    if (flagDatasetInitialized) {
        ui->horizontalSliderGlobalMin->setTracking(false);
        ui->horizontalSliderGlobalMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
        ui->horizontalSliderGlobalMin->setTracking(true);
        //setMinVG(value);
        minVG = value;
        if (gWindow != NULL)
            gWindow->changeMinValue(value);
        repaintSlices();
    }
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (flagDatasetInitialized) {
        ui->horizontalSliderGlobalMax->setTracking(false);
        ui->horizontalSliderGlobalMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
        ui->horizontalSliderGlobalMax->setTracking(true);
        //setMaxVG(value);
        maxVG = value;
        if (gWindow != NULL)
            gWindow->changeMaxValue(value);
        repaintSlices();
    }
}

// Local min and max values change

void MainWindow::on_horizontalSliderXYMin_valueChanged(int value)
{
    if (flagXYloaded)
        ui->doubleSpinBoxXYMin->setValue((double) value / 1000 * (ui->doubleSpinBoxXYMin->maximum() - ui->doubleSpinBoxXYMin->minimum()) + ui->doubleSpinBoxXYMin->minimum());
}

void MainWindow::on_doubleSpinBoxXYMin_valueChanged(double value)
{
    if (flagXYloaded) {
        ui->horizontalSliderXYMin->setTracking(false);
        ui->horizontalSliderXYMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXYMin->minimum()) / (ui->doubleSpinBoxXYMin->maximum() - ui->doubleSpinBoxXYMin->minimum())));
        ui->horizontalSliderXYMin->setTracking(true);
        minVXY = value;
        setImageXYFromData();
    }
}

void MainWindow::on_horizontalSliderXYMax_valueChanged(int value)
{
    if (flagXYloaded)
        ui->doubleSpinBoxXYMax->setValue((double) value / 1000 * (ui->doubleSpinBoxXYMax->maximum() - ui->doubleSpinBoxXYMax->minimum()) + ui->doubleSpinBoxXYMax->minimum());
}

void MainWindow::on_doubleSpinBoxXYMax_valueChanged(double value)
{
    if (flagXYloaded) {
        ui->horizontalSliderXYMax->setTracking(false);
        ui->horizontalSliderXYMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXYMax->minimum()) / (ui->doubleSpinBoxXYMax->maximum() - ui->doubleSpinBoxXYMax->minimum())));
        ui->horizontalSliderXYMax->setTracking(true);
        maxVXY = value;
        setImageXYFromData();
    }
}

void MainWindow::on_horizontalSliderXZMin_valueChanged(int value)
{
    if (flagXZloaded)
        ui->doubleSpinBoxXZMin->setValue((double) value / 1000 * (ui->doubleSpinBoxXZMin->maximum() - ui->doubleSpinBoxXZMin->minimum()) + ui->doubleSpinBoxXZMin->minimum());
}

void MainWindow::on_doubleSpinBoxXZMin_valueChanged(double value)
{
    if (flagXZloaded) {
        ui->horizontalSliderXZMin->setTracking(false);
        ui->horizontalSliderXZMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXZMin->minimum()) / (ui->doubleSpinBoxXZMin->maximum() - ui->doubleSpinBoxXZMin->minimum())));
        ui->horizontalSliderXZMin->setTracking(true);
        minVXZ = value;
        setImageXZFromData();
    }
}

void MainWindow::on_horizontalSliderXZMax_valueChanged(int value)
{
    if (flagXZloaded)
        ui->doubleSpinBoxXZMax->setValue((double) value / 1000 * (ui->doubleSpinBoxXZMax->maximum() - ui->doubleSpinBoxXZMax->minimum()) + ui->doubleSpinBoxXZMax->minimum());
}

void MainWindow::on_doubleSpinBoxXZMax_valueChanged(double value)
{
    if (flagXZloaded) {
        ui->horizontalSliderXZMax->setTracking(false);
        ui->horizontalSliderXZMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxXZMax->minimum()) / (ui->doubleSpinBoxXZMax->maximum() - ui->doubleSpinBoxXZMax->minimum())));
        ui->horizontalSliderXZMax->setTracking(true);
        maxVXZ = value;
        setImageXZFromData();
    }
}

void MainWindow::on_horizontalSliderYZMin_valueChanged(int value)
{
    if (flagYZloaded)
        ui->doubleSpinBoxYZMin->setValue((double) value / 1000 * (ui->doubleSpinBoxYZMin->maximum() - ui->doubleSpinBoxYZMin->minimum()) + ui->doubleSpinBoxYZMin->minimum());
}

void MainWindow::on_doubleSpinBoxYZMin_valueChanged(double value)
{
    if (flagYZloaded) {
        ui->horizontalSliderYZMin->setTracking(false);
        ui->horizontalSliderYZMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxYZMin->minimum()) / (ui->doubleSpinBoxYZMin->maximum() - ui->doubleSpinBoxYZMin->minimum())));
        ui->horizontalSliderYZMin->setTracking(true);
        minVYZ = value;
        setImageYZFromData();
    }
}

void MainWindow::on_horizontalSliderYZMax_valueChanged(int value)
{
    if (flagYZloaded)
        ui->doubleSpinBoxYZMax->setValue((double) value / 1000 * (ui->doubleSpinBoxYZMax->maximum() - ui->doubleSpinBoxYZMax->minimum()) + ui->doubleSpinBoxYZMax->minimum());
}

void MainWindow::on_doubleSpinBoxYZMax_valueChanged(double value)
{
    if (flagYZloaded) {
        ui->horizontalSliderYZMax->setTracking(false);
        ui->horizontalSliderYZMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxYZMax->minimum()) / (ui->doubleSpinBoxYZMax->maximum() - ui->doubleSpinBoxYZMax->minimum())));
        ui->horizontalSliderYZMax->setTracking(true);
        maxVYZ = value;
        setImageYZFromData();
    }
}

// Colormap change

void MainWindow::on_comboBoxColormap_currentIndexChanged(int index)
{
    if (gWindow != NULL)
        gWindow->changeColormap(index);
    repaintSlices();
}

// time series buttons

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    currentStep = step;
    if (selectedGroup != NULL) {
        try {
            threadXY->clearRequests();
            threadXY->wait();
            threadXZ->clearRequests();
            threadXZ->wait();
            threadYZ->clearRequests();
            threadYZ->wait();
            if (gWindow != NULL && ui->actionVolumeRendering->isChecked())
            {
                gWindow->getThread()->clearRequests();
                gWindow->getThread()->wait();
            }
            file->closeDataset(selectedDataset->getName());
            selectedDataset = file->openDataset(selectedName + "/" + std::to_string(step));
            datasetName = selectedName + "/" + std::to_string(step);

            flagVRLoaded = false;

            if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
                //ui->progressBar3D->setValue(0);
                //ui->progressBar3D->setVisible(true);
                gWindow->load3DTexture(selectedDataset, minVG, maxVG, ui->comboBoxColormap->currentIndex());
            }
            loadXYSlice(ui->verticalSliderXY->value());
            loadXZSlice(ui->verticalSliderXZ->value());
            loadYZSlice(ui->verticalSliderYZ->value());
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

void MainWindow::updateStep()
{
    currentStep += ui->spinBoxTMIncrement->value();
    if (currentStep >= steps) {
        timer->stop();
        play = false;
        ui->toolButtonPlay->setChecked(false);
        currentStep = 0;
    } else {
        timer->stop();
        ui->horizontalSliderSelectedDatasetStep->setValue(currentStep);
    }
}

void MainWindow::on_toolButtonPlay_clicked(bool checked)
{
    if (checked) {
        timer->start(ui->spinBoxTMInterval->value());
        play = true;
    } else {
        timer->stop();
        play = false;
        //ui->toolButtonPlay->setChecked(false);
    }
}

void MainWindow::on_toolButtonStart_clicked()
{
    timer->stop();
    play = false;
    ui->toolButtonPlay->setChecked(false);
    currentStep = 0;
    ui->horizontalSliderSelectedDatasetStep->setValue(currentStep);
}

void MainWindow::on_toolButtonEnd_clicked()
{
    timer->stop();
    play = false;
    ui->toolButtonPlay->setChecked(false);
    currentStep = steps-1;
    ui->horizontalSliderSelectedDatasetStep->setValue(currentStep);
}

void MainWindow::on_spinBoxTMInterval_valueChanged(int value)
{
    timer->setInterval(value);
}

// Select point on slices

void MainWindow::on_imageWidgetXY_hoveredPointInImage(int x, int y)
{
    if (flagXYloaded) {
        float value = dataXY[x + sizeX * (sizeY - y)];
        //qDebug() << x << " " << y << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

void MainWindow::on_imageWidgetXZ_hoveredPointInImage(int x, int z)
{
    if (flagXZloaded) {
        float value = dataXZ[x + sizeX * z];
        //qDebug() << x << " " << z << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

void MainWindow::on_imageWidgetYZ_hoveredPointInImage(int y, int z)
{
    if (flagYZloaded) {
        float value = dataYZ[(sizeZ - z) + sizeZ * (sizeY - y)];
        //qDebug() << y << " " << z << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

// Toolbar 3D buttons

void MainWindow::on_actionVolumeRendering_toggled(bool value)
{
    if (gWindow != NULL)
        gWindow->setViewVR(value);
    if (gWindow != NULL && selectedDataset != NULL && ui->actionVolumeRendering->isChecked() && !gWindow->isTexture3DInitialized()) {
        //ui->progressBar3D->setValue(0);
        //ui->progressBar3D->setVisible(true);
        gWindow->load3DTexture(selectedDataset, minVG, maxVG, ui->comboBoxColormap->currentIndex());
    }
}

void MainWindow::on_action3DXY_toggled(bool value)
{
    if (gWindow != NULL)
        gWindow->setViewXYSlice(value);
}

void MainWindow::on_action3DXZ_toggled(bool value)
{
    if (gWindow != NULL)
        gWindow->setViewXZSlice(value);
}

void MainWindow::on_action3DYZ_toggled(bool value)
{
    if (gWindow != NULL)
        gWindow->setViewYZSlice(value);
}

void MainWindow::on_actionExportImageFrom3DScene_triggered()
{
    if (gWindow != NULL) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", QString::fromStdString(replaceString(datasetName, "/", "-") + "_3Dscene.png"), "Image (*.png)");
        if (fileName != NULL)
            gWindow->saveImage(fileName);
    }
}

void MainWindow::on_checkBoxVRFrame_clicked(bool checked)
{
    if (gWindow != NULL) {
        gWindow->setViewFrame(checked);
    }
}

void MainWindow::on_horizontalSliderVRSlices_valueChanged(int value)
{
    if (gWindow != NULL) {
        gWindow->setSlicesCount(value);
    }
}

void MainWindow::on_horizontalSliderVRAlpha_valueChanged(int value)
{
    if (gWindow != NULL) {
        gWindow->setAlpha(value);
    }
    ui->doubleSpinBoxVRAlpha->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRRed_valueChanged(int value)
{
    if (gWindow != NULL) {
        gWindow->setRed(value);
    }
    ui->doubleSpinBoxVRRed->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRGreen_valueChanged(int value)
{
    if (gWindow != NULL) {
        gWindow->setGreen(value);
    }
    ui->doubleSpinBoxVRGreen->setValue((double) value / 1000);
}

void MainWindow::on_horizontalSliderVRBlue_valueChanged(int value)
{
    if (gWindow != NULL) {
        gWindow->setBlue(value);
    }
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
