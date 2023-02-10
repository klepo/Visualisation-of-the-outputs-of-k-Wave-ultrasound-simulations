/**
 * @file        slicedockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing SliceDockWidget class definition.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "slicedockwidget.h"

const std::map<SliceDockWidget::SliceType, std::string> SliceDockWidget::sliceTypeStr
    = { { XY, "XY" }, { XZ, "XZ" }, { YZ, "YZ" } };

/**
 * @brief Creates SliceDockWidget object
 * @param[in] parent Parent (optional)
 */
SliceDockWidget::SliceDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::SliceDockWidget)
{
    ui->setupUi(this);

    // Create loading animation
    // ui->labelLoading->setVisible(false);
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();
    // ui->labelLoading->setMovie(movie);

    QFont newFont = font();
    newFont.setBold(true);
    ui->spinBox->setFont(newFont);
    setSliceType(sliceType);
}

/**
 * @brief Destructor of SliceDockWidget
 *
 * Deletes ui and clears slices.
 */
SliceDockWidget::~SliceDockWidget()
{
    clear();
    delete ui;
}

/**
 * @brief Returns slice type
 * @return Slice type
 */
SliceDockWidget::SliceType SliceDockWidget::getSliceType() const
{
    return sliceType;
}

/**
 * @brief Sets slice type
 * @param[in] value Slice type
 */
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

/**
 * @brief Sets H5ObjectToVisualize object
 * @param[in] object H5ObjectToVisualize
 */
void SliceDockWidget::setObject(H5ObjectToVisualize *object)
{
    // Set image name
    imageName = QString::fromStdString(object->getFile()->getRawFilename()) + "_-_" + object->getOnlyName();
    ui->imageWidget->setFilename(getImageFilename());

    if (sliceType == XY) {
        ui->horizontalSlider->setMaximum(int(object->getDatasetSize().z() - 1));
        ui->spinBox->setMaximum(int(object->getDatasetSize().z() - 1));
        ui->spinBox->setValue(int(object->getZIndex()));
        connect(object, SIGNAL(dataXYLoadingStarted()), this, SLOT(showLabelLoading()));
        connect(object, SIGNAL(lastXYReadingTimeNs(qint64)), this, SLOT(showReadingTime(qint64)));
        connect(object, SIGNAL(zIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageXYChanged(QImage)), ui->imageWidget, SLOT(showImage(QImage)));
        connect(object, SIGNAL(currentXYLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object,
                SLOT(setHoveredPointInImageXY(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setZIndex(int)));
        connect(this, SIGNAL(visibilityChanged(bool)), object, SLOT(setDataXYLoadingFlag(bool)));
        object->setDataXYLoadingFlag(isVisible());
        if (object->isCurrentXYLoaded()) {
            ui->imageWidget->showImage(object->getImageXY());
        }
    } else if (sliceType == XZ) {
        ui->horizontalSlider->setMaximum(int(object->getDatasetSize().y() - 1));
        ui->spinBox->setMaximum(int(object->getDatasetSize().y() - 1));
        ui->spinBox->setValue(int(object->getYIndex()));
        connect(object, SIGNAL(dataXZLoadingStarted()), this, SLOT(showLabelLoading()));
        connect(object, SIGNAL(lastXZReadingTimeNs(qint64)), this, SLOT(showReadingTime(qint64)));
        connect(object, SIGNAL(yIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageXZChanged(QImage)), ui->imageWidget, SLOT(showImage(QImage)));
        connect(object, SIGNAL(currentXZLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object,
                SLOT(setHoveredPointInImageXZ(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setYIndex(int)));
        connect(this, SIGNAL(visibilityChanged(bool)), object, SLOT(setDataXZLoadingFlag(bool)));
        object->setDataXZLoadingFlag(isVisible());
        if (object->isCurrentXZLoaded()) {
            ui->imageWidget->showImage(object->getImageXZ());
        }
    } else if (sliceType == YZ) {
        ui->horizontalSlider->setMaximum(int(object->getDatasetSize().x() - 1));
        ui->spinBox->setMaximum(int(object->getDatasetSize().x() - 1));
        ui->spinBox->setValue(int(object->getXIndex()));
        connect(object, SIGNAL(dataYZLoadingStarted()), this, SLOT(showLabelLoading()));
        connect(object, SIGNAL(lastYZReadingTimeNs(qint64)), this, SLOT(showReadingTime(qint64)));
        connect(object, SIGNAL(xIndexChanged(int)), this, SLOT(setSliceIndex(int)));
        connect(object, SIGNAL(imageYZChanged(QImage)), ui->imageWidget, SLOT(showImage(QImage)));
        connect(object, SIGNAL(currentYZLoaded()), this, SLOT(hideLabelLoading()));
        connect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), object,
                SLOT(setHoveredPointInImageYZ(int, int)));
        connect(this, SIGNAL(sliceIndexChanged(int)), object, SLOT(setXIndex(int)));
        connect(this, SIGNAL(visibilityChanged(bool)), object, SLOT(setDataYZLoadingFlag(bool)));
        object->setDataYZLoadingFlag(isVisible());
        if (object->isCurrentYZLoaded()) {
            ui->imageWidget->showImage(object->getImageYZ());
        }
    }
}

/**
 * @brief Sets slice index
 * @param[in] value Slice index
 */
void SliceDockWidget::setSliceIndex(int value)
{
    ui->spinBox->setValue(value);
}

/**
 * @brief Clears slice, sets default values and disconnects signals
 */
void SliceDockWidget::clear()
{
    disconnect(ui->imageWidget, SIGNAL(hoveredPointInImage(int, int)), nullptr, nullptr);
    disconnect(this, SIGNAL(sliceIndexChanged(int)), nullptr, nullptr);
    // disconnect(this, SIGNAL(visibilityChanged(bool)), nullptr, nullptr);
    ui->horizontalSlider->setMaximum(0);
    ui->spinBox->setMaximum(0);
    ui->spinBox->setValue(0);
    ui->imageWidget->clearImage();
    hideLabelLoading();
    ui->labelInfo->clear();
    imageName.clear();
}

/**
 * @brief Spin box value changed slot
 * @param[in] value Spin box value
 */
void SliceDockWidget::on_spinBox_valueChanged(int value)
{
    if (sliceType == XY) {
        setWindowTitle("XY slice (Z = " + QString::number(value) + ")");
    } else if (sliceType == XZ) {
        setWindowTitle("XZ slice (Y = " + QString::number(value) + ")");
    } else if (sliceType == YZ) {
        setWindowTitle("YZ slice (X = " + QString::number(value) + ")");
    }
    ui->imageWidget->setFilename(getImageFilename());
    emit sliceIndexChanged(value);
}

/**
 * @brief Toggles label loading
 * @param[in] value True/False
 */
void SliceDockWidget::toggleLabelLoading(bool value)
{
    if (value) {
        showLabelLoading();
    } else {
        hideLabelLoading();
    }
}

/**
 * @brief Hides label loading
 */
void SliceDockWidget::hideLabelLoading()
{
    ui->labelLoading->clear();
}

/**
 * @brief Shows label loading
 */
void SliceDockWidget::showLabelLoading()
{
    ui->labelLoading->setMovie(movie);
}

/**
 * @brief Shows reading time
 * @param[in] value Time in nanoseconds
 */
void SliceDockWidget::showReadingTime(qint64 value)
{
    if (sliceType == XY) {
        ui->labelInfo->setText("Last read time: " + QString::number(double(value) / 1000000, 'f', 3) + " ms");
    } else if (sliceType == XZ) {
        ui->labelInfo->setText("Last read time: " + QString::number(double(value) / 1000000, 'f', 3) + " ms");
    } else if (sliceType == YZ) {
        ui->labelInfo->setText("Last read time: " + QString::number(double(value) / 1000000, 'f', 3) + " ms");
    }
}

/**
 * @brief Sets style color
 * @param[in] color Color
 */
void SliceDockWidget::setColor(QColor color)
{
    QString colorString = "color: rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", "
                          + QString::number(color.blue()) + ");";
    ui->spinBox->setStyleSheet(colorString);
}

/**
 * @brief Creates image filename
 * @return Image filename
 */
QString SliceDockWidget::getImageFilename()
{
    return imageName + "_-_" + QString::fromStdString(sliceTypeStr.at(sliceType)) + "_"
           + QString::number(ui->spinBox->value());
}
