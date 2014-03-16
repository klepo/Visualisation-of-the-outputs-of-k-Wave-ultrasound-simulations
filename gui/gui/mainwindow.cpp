#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cvimagewidget.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    file = NULL;
    windowTitle = "k-Wave HDF5 visualizer";
    nT = 0;
    nX = 0;
    nY = 0;
    nZ = 0;
    setWindowTitle(windowTitle);
    selectedDataset = NULL;
    selectedGroup = NULL;
    selectedName = "";

    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;
    flagUseGlobalValues = false;
    ui->checkBoxUseGlobal->setChecked(false);

    dataXY = NULL;
    dataXZ = NULL;
    dataYZ = NULL;

    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidget3D->setEnabled(false);
    ui->dockWidgetCT->setEnabled(false);
    ui->dockWidgetDatasets->setEnabled(false);
    ui->dockWidgetInfo->setEnabled(false);
    ui->dockWidgetXY->setEnabled(false);
    ui->dockWidgetXZ->setEnabled(false);
    ui->dockWidgetYZ->setEnabled(false);

    currentColormap = cv::COLORMAP_JET;

    steps = 0;
    currentStep = 0;
}

MainWindow::~MainWindow()
{
    on_actionCloseHDF5File_triggered();
    delete ui;
    //if (file != NULL)
    //    delete file;
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("HDF5 Files (*.h5)"));

    on_actionCloseHDF5File_triggered();

    if (fileName != "") {
        try {
            file = new HDF5File(fileName.toStdString());
            setWindowTitle(windowTitle + " - " + fileName);
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

void MainWindow::selectDataset()
{
    flagDatasetInitialized = false;
    flagGroupInitialized = false;
    flagXYloaded = false;
    flagXZloaded = false;
    flagYZloaded = false;

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
        ui->doubleSpinBoxMinGlobal->setSingleStep((maxVG - minVG) / 100);
        ui->doubleSpinBoxMaxGlobal->setSingleStep((maxVG - minVG) / 100);

        ui->horizontalSliderGlobalMin->setValue(ui->horizontalSliderGlobalMin->minimum());
        ui->horizontalSliderGlobalMax->setValue(ui->horizontalSliderGlobalMax->maximum());

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

            ui->spinBoxSelectedDatasetStep->setMaximum(steps-1);
            ui->spinBoxSelectedDatasetStep->setValue(0);
            ui->horizontalSliderSelectedDatasetStep->setMaximum(steps-1);
            ui->horizontalSliderSelectedDatasetStep->setValue(currentStep);
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Steps:"), new QLabel(QString::number(steps)));

            sizeZ = selectedGroup->readAttributeI("sizeZ");
            sizeY = selectedGroup->readAttributeI("sizeY");
            sizeX = selectedGroup->readAttributeI("sizeX");
            ui->formLayoutSelectedDatasetInfo->addRow(new QLabel("Size:"), new QLabel(QString::number(sizeZ) + " x " + QString::number(sizeY) + " x " + QString::number(sizeX)));

            selectedDataset = file->openDataset(selectedName + "/" + std::to_string(0));

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

            initSlices();

            ui->dockWidgetSelectedDataset->setEnabled(true);
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
        loadXYSlice(0);
        loadXZSlice(0);
        loadYZSlice(0);
    } else if (selectedGroup != NULL) {

    } else {
        //clear
    }
}

void MainWindow::loadXYSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        try {
            flagXYloaded = false;
            dataXY = selectedDataset->read3DDataset(index, 0, 0, 1, sizeY, sizeX, minVXY, maxVXY);
            // TODO set min max sliders
            flagXYloaded = true;
            setImageXYFromData();
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void MainWindow::loadXZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        try {
            flagXZloaded = false;
            dataXZ = selectedDataset->read3DDataset(0, index, 0, sizeZ, 1, sizeX, minVXZ, maxVXZ);
            // TODO set min max sliders
            flagXZloaded = true;
            setImageXZFromData();
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void MainWindow::loadYZSlice(hsize_t index)
{
    if (flagDatasetInitialized || flagGroupInitialized) {
        try {
            flagYZloaded = false;
            dataYZ = selectedDataset->read3DDataset(0, 0, index, sizeZ, sizeY, 1, minVYZ, maxVYZ);
            // TODO set min max sliders
            flagYZloaded = true;
            setImageYZFromData();
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void MainWindow::setImageXYFromData()
{
    if (flagXYloaded) {
        cv::Mat image = cv::Mat(sizeY, sizeX, CV_32FC1, dataXY); // rows, cols (height, width)
        if (flagUseGlobalValues)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXY - minVXY), - minVXY * 255.0 /(maxVXY - minVXY));
        cv::applyColorMap(image, image, currentColormap);
        ((CVImageWidget *) ui->imageWidgetXY)->showImage(image);
    }
}

void MainWindow::setImageXZFromData()
{
    if (flagXZloaded) {
        cv::Mat image = cv::Mat(sizeZ, sizeX, CV_32FC1, dataXZ); // rows, cols (height, width)
        if (flagUseGlobalValues)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXZ - minVXZ), - minVXZ * 255.0 /(maxVXZ - minVXZ));
        cv::applyColorMap(image, image, currentColormap);
        ((CVImageWidget *) ui->imageWidgetXZ)->showImage(image);
    }
}

void MainWindow::setImageYZFromData()
{
    if (flagYZloaded) {
        cv::Mat image = cv::Mat(sizeZ, sizeY, CV_32FC1, dataYZ); // rows, cols (height, width)
        if (flagUseGlobalValues)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 /(maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVYZ - minVYZ), - minVYZ * 255.0 /(maxVYZ - minVYZ));
        cv::applyColorMap(image, image, currentColormap);
        ((CVImageWidget *) ui->imageWidgetYZ)->showImage(image);
    }
}

void MainWindow::on_actionCloseHDF5File_triggered()
{
    if (file != NULL) {
        delete file;
        file = NULL;
    }

    ui->dockWidgetSelectedDataset->setEnabled(false);

    ui->dockWidgetSelectedDataset->setEnabled(false);
    ui->dockWidget3D->setEnabled(false);
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
    flagUseGlobalValues = false;
    ui->checkBoxUseGlobal->setChecked(false);

    selectedDataset = NULL;
    selectedGroup = NULL;
    selectedName = "";
    nT = 0;
    nX = 0;
    nY = 0;
    nZ = 0;

    steps = 0;
    currentStep = 0;

    // Clear dataset info
    QLayoutItem* item;
    while ((item = ui->formLayoutSelectedDatasetInfo->takeAt( 0 )) != NULL)
    {
        delete item->widget();
        delete item;
    }
    //ui->groupBoxSelectedDatasetInfo->adjustSize();

    // Clear datasets
    while ((item = ui->verticalLayoutDatasets->takeAt( 0 )) != NULL)
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
    repaintSlices();
}

void MainWindow::setMaxVG(float value)
{
    maxVG = value;
    repaintSlices();
}

void MainWindow::on_verticalSliderXY_valueChanged(int value)
{
    ui->dockWidgetXY->setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
    loadXYSlice(value);
}

void MainWindow::on_verticalSliderXZ_valueChanged(int value)
{
    ui->dockWidgetXZ->setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
    loadXZSlice(value);

}

void MainWindow::on_verticalSliderYZ_valueChanged(int value)
{
    ui->dockWidgetYZ->setWindowTitle("YZ slice (X = " + QString::number(value) + ")");
    loadXYSlice(value);

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

void MainWindow::on_checkBoxUseGlobal_clicked(bool checked)
{
    flagUseGlobalValues = checked;
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
        ui->horizontalSliderGlobalMin->setValue((int) 1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()));
        setMinVG(value);
    }
}

void MainWindow::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (flagDatasetInitialized) {
        ui->horizontalSliderGlobalMax->setValue((int) 1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()));
        setMaxVG(value);
    }
}

void MainWindow::on_comboBoxColormap_currentIndexChanged(int index)
{
    currentColormap = index;
    repaintSlices();
}

void MainWindow::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    file->closeDataset(selectedDataset->getName());
    selectedDataset = file->openDataset(selectedName + "/" + std::to_string(step));
    loadXYSlice(ui->verticalSliderXY->value());
    loadXZSlice(ui->verticalSliderXZ->value());
    loadYZSlice(ui->verticalSliderXZ->value());
}
