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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    file = NULL;
    fileName = "";
    windowTitle = "k-Wave HDF5 visualizer";
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
    //flagUseGlobalValues = false;

    flagvRLoaded = false;

    dataXY = NULL;
    dataXZ = NULL;
    dataYZ = NULL;

    ui->dockWidgetSelectedDataset->setEnabled(false);
    //ui->dockWidget3D->setEnabled(false);
    ui->dockWidgetCT->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);


    ui->checkBoxUseGlobal->setChecked(true);
    ui->toolButtonLocalValues->setChecked(false);
    ui->actionCT->setChecked(false);
    ui->actionInfo->setChecked(false);
    ui->dockWidgetCT->setVisible(false);
    ui->dockWidgetInfo->setVisible(false);

    //tabifyDockWidget(ui->dockWidgetCT, ui->dockWidgetInfo);

    currentColormap = cv::COLORMAP_JET;

    posZ = 0;
    posY = 0;
    posX = 0;

    steps = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));
    increment = ui->spinBoxTMIncrement->value();
    interval = ui->spinBoxTMInterval->value();
    play = false;
    //thread = NULL;
    //threadPool = NULL;
    qRegisterMetaType<hsize_t>("hsize_t");
    threadXY = new HDF5ReadingThread();
    connect(threadXY, SIGNAL(sliceLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setXYLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));
    threadXZ = new HDF5ReadingThread();
    connect(threadXZ, SIGNAL(sliceLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setXZLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));
    threadYZ = new HDF5ReadingThread();
    connect(threadYZ, SIGNAL(sliceLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setYZLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));
    //threadXY->start();


    gWindow = NULL;
    gWindow = new GWindow();
    QWidget *widget3D = createWindowContainer(gWindow);
    ui->dockWidgetContents3D->layout()->addWidget(widget3D);
    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));

    connect(gWindow, SIGNAL(loaded(std::string)), this, SLOT(loaded3D(std::string)));

}

MainWindow::~MainWindow()
{
    on_actionCloseHDF5File_triggered();
    delete ui;
    //if (file != NULL)
    //    delete file;
    delete timer;
    //thread->terminate();
    //thread->wait();
    //threadPool->waitForDone();
    //delete thread;
    //delete threadXY;
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

            fileName = _fileName.toStdString();
            size_t lastindex = fileName.find_last_of(".");
            std::string rawname = fileName.substr(0, lastindex);
            fileName = rawname;

            file = new HDF5File(_fileName.toStdString());
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

            // FInd datasets for visualization
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
        }
    }
}

void MainWindow::on_actionCloseHDF5File_triggered()
{
    threadXY->clearRequests();
    threadXY->wait();
    threadXZ->clearRequests();
    threadXZ->wait();
    threadYZ->clearRequests();
    threadYZ->wait();

    fileName = "";

    if (file != NULL) {
        delete file;
        file = NULL;
    }

    ui->dockWidgetSelectedDataset->setEnabled(false);

    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->groupBoxSelectedDatasetTMSeries->setEnabled(false);
    ui->groupBoxVolumeRendering->setEnabled(false);
    //ui->dockWidget3D->setEnabled(false);
    ui->dockWidgetCT->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

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

    ((CVImageWidget *) ui->imageWidgetXY)->clearImage();
    ((CVImageWidget *) ui->imageWidgetXZ)->clearImage();
    ((CVImageWidget *) ui->imageWidgetYZ)->clearImage();

    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;
    //flagUseGlobalValues = false;
    //ui->checkBoxUseGlobal->setChecked(false);

    selectedDataset = NULL;
    selectedGroup = NULL;
    selectedName = "";
    datasetName = "";

    nT = 0;
    nX = 0;
    nY = 0;
    nZ = 0;

    steps = 0;

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
    setWindowTitle(windowTitle);
}

void MainWindow::selectDataset()
{
    threadXY->clearRequests();
    threadXY->wait();
    threadXZ->clearRequests();
    threadXZ->wait();
    threadYZ->clearRequests();
    threadYZ->wait();

    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;

    ui->groupBoxSelectedDatasetTMSeries->setEnabled(false);
    ui->groupBoxVolumeRendering->setEnabled(false);
    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    datasetName = "";

    posZ = 0;
    posY = 0;
    posX = 0;

    play = false;

    currentStep = 0;

    currentXYloadedFlag = false;
    currentXZloadedFlag = false;
    currentYZloadedFlag = false;

    // Find selected
    QList<QRadioButton *> list = ui->dockWidgetContentsDatasets->findChildren<QRadioButton *>();
    foreach(QRadioButton *rB, list) {
        if (rB->isChecked()) {
            selectedName = rB->text().toStdString();
            break;
        }
    }

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

    try {
        selectedDataset = file->openDataset(selectedName);
        datasetName = selectedName;
        std::cout << "--> Selected dataset " << selectedName << std::endl;
        minVG = selectedDataset->getGlobalMinValueF();
        maxVG = selectedDataset->getGlobalMaxValueF();
        std::cout << "minVG: " << minVG << " maxVG: " << maxVG << std::endl;

        // Clear info
        QLayoutItem* item;
        while ((item = ui->formLayoutSelectedDatasetInfo->takeAt( 0 )) != NULL)
        {
            delete item->widget();
            delete item;
        }
        //ui->groupBoxSelectedDatasetInfo->adjustSize();
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Name:"), new QLabel(QString::fromStdString(selectedName)));
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Type:"), new QLabel(QString("3D dataset")));

        hsize_t *size = selectedDataset->getDims();
        sizeZ = size[0];
        sizeY = size[1];
        sizeX = size[2];
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Size:"), new QLabel(QString::number(sizeZ) + " x " + QString::number(sizeY) + " x " + QString::number(sizeX)));
        size = selectedDataset->getChunkDims();
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Chunk size:"), new QLabel(QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));

        // TODO set min max sliders
        ui->doubleSpinBoxMinGlobal->setRange((double) minVG, (double) maxVG);
        ui->doubleSpinBoxMaxGlobal->setRange((double) minVG, (double) maxVG);
        ui->doubleSpinBoxMinGlobal->setValue((double) minVG);
        ui->doubleSpinBoxMaxGlobal->setValue((double) maxVG);
        ui->doubleSpinBoxMinGlobal->setSingleStep((maxVG - minVG) / 1000);
        ui->doubleSpinBoxMaxGlobal->setSingleStep((maxVG - minVG) / 1000);

        ui->horizontalSliderGlobalMin->setValue(ui->horizontalSliderGlobalMin->minimum());
        ui->horizontalSliderGlobalMax->setValue(ui->horizontalSliderGlobalMax->maximum());

        //file->closeDataset(selectedDataset->getName());

        initSlices();

        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);

    } catch(std::exception &) {
        try {
            selectedGroup = file->openGroup(selectedName);
            std::cout << "--> Selected group " << selectedName << std::endl;
            minVG = selectedGroup->readAttributeF("min");
            maxVG = selectedGroup->readAttributeF("max");
            std::cout << "minVG: " << minVG << " maxVG: " << maxVG << std::endl;

            // Clear info
            QLayoutItem* item;
            while ((item = ui->formLayoutSelectedDatasetInfo->takeAt( 0 )) != NULL)
            {
                delete item->widget();
                delete item;
            }

            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Name:"), new QLabel(QString::fromStdString(selectedName)));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Type:"), new QLabel(QString("Mask type")));

            steps = selectedGroup->readAttributeI("count");

            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Steps:"), new QLabel(QString::number(steps)));

            sizeZ = selectedGroup->readAttributeI("sizeZ");
            sizeY = selectedGroup->readAttributeI("sizeY");
            sizeX = selectedGroup->readAttributeI("sizeX");
            posZ = selectedGroup->readAttributeI("positionZ");
            posY = selectedGroup->readAttributeI("positionY");
            posX = selectedGroup->readAttributeI("positionX");
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Size:"), new QLabel(QString::number(sizeZ) + " x " + QString::number(sizeY) + " x " + QString::number(sizeX)));
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Position:"), new QLabel(QString::number(posZ) + " x " + QString::number(posY) + " x " + QString::number(posX)));

            selectedDataset = file->openDataset(selectedName + "/" + std::to_string(0));

            datasetName = selectedDataset->getName();

            ui->spinBoxSelectedDatasetStep->setMaximum(steps-1);
            ui->spinBoxSelectedDatasetStep->setValue(0);
            ui->horizontalSliderSelectedDatasetStep->setMaximum(steps-1);
            ui->horizontalSliderSelectedDatasetStep->setValue(0);

            hsize_t *size;
            size = selectedDataset->getChunkDims();
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Chunk size:"), new QLabel(QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));

            // TODO set min max sliders
            ui->doubleSpinBoxMinGlobal->setRange((double) minVG, (double) maxVG);
            ui->doubleSpinBoxMaxGlobal->setRange((double) minVG, (double) maxVG);
            ui->doubleSpinBoxMinGlobal->setValue((double) minVG);
            ui->doubleSpinBoxMaxGlobal->setValue((double) maxVG);
            ui->doubleSpinBoxMinGlobal->setSingleStep((maxVG - minVG) / 100);
            ui->doubleSpinBoxMaxGlobal->setSingleStep((maxVG - minVG) / 100);

            ui->horizontalSliderGlobalMin->setValue(ui->horizontalSliderGlobalMin->minimum());
            ui->horizontalSliderGlobalMax->setValue(ui->horizontalSliderGlobalMax->maximum());

            //file->closeDataset(datasetName);

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
    if (datasetName != "") {
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = 0)");
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = 0)");
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = 0)");
        std::cout << "Dataset size: " << sizeZ << " x " << sizeY << " x " << sizeX << std::endl;

        ui->verticalSliderXY->setMaximum(sizeZ-1);
        ui->verticalSliderXY->setValue(0);
        ui->spinBoxXY->setMaximum(sizeZ-1);
        ui->spinBoxXY->setValue(0);

        ui->verticalSliderXZ->setMaximum(sizeY-1);
        ui->verticalSliderXZ->setValue(0);
        ui->spinBoxXZ->setMaximum(sizeY-1);
        ui->spinBoxXZ->setValue(0);

        ui->verticalSliderYZ->setMaximum(sizeX-1);
        ui->verticalSliderYZ->setValue(0);
        ui->spinBoxYZ->setMaximum(sizeX-1);
        ui->spinBoxYZ->setValue(0);

        flagDatasetInitialized = true;
        flagvRLoaded = false;
        loadXYSlice(0);
        loadXZSlice(0);
        loadYZSlice(0);

        if (gWindow != NULL) {
            gWindow->changeMinValue(ui->doubleSpinBoxMinGlobal->value());
            gWindow->changeMinValue(ui->doubleSpinBoxMinGlobal->value());
        }

        if (gWindow != NULL && ui->actionVolumeRendering->isChecked())
            gWindow->load3DTexture(selectedDataset, minVG, maxVG, currentColormap);

    } else if (selectedGroup != NULL) {

    } else {
        //clear
    }
}

void MainWindow::loadXYSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        threadXY->setParams(selectedDataset, index, 0, 0, 1, sizeY, sizeX);
        if (!threadXY->isRunning())
            threadXY->start();
    }
}

void MainWindow::setXYLoaded(hsize_t zO, hsize_t, hsize_t, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{
    //if (!currentXYloadedFlag) {
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(zO) + ")");
        //if (zO == ui->verticalSliderXY->value())
        //    currentXYloadedFlag = true;
        flagXYloaded = false;
        delete [] dataXY;
        dataXY = NULL;

        // TODO mutex
        hsize_t size = zC * yC * xC;
        dataXY = new float[size];
        std::copy(data, data + size, dataXY);
        delete [] data;
        data = NULL;

        minVXY = min;
        maxVXY = max;

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
        if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagvRLoaded))
            timer->start(interval);
    //}
}

void MainWindow::loadXZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        threadXZ->setParams(selectedDataset, 0, index, 0, sizeZ, 1, sizeX);
        if (!threadXZ->isRunning())
            threadXZ->start();
    }
}

void MainWindow::setXZLoaded(hsize_t, hsize_t yO, hsize_t, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{

    //if (!currentXZloadedFlag) {
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(yO) + ")");
        //if (yO == ui->verticalSliderXZ->value())
        //    currentXZloadedFlag = true;
        flagXZloaded = false;
        delete [] dataXZ;
        dataXZ = NULL;

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
        if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagvRLoaded))
            timer->start(interval);
    //}
}

void MainWindow::loadYZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        threadYZ->setParams(selectedDataset, 0, 0, index, sizeZ, sizeY, 1);
        if (!threadYZ->isRunning())
            threadYZ->start();
    }
}

void MainWindow::setYZLoaded(hsize_t, hsize_t, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float *data, float min, float max)
{
    //if (!currentYZloadedFlag) {
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(xO) + ")");
        //if (xO == ui->verticalSliderYZ->value())
        //    currentYZloadedFlag = true;
        flagYZloaded = false;
        delete [] dataYZ;
        dataYZ = NULL;

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
        if (play && flagXZloaded && flagYZloaded && flagXZloaded && (!ui->actionVolumeRendering->isChecked() || flagvRLoaded))
            timer->start(interval);
    //}
}

void MainWindow::loaded3D(std::string _datasetName)
{
    flagvRLoaded = true;
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && flagvRLoaded && _datasetName == datasetName)
        timer->start(interval);
}

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
        cv::applyColorMap(image, image, currentColormap);
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
        cv::applyColorMap(image, image, currentColormap);
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
        cv::applyColorMap(image, image, currentColormap);
        cv::transpose(image, image);
        cv::flip(image, image, 0);
        QPoint p = QPoint(posY, posZ);
        if (!ui->toolButtonPositionYZ->isChecked())
            p = QPoint(0, 0);
        ((CVImageWidget *) ui->imageWidgetYZ)->showImage(image, p, ui->toolButtonFillYZ->isChecked(), fileName + "_-_" + replaceString(datasetName, "/", "-") + "_-_YZ_" + std::to_string(ui->verticalSliderYZ->value()));
    }
}

void MainWindow::repaintSlices()
{
    setImageXYFromData();
    setImageXZFromData();
    setImageYZFromData();
    // TODO
}

void MainWindow::setMinVG(float value)
{
    minVG = value;
    if (gWindow != NULL)
        gWindow->changeMinValue(value);
    repaintSlices();
}

void MainWindow::setMaxVG(float value)
{
    maxVG = value;
    if (gWindow != NULL)
        gWindow->changeMaxValue(value);
    repaintSlices();
}

void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    //ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
    currentXYloadedFlag = false;
    loadXYSlice(value);
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    //ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
    currentXZloadedFlag = false;
    loadXZSlice(value);

}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    currentYZloadedFlag = false;
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

void MainWindow::on_checkBoxUseGlobal_clicked(bool /*checked*/)
{
    //flagUseGlobalValues = checked;
    repaintSlices();
}

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
        setMinVG(value);
    }
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (flagDatasetInitialized) {
        ui->horizontalSliderGlobalMax->setTracking(false);
        ui->horizontalSliderGlobalMax->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
        ui->horizontalSliderGlobalMax->setTracking(true);
        setMaxVG(value);
    }
}

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

void MainWindow::on_comboBoxColormap_currentIndexChanged(int index)
{
    currentColormap = index;
    if (gWindow != NULL)
        gWindow->changeColormap(currentColormap);
    repaintSlices();
}

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
            currentXYloadedFlag = false;
            currentXZloadedFlag = false;
            currentYZloadedFlag = false;

            flagvRLoaded = false;

            if (gWindow != NULL && ui->actionVolumeRendering->isChecked())
                gWindow->load3DTexture(selectedDataset, minVG, maxVG, currentColormap);

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
    currentStep += increment;
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

void MainWindow::on_imageWidgetXY_imageResized(int, int)
{
    //ui->verticalSliderXY->setMaximumHeight(height);
}

void MainWindow::on_imageWidgetXZ_imageResized(int, int)
{
    //ui->verticalSliderXZ->setMaximumHeight(height);
}

void MainWindow::on_imageWidgetYZ_imageResized(int, int)
{
    //ui->verticalSliderYZ->setMaximumHeight(height);
}

void MainWindow::on_toolButtonPlay_clicked(bool checked)
{
    if (checked) {
        timer->start(interval);
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

void MainWindow::on_spinBoxTMIncrement_valueChanged(int value)
{
    increment = value;
}

void MainWindow::on_spinBoxTMInterval_valueChanged(int value)
{
    interval = value;
    timer->setInterval(value);
}

void MainWindow::on_imageWidgetXY_clickedPointInImage(int x, int y)
{
    if (flagXYloaded) {
        float value = dataXY[x + sizeX * (sizeY - y)];
        //qDebug() << x << " " << y << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

void MainWindow::on_imageWidgetXZ_clickedPointInImage(int x, int z)
{
    if (flagXZloaded) {
        float value = dataXZ[x + sizeX * z];
        //qDebug() << x << " " << z << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

void MainWindow::on_imageWidgetYZ_clickedPointInImage(int y, int z)
{
    if (flagYZloaded) {
        float value = dataYZ[(sizeZ - z) + sizeZ * y];
        //qDebug() << y << " " << z << " " << value;
        ui->statusBar->showMessage("Value: " + QWidget::locale().toString(value, 'f', 4), 3000);
        //QToolTip::showText(QCursor::pos(), QWidget::locale().toString(value, 'f', 4));
    }
}

void MainWindow::on_actionVolumeRendering_toggled(bool value)
{
    if (gWindow != NULL)
        gWindow->setViewVR(value);
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
