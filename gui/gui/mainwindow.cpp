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
#include <QSignalMapper>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    file = NULL;

    object = NULL;
    subobject = NULL;

    openedH5File = NULL;

    dataXY = NULL;
    dataXZ = NULL;
    dataYZ = NULL;

    windowTitle = "k-Wave HDF5 visualizer";

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));

    qRegisterMetaType<hsize_t>("hsize_t");
    threadXY = new HDF5ReadingThread();
    connect(threadXY, SIGNAL(requestDone(Request *)), this, SLOT(setXYLoaded(Request *)));
    //threadXY->start();
    threadXZ = new HDF5ReadingThread();
    connect(threadXZ, SIGNAL(requestDone(Request *)), this, SLOT(setXZLoaded(Request *)));
    //threadXZ->start();
    threadYZ = new HDF5ReadingThread();
    connect(threadYZ, SIGNAL(requestDone(Request *)), this, SLOT(setYZLoaded(Request *)));
    //threadYZ->start();

    qRegisterMetaType<cv::Mat>("cv::Mat");

    // Slices indices change connect
    //connect(ui->verticalSliderXY, SIGNAL(valueChanged(int)), this, SLOT(loadXYSlice(int)));
    //connect(ui->verticalSliderXZ, SIGNAL(valueChanged(int)), this, SLOT(loadXZSlice(int)));
    //connect(ui->verticalSliderYZ, SIGNAL(valueChanged(int)), this, SLOT(loadYZSlice(int)));

    // OpenGL window initialize
    gWindow = NULL;
    gWindow = new GWindow();

    QWidget *widget3D = createWindowContainer(gWindow);
    widget3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = (QVBoxLayout *) ui->dockWidgetContents3D->layout();
    l->insertWidget(0, widget3D);

    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    connect(gWindow, SIGNAL(loaded(std::string)), this, SLOT(loaded3D(std::string)));

    //connect(gWindow, SIGNAL(partLoaded(int)), ui->progressBar3D, SLOT(setValue(int)));

    connect(ui->actionVolumeRendering, SIGNAL(toggled(bool)), gWindow, SLOT(setViewVR(bool)));

    connect(ui->action3DXY, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXYSlice(bool)));
    connect(ui->action3DXZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXZSlice(bool)));
    connect(ui->action3DYZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewYZSlice(bool)));

    connect(ui->horizontalSliderVRSlices, SIGNAL(valueChanged(int)), gWindow, SLOT(setSlicesCount(int)));
    connect(ui->checkBoxVRFrame, SIGNAL(clicked(bool)), gWindow, SLOT(setViewFrame(bool)));

    connect(ui->horizontalSliderVRAlpha, SIGNAL(valueChanged(int)), gWindow, SLOT(setAlpha(int)));
    connect(ui->horizontalSliderVRRed, SIGNAL(valueChanged(int)), gWindow, SLOT(setRed(int)));
    connect(ui->horizontalSliderVRGreen, SIGNAL(valueChanged(int)), gWindow, SLOT(setGreen(int)));
    connect(ui->horizontalSliderVRBlue, SIGNAL(valueChanged(int)), gWindow, SLOT(setBlue(int)));

    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));

    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), gWindow, SLOT(setTrim(bool)));

    //ui->progressBar3D->setVisible(false);

    ui->toolButtonPositionXY->setVisible(false);
    ui->toolButtonPositionXZ->setVisible(false);
    ui->toolButtonPositionYZ->setVisible(false);

    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();
    ui->labelXYLoading->setVisible(false);
    ui->labelXZLoading->setVisible(false);
    ui->labelYZLoading->setVisible(false);
    ui->labelXYLoading->setMovie(movie);
    ui->labelXZLoading->setMovie(movie);
    ui->labelYZLoading->setMovie(movie);
    clearGUI();
}

MainWindow::~MainWindow()
{
    on_actionCloseHDF5File_triggered();
    delete ui;
    delete timer;
    if (gWindow != NULL)
        delete gWindow;
    delete threadXY;
    delete threadXZ;
    delete threadYZ;
}

void MainWindow::on_actionLoadOutputHDF5File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "HDF5 Files (*.h5)");

    if (fileName != "") {

        on_actionCloseHDF5File_triggered();

        try {
            openedH5File = new OpenedH5File(fileName);

            // Set info to GUI
            ui->textBrowserInfo->clear();
            foreach (QString key, openedH5File->getInfo().keys())
                ui->textBrowserInfo->append("<strong>" + key + "</strong><br>" + openedH5File->getInfo().value(key) + "<br>");
            QScrollBar *v = ui->textBrowserInfo->verticalScrollBar();
            v->setValue(v->minimum());

            setWindowTitle(windowTitle + " - " + openedH5File->getFilename());

            // Clear datasets GUI
            QLayoutItem* item;
            while ((item = ui->verticalLayoutDatasets->takeAt(0)) != NULL)
            {
                delete item->widget();
                delete item;
            }

            // Fill datasets list
            //QSignalMapper* signalMapper = new QSignalMapper(this);

            foreach (QString key, openedH5File->getObjects().keys()) {
                qDebug() << key;
                QWidget *widget = new QWidget();
                widget->setLayout(new QHBoxLayout);
                widget->layout()->setMargin(0);
                QCheckBox *checkBox = new QCheckBox();

                //connect(checkBox, SIGNAL(clicked()), signalMapper, SLOT(map()));
                //signalMapper->setMapping(checkBox, key);

                connect(checkBox, SIGNAL(clicked()), this, SLOT(selectDataset()));

                QComboBox *comboBox = new QComboBox();
                widget->layout()->addWidget(checkBox);
                widget->layout()->addWidget(comboBox);
                ui->verticalLayoutDatasets->addWidget(widget);
                foreach (QString item, openedH5File->getObjects().value(key)->getSubobjectNames()) {
                    qDebug() << "->" << item;
                    comboBox->addItem(item);
                    comboBox->setCurrentText(item);
                }
                //connect(comboBox, SIGNAL(currentIndexChanged(QString)), openedH5File, SLOT(setSelectedSubobject(QString)));
                connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectDataset()));
            }

            //connect(signalMapper, SIGNAL(mapped(QString)), openedH5File, SLOT(toogleObjectSelected(QString)));

            file = openedH5File->getFile();

            ui->actionCloseHDF5File->setEnabled(true);
            ui->dockWidgetDatasets->setEnabled(true);
            ui->dockWidgetInfo->setEnabled(true);

            //threadXY->start();
            //threadXZ->start();
            //threadYZ->start();

        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            // TODO error dialog
        }

    }
}

void MainWindow::on_actionCloseHDF5File_triggered()
{
    clearRequestsAndWaitThreads();

    delete openedH5File;
    openedH5File = NULL;

    delete [] dataXY;
    dataXY = NULL;

    delete [] dataXZ;
    dataXZ = NULL;

    delete [] dataYZ;
    dataYZ = NULL;

    object = NULL;
    subobject = NULL;

    clearGUI();
}

void MainWindow::clearGUI()
{
    setWindowTitle(windowTitle);

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

    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    // TODO clear 3D scene
    if (gWindow != NULL) {
        gWindow->clearData();
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

void MainWindow::clearRequestsAndWaitThreads()
{
    threadXY->clearRequests();
    QMetaObject::invokeMethod(threadXY, "stop");
    threadXY->wait();
    //threadXY->clearDoneRequests();

    threadXZ->clearRequests();
    QMetaObject::invokeMethod(threadXZ, "stop");
    threadXZ->wait();
    //threadXZ->clearDoneRequests();

    threadYZ->clearRequests();
    QMetaObject::invokeMethod(threadYZ, "stop");
    threadYZ->wait();
    //threadYZ->clearDoneRequests();

    if (gWindow != NULL)
    {
        gWindow->getThread()->clearRequests();
        QMetaObject::invokeMethod(gWindow->getThread(), "stop");
        gWindow->getThread()->wait();
        //gWindow->getThread()->clearDoneRequests();
    }
}

void MainWindow::selectDataset()
{
    QString selectedName = "";

    // Find selected dataset or group
    QList<QCheckBox *> list = ui->dockWidgetContentsDatasets->findChildren<QCheckBox *>();
    foreach (QCheckBox *checkBox, list) {
        if (checkBox->isChecked()) {
            QComboBox *comboBox = checkBox->parent()->findChild<QComboBox *>();
            selectedName = comboBox->currentText();
            break;
        }
    }

    if (subobject != NULL && selectedName == subobject->getName())
        return;


    //clearRequestsAndWaitThreads();
    threadXY->clearRequests();
    threadXZ->clearRequests();
    threadYZ->clearRequests();
    if (gWindow != NULL)
        gWindow->getThread()->clearRequests();


    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    if (gWindow != NULL) {
        gWindow->clearData();
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
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    posZ = 0;
    posY = 0;
    posX = 0;

    steps = 0;
    play = false;
    currentStep = 0;

    // Clear dataset info
    QLayoutItem* item;
    while ((item = ui->formLayoutSelectedDatasetInfo->takeAt(0)) != NULL) {
        delete item->widget();
        delete item;
    }

    // Disconnect all
    foreach (OpenedH5File::H5ObjectToVisualize *object, openedH5File->getObjects()) {
        foreach (OpenedH5File::H5SubobjectToVisualize *subobject, object->getSubobjects()) {
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat)), 0, 0);
            qDebug() << subobject->getName();
        }
    }


    object = NULL;
    subobject = NULL;

    if (selectedName == "")
        return;

    object = openedH5File->getObjectBySubobjectName(selectedName);
    openedH5File->setSelectedSubobject(selectedName);
    if (object != NULL)
        subobject = object->getSelectedSubobject();

    if (subobject != NULL) {

        qDebug() << "--> Selected dataset" << subobject->getName();

        minVG = subobject->getMinVG();
        maxVG = subobject->getMaxVG();

        // Set dataset info
        QList<QPair<QString, QString>> info = subobject->getInfo();
        for(int i = 0; i < info.count(); ++i)
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel(info[i].first + ":"), new QLabel(info[i].second));

        hsize_t *size = subobject->getSize();
        sizeZ = size[0];
        sizeY = size[1];
        sizeX = size[2];

        steps = subobject->getSteps();

        hsize_t *pos = subobject->getPos();
        posZ = pos[0];
        posY = pos[1];
        posX = pos[2];

        // Init GUI controls
        setControls();

        //initSlices();
        subobject->setXIndex(subobject->getXIndex());
        subobject->setYIndex(subobject->getYIndex());
        subobject->setZIndex(subobject->getZIndex());


        connect(subobject, SIGNAL(imageXYChanged(cv::Mat)), this, SLOT(repaintXYImage(cv::Mat)));
        connect(subobject, SIGNAL(imageXZChanged(cv::Mat)), this, SLOT(repaintXZImage(cv::Mat)));
        connect(subobject, SIGNAL(imageYZChanged(cv::Mat)), this, SLOT(repaintYZImage(cv::Mat)));


        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);

        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            gWindow->load3DTexture(subobject->getDataset());
        }

        if (gWindow != NULL) {
            gWindow->changeMinValue(subobject->getMinVG());
            gWindow->changeMaxValue(subobject->getMaxVG());
            gWindow->changeColormap(subobject->getColormap());
            gWindow->setMainSize(subobject->getFrameSize()[0], subobject->getFrameSize()[1], subobject->getFrameSize()[2]);
            gWindow->setSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
            gWindow->setPosition(subobject->getPos()[0], subobject->getPos()[1], subobject->getPos()[2]);
        }

        flagDatasetInitialized = true;
    }
}

void MainWindow::repaintXYImage(cv::Mat image)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[0] == 1)
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) 0);
            else
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) ui->verticalSliderXY->value() / (subobject->getSize()[0] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[1]);
        if (!ui->toolButtonPositionXY->isChecked())
            p = QPoint(0, 0);
        ui->imageWidgetXY->showImage(image, p, ui->toolButtonFillXY->isChecked(), openedH5File->getRawFilename().toStdString() + "_-_" + subobject->getName().toStdString() + "_-_XY_" + std::to_string(ui->verticalSliderXY->value()));
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(subobject->getZIndex()) + ")");
    }
    //if (r->zO == ui->verticalSliderXY->value()) ui->labelXYLoading->setVisible(false);

}

void MainWindow::repaintXZImage(cv::Mat image)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[1] == 1)
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) 0);
            else
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) ui->verticalSliderXZ->value() / (subobject->getSize()[1] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[0]);
        if (!ui->toolButtonPositionXZ->isChecked())
            p = QPoint(0, 0);
        ui->imageWidgetXZ->showImage(image, p, ui->toolButtonFillXY->isChecked(), openedH5File->getRawFilename().toStdString() + "_-_" + subobject->getName().toStdString() + "_-_XY_" + std::to_string(ui->verticalSliderXY->value()));
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(subobject->getYIndex()) + ")");
    }
}

void MainWindow::repaintYZImage(cv::Mat image)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[2] == 1)
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) 0);
            else
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) ui->verticalSliderYZ->value() / (subobject->getSize()[2] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[1], subobject->getPos()[0]);
        if (!ui->toolButtonPositionYZ->isChecked())
            p = QPoint(0, 0);
        ui->imageWidgetYZ->showImage(image, p, ui->toolButtonFillXY->isChecked(), openedH5File->getRawFilename().toStdString() + "_-_" + subobject->getName().toStdString() + "_-_XY_" + std::to_string(ui->verticalSliderXY->value()));
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(subobject->getXIndex()) + ")");
    }
}

void MainWindow::setControls()
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

        ui->comboBoxColormap->setCurrentIndex(subobject->getColormap());
    }
}

void MainWindow::initSlices()
{
    if (subobject != NULL && subobject->getDataset() != NULL) {

        if (gWindow != NULL) {
            gWindow->changeMinValue(subobject->getMinVG());
            gWindow->changeMaxValue(subobject->getMaxVG());
            gWindow->changeColormap(subobject->getColormap());
            gWindow->setMainSize(subobject->getFrameSize()[0], subobject->getFrameSize()[1], subobject->getFrameSize()[2]);
            gWindow->setSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
            gWindow->setPosition(subobject->getPos()[0], subobject->getPos()[1], subobject->getPos()[2]);
        }

        flagDatasetInitialized = true;
        loadXYSlice(ui->verticalSliderXY->value());
        loadXZSlice(ui->verticalSliderXZ->value());
        loadYZSlice(ui->verticalSliderYZ->value());

        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            gWindow->load3DTexture(subobject->getDataset());
        }
    }
}

// Loading slices

void MainWindow::loadXYSlice(int index)
{
    if (flagDatasetInitialized && subobject != NULL && subobject->getDataset() != NULL) {
        ui->labelXYLoading->setVisible(true);
        threadXY->createRequest(subobject->getDataset(), index, 0, 0, 1, sizeY, sizeX);
        //if (!threadXY->isRunning())
        //threadXY->start();
    }
}

void MainWindow::setXYLoaded(Request *r)
{
    ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(r->zO) + ")");
    flagXYloaded = false;
    delete [] dataXY;
    dataXY = NULL;

    if (r->zO == ui->verticalSliderXY->value()) ui->labelXYLoading->setVisible(false);

    // TODO mutex
    hsize_t size = r->zC * r->yC * r->xC;
    dataXY = new float[size];
    memcpy(dataXY, r->data, size * sizeof(float));
    threadXY->deleteDoneRequest(r);

    minVXY = r->min;
    maxVXY = r->max;

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

void MainWindow::loadXZSlice(int index)
{
    if (flagDatasetInitialized && subobject != NULL && subobject->getDataset() != NULL) {
        ui->labelXZLoading->setVisible(true);
        threadXZ->createRequest(subobject->getDataset(), 0, index, 0, sizeZ, 1, sizeX);
        //if (!threadXZ->isRunning())
        //threadXZ->start();
    }
}

void MainWindow::setXZLoaded(Request *r)
{
    ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(r->yO) + ")");
    flagXZloaded = false;
    delete [] dataXZ;
    dataXZ = NULL;

    if (r->yO == ui->verticalSliderXZ->value()) ui->labelXZLoading->setVisible(false);

    hsize_t size = r->zC * r->yC * r->xC;
    dataXZ = new float[size];
    memcpy(dataXZ, r->data, size * sizeof(float));
    threadXZ->deleteDoneRequest(r);

    minVXZ = r->min;
    maxVXZ = r->max;

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

void MainWindow::loadYZSlice(int index)
{
    if (flagDatasetInitialized && subobject != NULL && subobject->getDataset() != NULL) {
        ui->labelYZLoading->setVisible(true);
        threadYZ->createRequest(subobject->getDataset(), 0, 0, index, sizeZ, sizeY, 1);
        //if (!threadYZ->isRunning())
        //threadYZ->start();
    }
}

void MainWindow::setYZLoaded(Request *r)
{
    ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(r->xO) + ")");
    flagYZloaded = false;
    delete [] dataYZ;
    dataYZ = NULL;

    if (r->xO == ui->verticalSliderYZ->value()) ui->labelYZLoading->setVisible(false);

    hsize_t size = r->zC * r->yC * r->xC;
    dataYZ = new float[size];
    memcpy(dataYZ, r->data, size * sizeof(float));
    threadYZ->deleteDoneRequest(r);

    minVYZ = r->min;
    maxVYZ = r->max;

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

void MainWindow::loaded3D(std::string datasetName)
{
    flagVRLoaded = true;
    //ui->progressBar3D->setVisible(false);
    //ui->progressBar3D->setValue(0);
    // If animation is running...
    if (play && flagXZloaded && flagYZloaded && flagXZloaded && flagVRLoaded && datasetName == subobject->getDataset()->getName())
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
        ((CVImageWidget *) ui->imageWidgetXY)->showImage(image, p, ui->toolButtonFillXY->isChecked(), openedH5File->getRawFilename().toStdString() + "__" + std::to_string(ui->verticalSliderXY->value()));
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
        ((CVImageWidget *) ui->imageWidgetXZ)->showImage(image, p, ui->toolButtonFillXZ->isChecked(), openedH5File->getRawFilename().toStdString() + "__" + std::to_string(ui->verticalSliderXZ->value()));
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
        ((CVImageWidget *) ui->imageWidgetYZ)->showImage(image, p, ui->toolButtonFillYZ->isChecked(), openedH5File->getRawFilename().toStdString() + "__" + std::to_string(ui->verticalSliderYZ->value()));
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
    ui->doubleSpinBoxMinGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    ui->doubleSpinBoxMaxGlobal->setValue((double) value / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MainWindow::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    if (subobject != NULL && flagDatasetInitialized) {
        subobject->setMinVG(value);
    }
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition((int) qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);

    if (gWindow != NULL)
        gWindow->changeMinValue(value);
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (subobject != NULL && flagDatasetInitialized) {
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
    //repaintSlices();
    if (subobject != NULL && flagDatasetInitialized) {
        subobject->setColormap(index);
    }
}

// time series buttons

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    currentStep = step;
    if (subobject->getGroup() != NULL) {
        try {
            clearRequestsAndWaitThreads();

            /*file->closeDataset(object->getSelectedDataset()->getName());
            selectedDataset = file->openDataset(selectedName + "/" + std::to_string(step));
            datasetName = selectedName + "/" + std::to_string(step);

            flagVRLoaded = false;

            if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
                //ui->progressBar3D->setValue(0);
                //ui->progressBar3D->setVisible(true);
                gWindow->load3DTexture(object->getSelectedDataset());
            }
            loadXYSlice(ui->verticalSliderXY->value());
            loadXZSlice(ui->verticalSliderXZ->value());
            loadYZSlice(ui->verticalSliderYZ->value());*/
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

// Select point on slices, show value

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

// Load 3D texture on enable

void MainWindow::on_actionVolumeRendering_toggled(bool value)
{
    if (gWindow != NULL && subobject != NULL && subobject->getDataset() != NULL && value && !gWindow->isTexture3DInitialized()) {
        //ui->progressBar3D->setValue(0);
        //ui->progressBar3D->setVisible(true);
        gWindow->load3DTexture(subobject->getDataset());
    }
}

// Save 3D scene image

void MainWindow::on_actionExportImageFrom3DScene_triggered()
{
    if (gWindow != NULL) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", subobject->getName() + "_3Dscene.png", "Image (*.png)");
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


void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    if (subobject != NULL && flagDatasetInitialized) {
        subobject->setZIndex(value);
    }
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    if (subobject != NULL && flagDatasetInitialized) {
        subobject->setYIndex(value);
    }
}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    if (subobject != NULL && flagDatasetInitialized) {
        subobject->setXIndex(value);
    }
}

