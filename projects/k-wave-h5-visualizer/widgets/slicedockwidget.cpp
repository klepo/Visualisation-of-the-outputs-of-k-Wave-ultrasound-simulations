/**
 * @file        slicedockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The implementation file containing SliceDockWidget class definition.
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

#include <slicedockwidget.h>

const std::map<SliceDockWidget::SliceType, std::string> SliceDockWidget::sliceTypeStr = {
    {XY, "XY"},
    {XZ, "XZ"},
    {YZ, "YZ"}
};

SliceDockWidget::SliceDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SliceDockWidget)
{
    ui->setupUi(this);

    // Create loading animation
    //ui->labelLoading->setVisible(false);
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();
    //ui->labelLoading->setMovie(movie);

    QFont newFont = font();
    newFont.setBold(true);
    setFont(newFont);
    setSliceType(sliceType);
}

SliceDockWidget::~SliceDockWidget()
{
    clear();
    delete ui;
}

SliceDockWidget::SliceType SliceDockWidget::getSliceType() const
{
    return sliceType;
}

void SliceDockWidget::setSliceType(const SliceType &value)
{
    sliceType = value;
    if (sliceType == XY) {
        setColor(QColor(0, 0, 255));
        setWindowTitle("XY slice");
    } else if (sliceType == XZ) {
        setColor(QColor(0, 170, 0));
        setWindowTitle("XZ slice");
    } else if (sliceType == YZ) {
        setColor(QColor(255, 0, 0));
        setWindowTitle("YZ slice");
    }
}

void SliceDockWidget::setObject(H5ObjectToVisualize *object)
{
    // Set image name
    imageName = QString::fromStdString(object->getFile()->getRawFilename()) + "_-_" + object->getName();

    if (sliceType == XY) {
        ui->verticalSlider->setMaximum(int(object->getSize().z() - 1));
        ui->spinBox->setMaximum(int(object->getSize().z() - 1));
        ui->spinBox->setValue(int(object->getZIndex()));
        connect(object, SIGNAL(dataXYLoadingStarted()), this, SLOT(showLabelLoading()));
        if (!object->isCurrentXYLoaded()) {
            object->reloadXY();
        } else {
            repaintImage(object->getImageXY());
        }
        connect(object, SIGNAL(zIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageXYChanged(QImage)), this, SLOT(repaintImage(QImage)));
        connect(object, SIGNAL(currentXYLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object, SLOT(setHoveredPointInImageXY(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setZIndex(int)));
    } else if (sliceType == XZ) {
        ui->verticalSlider->setMaximum(int(object->getSize().y() - 1));
        ui->spinBox->setMaximum(int(object->getSize().y() - 1));
        ui->spinBox->setValue(int(object->getYIndex()));
        connect(object, SIGNAL(dataXZLoadingStarted()), this, SLOT(showLabelLoading()));
        if (!object->isCurrentXZLoaded()) {
            object->reloadXZ();
        } else {
            repaintImage(object->getImageXZ());
        }
        connect(object, SIGNAL(yIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageXZChanged(QImage)), this, SLOT(repaintImage(QImage)));
        connect(object, SIGNAL(currentXZLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object, SLOT(setHoveredPointInImageXZ(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setYIndex(int)));
    } else if (sliceType == YZ) {
        ui->verticalSlider->setMaximum(int(object->getSize().x() - 1));
        ui->spinBox->setMaximum(int(object->getSize().x() - 1));
        ui->spinBox->setValue(int(object->getXIndex()));
        connect(object, SIGNAL(dataYZLoadingStarted()), this, SLOT(showLabelLoading()));
        if (!object->isCurrentYZLoaded()) {
            object->reloadYZ();
        } else {
            repaintImage(object->getImageYZ());
        }
        connect(object, SIGNAL(xIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageYZChanged(QImage)), this, SLOT(repaintImage(QImage)));
        connect(object, SIGNAL(currentYZLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object, SLOT(setHoveredPointInImageYZ(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setXIndex(int)));
    }
}

void SliceDockWidget::setSliceIndex(int value)
{
    ui->spinBox->setValue(value);
}

void SliceDockWidget::clear()
{
    disconnect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), nullptr, nullptr);
    disconnect(this, SIGNAL(sliceIndexChanged(int)), nullptr, nullptr);
    ui->verticalSlider->setMaximum(0);
    ui->spinBox->setMaximum(0);
    ui->spinBox->setValue(0);
    ui->imageWidget->clearImage();
    hideLabelLoading();
    imageName.clear();
}

void SliceDockWidget::on_spinBox_valueChanged(int value)
{
    if (sliceType == XY) {
        setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
    } else if (sliceType == XZ) {
        setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
    } else if (sliceType == YZ) {
        setWindowTitle("YZ slice (X = " + QString::number(value) + ")");
    }
    emit sliceIndexChanged(value);
}

/**
 * @brief Sets new XY image if it is loaded
 * @param[in] image Image data of XY slice
 * @param[in] index Index of XY slice
 */
void SliceDockWidget::repaintImage(QImage image)
{
    // Set image data to image widget
    ui->imageWidget->showImage(image, QPoint(0, 0), getImageFilename());
}

void SliceDockWidget::toggleLabelLoading(bool value)
{
    if (value) {
        showLabelLoading();
    } else {
        hideLabelLoading();
    }
}

void SliceDockWidget::hideLabelLoading()
{
    ui->labelLoading->clear();
}

void SliceDockWidget::showLabelLoading()
{
    ui->labelLoading->setMovie(movie);
}

void SliceDockWidget::setColor(QColor color)
{
    QString colorString = "color: rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ");";
    setStyleSheet(colorString);
}

QString SliceDockWidget::getImageFilename()
{
    return imageName + "_-_" + QString::fromStdString(sliceTypeStr.at(sliceType)) + "_" + QString::number(ui->spinBox->value());
}


