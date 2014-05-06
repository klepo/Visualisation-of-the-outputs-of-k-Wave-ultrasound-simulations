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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    file = NULL;

    object = NULL;
    subobject = NULL;

    openedH5File = NULL;

    windowTitle = "k-Wave HDF5 visualizer";

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));

    // OpenGL window initialize
    gWindow = NULL;
    gWindow = new GWindow();

    QWidget *widget3D = createWindowContainer(gWindow);
    widget3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = (QVBoxLayout *) ui->dockWidgetContents3D->layout();
    l->insertWidget(0, widget3D);

    connect(gWindow, SIGNAL(setStatusMessage(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    connect(gWindow, SIGNAL(loaded(std::string)), this, SLOT(loaded3D(std::string)));

    connect(ui->actionVolumeRendering, SIGNAL(toggled(bool)), gWindow, SLOT(setViewVR(bool)));

    connect(ui->action3DXY, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXYSlice(bool)));
    connect(ui->action3DXZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewXZSlice(bool)));
    connect(ui->action3DYZ, SIGNAL(toggled(bool)), gWindow, SLOT(setViewYZSlice(bool)));

    connect(ui->horizontalSliderVRSlices, SIGNAL(valueChanged(int)), gWindow, SLOT(setSlicesCount(int)));

    connect(ui->actionViewFrame, SIGNAL(toggled(bool)), gWindow, SLOT(setViewFrame(bool)));

    connect(ui->horizontalSliderVRAlpha, SIGNAL(valueChanged(int)), gWindow, SLOT(setAlpha(int)));
    connect(ui->horizontalSliderVRRed, SIGNAL(valueChanged(int)), gWindow, SLOT(setRed(int)));
    connect(ui->horizontalSliderVRGreen, SIGNAL(valueChanged(int)), gWindow, SLOT(setGreen(int)));
    connect(ui->horizontalSliderVRBlue, SIGNAL(valueChanged(int)), gWindow, SLOT(setBlue(int)));

    connect(ui->actionAlignToXY, SIGNAL(triggered()), gWindow, SLOT(alignToXY()));
    connect(ui->actionAlignToXZ, SIGNAL(triggered()), gWindow, SLOT(alignToXZ()));
    connect(ui->actionAlignToYZ, SIGNAL(triggered()), gWindow, SLOT(alignToYZ()));
    connect(ui->actionAlignToXYFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXYFromBack()));
    connect(ui->actionAlignToXZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToXZFromBack()));
    connect(ui->actionAlignToYZFromBack, SIGNAL(triggered()), gWindow, SLOT(alignToYZFromBack()));

    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), gWindow, SLOT(setTrim(bool)));

    //ui->progressBar3D->setVisible(false);

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

    dialog = new QDialog(this, Qt::CustomizeWindowHint);
    //dialog->setModal(true);
    Ui::Dialog *dialogUi = new Ui::Dialog;
    dialogUi->setupUi(dialog);
    dialogUi->label->setText("Waiting for completion of reading the file...");
}

MainWindow::~MainWindow()
{
    on_actionCloseHDF5File_triggered();
    delete ui;
    delete timer;
    if (gWindow != NULL)
        delete gWindow;
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
            clearLayout(ui->gridLayoutDatasets);

            int i = 0;
            foreach (QString key, openedH5File->getObjects().keys()) {
                //qDebug() << key;

                QGridLayout *gridLayout = ui->gridLayoutDatasets;

                QRadioButton *radioButton = new QRadioButton(key);
                QCheckBox *checkBox = new QCheckBox();
                checkBox->setAccessibleName(key);
                QComboBox *comboBox = new QComboBox();

                connect(radioButton, SIGNAL(clicked()), this, SLOT(selectDataset()));
                connect(checkBox, SIGNAL(clicked()), this, SLOT(selectDataset()));

                radioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

                gridLayout->addWidget(radioButton, i, 0);
                gridLayout->addWidget(checkBox, i, 1);
                gridLayout->addWidget(comboBox, i, 2);

                i++;

                foreach (QString item, openedH5File->getObjects().value(key)->getSubobjectNames()) {
                    //qDebug() << "->" << item;
                    comboBox->addItem(item);
                    comboBox->setCurrentText(item);
                }

                connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectDataset()));
            }

            file = openedH5File->getFile();

            ui->actionCloseHDF5File->setEnabled(true);
            ui->dockWidgetDatasets->setEnabled(true);
            ui->dockWidgetInfo->setEnabled(true);

        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            QMessageBox messageBox;
            messageBox.critical(0, "Error", "Wrong HDF5 file!");
        }

    }
}

void MainWindow::on_actionCloseHDF5File_triggered()
{
    dialog->show();
    QApplication::processEvents();

    clearRequestsAndWaitThreads();

    delete openedH5File;
    openedH5File = NULL;

    dialog->hide();

    object = NULL;
    subobject = NULL;

    clearGUI();
}

void MainWindow::clearGUI()
{
    setWindowTitle(windowTitle);

    flagVRLoaded = false;

    ui->comboBoxColormap->setCurrentIndex(cv::COLORMAP_JET);

    play = false;

    ui->dockWidgetSelectedDataset->setEnabled(false);
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
    ui->actionInfo->setChecked(false);
    ui->dockWidgetInfo->setVisible(false);

    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    // TODO clear 3D scene
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

    ui->actionCloseHDF5File->setEnabled(false);

}

void MainWindow::clearLayout(QLayout *layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != NULL) {
        delete item->widget();
        delete item;
    }
}

void MainWindow::clearRequestsAndWaitThreads()
{
    if (gWindow != NULL)
    {
        gWindow->getThread()->clearRequests();
        gWindow->getThread()->wait();
        //gWindow->getThread()->clearDoneRequests();
    }
}

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

    if (gWindow != NULL)
        gWindow->getThread()->clearRequests();

    ui->imageWidgetXY->clearImage();
    ui->imageWidgetXZ->clearImage();
    ui->imageWidgetYZ->clearImage();

    if (gWindow != NULL) {
        gWindow->clearData();
    }

    flagDatasetInitialized = false;
    flagVRLoaded = false;

    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    play = false;

    // Clear dataset info
    clearLayout(ui->formLayoutSelectedDatasetInfo);

    // Disconnect all
    foreach (OpenedH5File::H5ObjectToVisualize *object, openedH5File->getObjects()) {
        foreach (OpenedH5File::H5SubobjectToVisualize *subobject, object->getSubobjects()) {
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
            disconnect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), 0, 0);
        }
    }

    object = NULL;
    subobject = NULL;

    if (selectedObjectName == "")
        return;

    selectedSubobjectName = openedH5File->getObject(selectedObjectName)->getSelectedSubobject()->getName();

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

        connect(subobject, SIGNAL(imageXYChanged(cv::Mat, int)), this, SLOT(repaintXYImage(cv::Mat, int)));
        connect(subobject, SIGNAL(imageXZChanged(cv::Mat, int)), this, SLOT(repaintXZImage(cv::Mat, int)));
        connect(subobject, SIGNAL(imageYZChanged(cv::Mat, int)), this, SLOT(repaintYZImage(cv::Mat, int)));

        ui->dockWidgetSelectedDataset->setEnabled(true);
        ui->dockWidgetXY->setEnabled(true);
        ui->dockWidgetXZ->setEnabled(true);
        ui->dockWidgetYZ->setEnabled(true);

        if (gWindow != NULL) {
            gWindow->changeMinValue(subobject->getMinVG());
            gWindow->changeMaxValue(subobject->getMaxVG());
            gWindow->changeColormap(subobject->getColormap());
            gWindow->setMainSize(subobject->getFrameSize()[0], subobject->getFrameSize()[1], subobject->getFrameSize()[2]);
            gWindow->setSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
            gWindow->setPosition(subobject->getPos()[0], subobject->getPos()[1], subobject->getPos()[2]);

            if (ui->actionFillSpace->isChecked()) {
                gWindow->setMainSize(subobject->getSize()[0], subobject->getSize()[1], subobject->getSize()[2]);
                gWindow->setPosition(0, 0, 0);
            }
        }

        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            //if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
            gWindow->load3DTexture(subobject->getDataset());
        }

        subobject->setGUIInitialized(true);
    }
}

void MainWindow::repaintXYImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[0] == 1)
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) 0);
            else
                gWindow->setXYSlice(subobject->getDataXY(), subobject->getSize()[2], subobject->getSize()[1], (float) ui->verticalSliderXY->value() / (subobject->getSize()[0] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[1]);
        p = QPoint(0, 0);
        ui->imageWidgetXY->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(index) + ")");

        subobject->setGUIXYInitialized(false);
        ui->doubleSpinBoxXYMin->setRange(subobject->getOriginalMinVXY(), subobject->getOriginalMaxVXY());
        ui->doubleSpinBoxXYMax->setRange(subobject->getOriginalMinVXY(), subobject->getOriginalMaxVXY());
        ui->doubleSpinBoxXYMin->setValue(subobject->getMinVXY());
        ui->doubleSpinBoxXYMax->setValue(subobject->getMaxVXY());
        ui->doubleSpinBoxXYMin->setSingleStep((subobject->getOriginalMaxVXY() - subobject->getOriginalMinVXY()) / 1000);
        ui->doubleSpinBoxXYMax->setSingleStep((subobject->getOriginalMaxVXY() - subobject->getOriginalMinVXY()) / 1000);
        subobject->setGUIXYInitialized(true);

        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        if (subobject->isCurrentXYLoaded()) ui->labelXYLoading->setVisible(false);
    }
}

void MainWindow::repaintXZImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[1] == 1)
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) 0);
            else
                gWindow->setXZSlice(subobject->getDataXZ(), subobject->getSize()[2], subobject->getSize()[0], (float) ui->verticalSliderXZ->value() / (subobject->getSize()[1] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[2], subobject->getPos()[0]);
        p = QPoint(0, 0);
        ui->imageWidgetXZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(index) + ")");

        subobject->setGUIXZInitialized(false);
        ui->doubleSpinBoxXZMin->setRange(subobject->getOriginalMinVXZ(), subobject->getOriginalMaxVXZ());
        ui->doubleSpinBoxXZMax->setRange(subobject->getOriginalMinVXZ(), subobject->getOriginalMaxVXZ());
        ui->doubleSpinBoxXZMin->setValue(subobject->getMinVXZ());
        ui->doubleSpinBoxXZMax->setValue(subobject->getMaxVXZ());
        ui->doubleSpinBoxXZMin->setSingleStep((subobject->getOriginalMaxVXZ() - subobject->getOriginalMinVXZ()) / 1000);
        ui->doubleSpinBoxXZMax->setSingleStep((subobject->getOriginalMaxVXZ() - subobject->getOriginalMinVXZ()) / 1000);
        subobject->setGUIXZInitialized(true);

        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        if (subobject->isCurrentXZLoaded()) ui->labelXZLoading->setVisible(false);
    }
}

void MainWindow::repaintYZImage(cv::Mat image, int index)
{
    if (subobject != NULL) {
        if (gWindow != NULL) {
            if (subobject->getSize()[2] == 1)
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) 0);
            else
                gWindow->setYZSlice(subobject->getDataYZ(), subobject->getSize()[1], subobject->getSize()[0], (float) ui->verticalSliderYZ->value() / (subobject->getSize()[2] - 1));
        }
        QPoint p = QPoint(subobject->getPos()[1], subobject->getPos()[0]);
        p = QPoint(0, 0);
        ui->imageWidgetYZ->showImage(image, p, openedH5File->getRawFilename() + "_-_" + subobject->getName() + "_-_XY_" + QString::number(ui->verticalSliderXY->value()));
        ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(index) + ")");

        subobject->setGUIYZInitialized(false);
        ui->doubleSpinBoxYZMin->setRange(subobject->getOriginalMinVYZ(), subobject->getOriginalMaxVYZ());
        ui->doubleSpinBoxYZMax->setRange(subobject->getOriginalMinVYZ(), subobject->getOriginalMaxVYZ());
        ui->doubleSpinBoxYZMin->setValue(subobject->getMinVYZ());
        ui->doubleSpinBoxYZMax->setValue(subobject->getMaxVYZ());
        ui->doubleSpinBoxYZMin->setSingleStep((subobject->getOriginalMaxVYZ() - subobject->getOriginalMinVYZ()) / 1000);
        ui->doubleSpinBoxYZMax->setSingleStep((subobject->getOriginalMaxVYZ() - subobject->getOriginalMinVYZ()) / 1000);
        subobject->setGUIYZInitialized(true);

        if (play && subobject->areCurrentSlicesLoaded() && (!ui->actionVolumeRendering->isChecked() || flagVRLoaded))
            timer->start(ui->spinBoxTMInterval->value());

        if (subobject->isCurrentYZLoaded()) ui->labelYZLoading->setVisible(false);
    }
}

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

        ui->labelXYLoading->setVisible(true);
        ui->labelXZLoading->setVisible(true);
        ui->labelYZLoading->setVisible(true);
    }
}

// 3D data loaded event

void MainWindow::loaded3D(std::string datasetName)
{
    flagVRLoaded = true;
    ui->label3DLoading->clear();

    //ui->progressBar3D->setVisible(false);
    //ui->progressBar3D->setValue(0);
    if (subobject != NULL && subobject->getGroup() != NULL)
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
    if (subobject->getGroup() != NULL) {
        subobject->setCurrentStep(step, gWindow->getThread());
        if (gWindow != NULL && ui->actionVolumeRendering->isChecked()) {
            //ui->progressBar3D->setValue(0);
            //ui->progressBar3D->setVisible(true);
            //if (!gWindow->isTexture3DInitialized())
            ui->label3DLoading->setMovie(movie);
            gWindow->load3DTexture(subobject->getDataset());
        }
    }
}

void MainWindow::updateStep()
{
    if (subobject->getGroup() != NULL) {
        uint64_t step = subobject->getCurrentStep();
        step += ui->spinBoxTMIncrement->value();
        if (step >= subobject->getSteps()) {
            timer->stop();
            play = false;
            ui->toolButtonPlay->setChecked(false);
            ui->horizontalSliderSelectedDatasetStep->setValue(subobject->getSteps() - 1);
        } else {
            timer->stop();
            ui->horizontalSliderSelectedDatasetStep->setValue(step);
        }
    }
}

void MainWindow::on_toolButtonPlay_clicked(bool checked)
{
    if (subobject->getGroup() != NULL) {
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
    if (subobject->getGroup() != NULL) {
        timer->stop();
        play = false;
        ui->toolButtonPlay->setChecked(false);
        ui->horizontalSliderSelectedDatasetStep->setValue(0);
    }
}

void MainWindow::on_toolButtonEnd_clicked()
{
    if (subobject->getGroup() != NULL) {
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
    if (gWindow != NULL && subobject != NULL && value/*&& !gWindow->isTexture3DInitialized()*/) {
        //ui->progressBar3D->setValue(0);
        //ui->progressBar3D->setVisible(true);
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
        if (subobject != NULL) name = subobject->getName();
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
    if (subobject != NULL) {
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
