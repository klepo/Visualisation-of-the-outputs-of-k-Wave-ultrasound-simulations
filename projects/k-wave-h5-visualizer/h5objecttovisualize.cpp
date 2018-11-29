/**
 * @file        h5objecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              29 November  2018 (updated)
 *
 * @brief       The implementation file containing H5ObjectToVisualize
 *              class definition.
 *
 * Object of class H5ObjectToVisualize is for one dataset or group of datasets with
 * time series and has own threads and view settings.
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

#include "h5objecttovisualize.h"

/**
 * @brief Creates H5ObjectToVisualize object
 * @param[in] dataset Dataset
 * @param[in] type Object type
 * @param[in] parent Parent (optional)
 */
H5ObjectToVisualize::H5ObjectToVisualize(H5Helper::Dataset *dataset, H5OpenedFile::ObjectType type, QObject *parent)
    : QObject(parent)
    , dataset(dataset)
    , type(type)
{
    // Create threads for slices
    threadXY = new H5ReadingThread();
    connect(threadXY, SIGNAL(requestDone(Request *)), this, SLOT(sliceXYLoaded(Request *)));
    threadXZ = new H5ReadingThread();
    connect(threadXZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceXZLoaded(Request *)));
    threadYZ = new H5ReadingThread();
    connect(threadYZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceYZLoaded(Request *)));

    thread3D = new H5ReadingThread();
    connect(thread3D, SIGNAL(requestDone(Request *)), this, SLOT(data3DLoaded(Request *)));

    loadObjectData();
    initialize();
}

/**
 * @brief Destructor of H5ObjectToVisualize
 *
 * Deletes threads and slice data.
 */
H5ObjectToVisualize::~H5ObjectToVisualize()
{
    threadXY->clearRequests();
    threadXY->wait();
    //threadXY->terminate();
    threadXY->deleteLater();

    threadXZ->clearRequests();
    threadXZ->wait();
    //threadXZ->terminate();
    threadXZ->deleteLater();

    threadYZ->clearRequests();
    threadYZ->wait();
    //threadYZ->terminate();
    threadYZ->deleteLater();

    thread3D->clearRequests();
    //thread3D->terminate();
    thread3D->wait();
    thread3D->deleteLater();

    disconnectSignals();

    if (dataXY) {
        delete[] dataXY;
        dataXY = nullptr;
    }
    if (dataXYCC) {
        delete[] dataXYCC;
        dataXYCC = nullptr;
    }
    if (dataXYLC) {
        delete[] dataXYLC;
        dataXYLC = nullptr;
    }
    if (dataXZ) {
        delete[] dataXZ;
        dataXZ = nullptr;
    }
    if (dataXZCC) {
        delete[] dataXZCC;
        dataXZCC = nullptr;
    }
    if (dataXZLC) {
        delete[] dataXZLC;
        dataXZLC = nullptr;
    }
    if (dataYZ) {
        delete[] dataYZ;
        dataYZ = nullptr;
    }
    if (dataYZCC) {
        delete[] dataYZCC;
        dataYZCC = nullptr;
    }
    if (dataYZLC) {
        delete[] dataYZLC;
        dataYZLC = nullptr;
    }
    if (data3D) {
        delete[] data3D;
        data3D = nullptr;
    }
    if (data3DCC) {
        delete[] data3DCC;
        data3DCC = nullptr;
    }
    if (data3DLC) {
        delete[] data3DLC;
        data3DLC = nullptr;
    }

    if (compressHelper) {
        delete compressHelper;
        compressHelper = nullptr;
    }
}

/**
 * @brief Returns data XY
 * @return Float XY data
 */
const float *H5ObjectToVisualize::getDataXY() const
{
    return dataXY;
}

/**
 * @brief Returns data XZ
 * @return Float XZ data
 */
const float *H5ObjectToVisualize::getDataXZ() const
{
    return dataXZ;
}

/**
 * @brief Returns data YZ
 * @return Float YZ data
 */
const float *H5ObjectToVisualize::getDataYZ() const
{
    return dataYZ;
}

/**
 * @brief Returns data 3D
 * @return Data 3D
 */
const float *H5ObjectToVisualize::getData3D() const
{
    return data3D;
}

/**
 * @brief Returns data 3D for last compress coefficient
 * @return Data 3D for last compress coefficient
 */
const float *H5ObjectToVisualize::getData3DLC() const
{
    return data3DLC;
}

/**
 * @brief Returns data 3D for current compress coefficient
 * @return Data 3D for current compress coefficient
 */
const float *H5ObjectToVisualize::getData3DCC() const
{
    return data3DCC;
}

/**
 * @brief Returns current X index
 * @return Current X index
 */
hsize_t H5ObjectToVisualize::getXIndex() const
{
    return index.x();
}

/**
 * @brief Returns current Y index
 * @return Current Y index
 */
hsize_t H5ObjectToVisualize::getYIndex() const
{
    return index.y();
}

/**
 * @brief Returns current Z index
 * @return Current Z index
 */
hsize_t H5ObjectToVisualize::getZIndex() const
{
    return index.z();
}

/**
 * @brief Returns steps
 * @return Steps
 */
hsize_t H5ObjectToVisualize::getSteps() const
{
    return steps;
}

/**
 * @brief Returns current step
 * @return Current step
 */
hsize_t H5ObjectToVisualize::getCurrentStep() const
{
    return index.t();
}

/**
 * @brief Returns local step
 * @return Local step
 */
hsize_t H5ObjectToVisualize::getLocalStep() const
{
    return thread3D->getLocalStep();
}

/**
 * @brief Returns image XY
 * @return image XY
 */
QImage H5ObjectToVisualize::getImageXY() const
{
    QImage qimage = QImage(int(size.x()), int(size.y()), QImage::Format_RGB32);
    ColorMap::applyColorMap(int(size.x() * size.y()), minValue, maxValue, dataXY, qimage.bits(), colormap);
    return qimage;}

/**
 * @brief Returns image XZ
 * @return image XZ
 */
QImage H5ObjectToVisualize::getImageXZ() const
{
    QImage qimage = QImage(int(size.x()), int(size.z()), QImage::Format_RGB32);
    ColorMap::applyColorMap(int(size.x() * size.z()), minValue, maxValue, dataXZ, qimage.bits(), colormap);
    return qimage;}

/**
 * @brief Returns image YZ
 * @return image YZ
 */
QImage H5ObjectToVisualize::getImageYZ() const
{
    QImage qimage  = QImage(int(size.y()), int(size.z()), QImage::Format_RGB32);
    ColorMap::applyColorMap(int(size.y() * size.z()), minValue, maxValue, dataYZ, qimage.bits(), colormap);
    return qimage;}

/**
 * @brief Returns minimal value
 * @return Minimal global current value
 */
float H5ObjectToVisualize::getMinValue() const
{
    return minValue;
}

/**
 * @brief Returns maximal value
 * @return Maximal global current value
 */
float H5ObjectToVisualize::getMaxValue() const
{
    return maxValue;
}

/**
 * @brief Returns original minimal value
 * @return Minimal global original value
 */
float H5ObjectToVisualize::getOriginalMinValue() const
{
    return originalMinValue;
}

/**
 * @brief Returns original maximal value
 * @return Maximal global original value
 */
float H5ObjectToVisualize::getOriginalMaxValue() const
{
    return originalMaxValue;
}

/**
 * @brief Returns colormap
 * @return Colormap
 */
ColorMap::Type H5ObjectToVisualize::getColormap() const
{
    return colormap;
}

/**
 * @brief Returns opacity
 * @return opacity
 */
QVector<float> H5ObjectToVisualize::getOpacity() const
{
  return opacity;
}

/**
 * @brief Returns min/max values trim flag
 * @return Min/max values trim flag
 */
bool H5ObjectToVisualize::getMinMaxValuesTrim() const
{
    return minMaxValuesTrimFlag;
}

/**
 * @brief Returns frame size
 * @return Frame size
 */
H5Helper::Vector3D H5ObjectToVisualize::getFrameSize() const
{
    return frameSize;
}

/**
 * @brief Returns original frame size
 * @return Original frame size
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalFrameSize() const
{
    return originalFrameSize;
}

/**
 * @brief Returns size
 * @return Size
 */
H5Helper::Vector3D H5ObjectToVisualize::getDatasetSize() const
{
    return size;
}

/**
 * @brief Returns original size
 * @return Original size
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalDatasetSize() const
{
    return originalSize;
}

/**
 * @brief Returns Position
 * @return Position
 */
H5Helper::Vector3D H5ObjectToVisualize::getDatasetPosition() const
{
    return position;
}

/**
 * @brief Returns original position
 * @return Original position
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalDatasetPosition() const
{
    return originalPosition;
}

/**
 * @brief Returns subobject name
 * @return Subobject name
 */
QString H5ObjectToVisualize::getName() const
{
    return QString::fromStdString(dataset->getName());
}

/**
 * @brief Returns subobject only name
 * @return Subobject only name
 */
QString H5ObjectToVisualize::getOnlyName() const
{
    return QString::fromStdString(dataset->getOnlyName());;
}

/**
 * @brief Returns subobject type
 * @return Subobject type
 */
H5OpenedFile::ObjectType H5ObjectToVisualize::getType() const
{
    return type;
}

/**
 * @brief Creates simulation info data structure
 * @return Info structure
 */
QList<QPair<QString, QString>> H5ObjectToVisualize::getInfo() const
{
    QList<QPair<QString, QString>> info;
    if (type == H5OpenedFile::DATASET_3D) {
        info.append(QPair<QString, QString>("Name", getName()));
        info.append(QPair<QString, QString>("Type", "3D dataset (" + QString::fromStdString(dataset->getTypeString()) + ")"));
        info.append(QPair<QString, QString>("Size", QString::fromStdString(size)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Original size", QString::fromStdString(originalSize)));

        if (frameSize.x() != size.x() || frameSize.y() != size.y() || frameSize.z() != size.z()) {
            info.append(QPair<QString, QString>("Base size", QString::fromStdString(frameSize)));
            if (frameSize.x() != originalFrameSize.x() || frameSize.y() != originalFrameSize.y() || frameSize.z() != originalFrameSize.z())
                info.append(QPair<QString, QString>("Original base size", QString::fromStdString(originalFrameSize)));

            info.append(QPair<QString, QString>("Position", QString::fromStdString(position)));
            if (position.x() != originalPosition.x() || position.y() != originalPosition.y() || position.z() != originalPosition.z())
                info.append(QPair<QString, QString>("Original position", QString::fromStdString(originalPosition)));
        }

        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
        info.append(QPair<QString, QString>("Min value position", QString::fromStdString(minValuePosition)));
        info.append(QPair<QString, QString>("Max value position", QString::fromStdString(maxValuePosition)));
    } else if (type == H5OpenedFile::DATASET_4D) {
        info.append(QPair<QString, QString>("Name", getName()));
        info.append(QPair<QString, QString>("Type", "4D dataset (" + QString::fromStdString(dataset->getTypeString()) + ")"));

        if (compressHelper) {
            info.append(QPair<QString, QString>("Compression period", QString::number(compressHelper->getPeriod())));
            info.append(QPair<QString, QString>("Multiple of overlap size", QString::number(compressHelper->getMos())));
            info.append(QPair<QString, QString>("Number of harmonics", QString::number(compressHelper->getHarmonics())));
        }

        info.append(QPair<QString, QString>("Size", QString::number(steps) + " x " + QString::fromStdString(size)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Original size", QString::number(steps) + " x " + QString::fromStdString(originalSize)));

        if (frameSize.x() != size.x() || frameSize.y() != size.y() || frameSize.z() != size.z()) {
            info.append(QPair<QString, QString>("Base size", QString::fromStdString(frameSize)));
            if (frameSize.x() != originalFrameSize.x() || frameSize.y() != originalFrameSize.y() || frameSize.z() != originalFrameSize.z())
                info.append(QPair<QString, QString>("Original base size", QString::fromStdString(originalFrameSize)));

            info.append(QPair<QString, QString>("Position", QString::fromStdString(position)));
            if (position.x() != originalPosition.x() || position.y() != originalPosition.y() || position.z() != originalPosition.z())
                info.append(QPair<QString, QString>("Original position", QString::fromStdString(originalPosition)));
        }

        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
        info.append(QPair<QString, QString>("Min value position", QString::fromStdString(minValuePosition)));
        info.append(QPair<QString, QString>("Max value position", QString::fromStdString(maxValuePosition)));
        info.append(QPair<QString, QString>("Steps", QString::number(steps)));
    }
    return info;
}

/**
 * @brief Returns value at 2D position on XY image
 * @param[in] x X-coord
 * @param[in] y Y-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromXY(int x, int y) const
{
    return dataXY[hsize_t(x) + size.x() * hsize_t(y)];
}

/**
 * @brief Returns value at 2D position on XZ image
 * @param[in] x X(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromXZ(int x, int z) const
{
    return dataXZ[hsize_t(x) + size.x() * hsize_t(z)];
}

/**
 * @brief Returns value at 2D position on YZ image
 * @param[in] y Y(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromYZ(int y, int z) const
{
    return dataYZ[hsize_t(y) + size.y() * hsize_t(z)];
}

/**
 * @brief Is current XY Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXYLoaded() const
{
    return currentXYLoadedFlag;
}

/**
 * @brief Is current XZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXZLoaded() const
{
    return currentXZLoadedFlag;
}

/**
 * @brief Is current YZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentYZLoaded() const
{
    return currentYZLoadedFlag;
}

/**
 * @brief Are current slices Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::areCurrentSlicesLoaded() const
{
    if (currentXYLoadedFlag && currentXZLoadedFlag && currentYZLoadedFlag)
        return true;
    else
        return false;
}

/**
 * @brief Are current data 3D loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentData3DLoaded() const
{
    return currentData3DLoadedFlag;
}

/**
 * @brief Is current step loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentStepLoaded() const
{
    if (loadData3DFlag) {
        if (isCurrentData3DLoaded() && areCurrentSlicesLoaded()) {
            return true;
        }
    } else if (areCurrentSlicesLoaded()) {
        return true;
    }
    return false;
}

/**
 * @brief Returns OpenedH5File
 * @return OpenedH5File
 */
const H5Helper::File *H5ObjectToVisualize::getFile() const
{
    return dataset->getFile();
}

/**
 * @brief Returns compress helper
 * @return Compress helper
 */
const H5Helper::CompressHelper *H5ObjectToVisualize::getCompressHelper() const
{
  return compressHelper;
}

/**
 * @brief Is object selected?
 * @return True/False
 */
bool H5ObjectToVisualize::isSelected() const
{
    return selectedFlag;
}

/**
 * @brief Sets minimal value
 * @param[in] value Value
 */
void H5ObjectToVisualize::setMinValue(float value)
{
    minValue = value;
    emit minValueChanged(value);
    changeImages();
}

/**
 * @brief Sets maximal value
 * @param[in] value Value
 */
void H5ObjectToVisualize::setMaxValue(float value)
{
    maxValue = value;
    emit maxValueChanged(value);
    changeImages();
}

/**
 * @brief Sets colormap
 * @param[in] colormap Colormap
 */
void H5ObjectToVisualize::setColormap(ColorMap::Type colormap)
{
    this->colormap = colormap;
    emit colormapChanged(colormap);
    changeImages();
}

/**
 * @brief Sets colormap
 * @param[in] colormap Colormap
 */
void H5ObjectToVisualize::setColormap(int colormap)
{
    setColormap(static_cast<ColorMap::Type>(colormap));
}

/**
 * @brief Sets opacity
 * @param[in] opacity Opacity
 */
void H5ObjectToVisualize::setOpacity(const QVector<float> &opacity)
{
    this->opacity = opacity;
    emit(opacityChanged(opacity));
}

/**
 * @brief Sets min/max values trim
 * @param[in] value True/False
 */
void H5ObjectToVisualize::setMinMaxValuesTrim(bool value)
{
    minMaxValuesTrimFlag = value;
    emit minMaxValuesTrimChanged(value);
}

/**
 * @brief Starts loading image data with new X index
 * @param[in] value X index
 */
void H5ObjectToVisualize::setXIndex(int value)
{
    index.x(value);
    emit xIndexChanged(value);
    currentYZLoadedFlag = false;
    if (type == H5OpenedFile::DATASET_3D) {
        threadYZ->createRequest(dataset, H5Helper::Vector3D(0, 0, hsize_t(value)), H5Helper::Vector3D(size.z(), size.y(), 1), dataYZ, dataYZLC, dataYZCC);
    } else {
        threadYZ->createRequest(dataset, H5Helper::Vector4D(index.t(), 0, 0, hsize_t(value)), H5Helper::Vector4D(1, size.z(), size.y(), 1), dataYZ, dataYZLC, dataYZCC);
    }
    emit dataYZLoadingStarted();
    threadYZ->start();
}

/**
 * @brief Starts loading image data with new Y index
 * @param[in] value Y index
 */
void H5ObjectToVisualize::setYIndex(int value)
{
    index.y(value);
    emit yIndexChanged(value);
    currentXZLoadedFlag = false;
    if (type == H5OpenedFile::DATASET_3D)
        threadXZ->createRequest(dataset, H5Helper::Vector3D(0, hsize_t(value), 0), H5Helper::Vector3D(size.z(), 1, size.x()), dataXZ, dataXZLC, dataXZCC);
    else
        threadXZ->createRequest(dataset, H5Helper::Vector4D(index.t(), 0, hsize_t(value), 0), H5Helper::Vector4D(1, size.z(), 1, size.x()), dataXZ, dataXZLC, dataXZCC);
    emit dataXZLoadingStarted();
    threadXZ->start();
}

/**
 * @brief Starts loading image data with new Z index
 * @param[in] value Z index
 */
void H5ObjectToVisualize::setZIndex(int value)
{
    index.z(value);
    emit zIndexChanged(value);
    currentXYLoadedFlag = false;
    if (type == H5OpenedFile::DATASET_3D)
        threadXY->createRequest(dataset, H5Helper::Vector3D(hsize_t(value), 0, 0), H5Helper::Vector3D(1, size.y(), size.x()), dataXY, dataXYLC, dataXYCC);
    else
        threadXY->createRequest(dataset, H5Helper::Vector4D(index.t(), hsize_t(value), 0, 0), H5Helper::Vector4D(1, 1, size.y(), size.x()), dataXY, dataXYLC, dataXYCC);
    emit dataXYLoadingStarted();
    threadXY->start();
}

/**
 * @brief Sets position to max value position
 */
void H5ObjectToVisualize::setToMaxValuePosition()
{
    setXIndex(int(H5Helper::Vector3D(maxValuePosition).x()));
    setYIndex(int(H5Helper::Vector3D(maxValuePosition).y()));
    setZIndex(int(H5Helper::Vector3D(maxValuePosition).z()));
    setCurrentStep(int(H5Helper::Vector4D(maxValuePosition).t()));
}

/**
 * @brief Sets position to min value position
 */
void H5ObjectToVisualize::setToMinValuePosition()
{
    setXIndex(int(H5Helper::Vector3D(minValuePosition).x()));
    setYIndex(int(H5Helper::Vector3D(minValuePosition).y()));
    setZIndex(int(H5Helper::Vector3D(minValuePosition).z()));
    setCurrentStep(int(H5Helper::Vector4D(minValuePosition).t()));
}

/**
 * @brief Synchronizes loading time series
 * @param[in] step Step
 */
void H5ObjectToVisualize::setCurrentStep(int step)
{
    if (type == H5OpenedFile::DATASET_4D) {
        try {
            index.t(hsize_t(step));
            emit stepChanged(step);
            if (loadData3DFlag) {
                load3Ddata();
            }
            reloadSlices();
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

/**
 * @brief Reloads images
 */
void H5ObjectToVisualize::reloadSlices()
{
    reloadXY();
    reloadXZ();
    reloadYZ();
}

/**
 * @brief Reloads XY slice
 */
void H5ObjectToVisualize::reloadXY()
{
    setZIndex(int(index.z()));
}

/**
 * @brief Reloads XZ slice
 */
void H5ObjectToVisualize::reloadXZ()
{
    setYIndex(int(index.y()));
}

/**
 * @brief Reloads YZ slice
 */
void H5ObjectToVisualize::reloadYZ()
{
    setXIndex(int(index.x()));
}

/**
 * @brief Sets object selection flag
 * @param[in] value True/False
 */
void H5ObjectToVisualize::setSelected(bool value)
{
    selectedFlag = value;
}

/**
 * @brief Toggles selection flag
 */
void H5ObjectToVisualize::toggleSelected()
{
    setSelected(!selectedFlag);
}

/**
 * @brief Sets data 3D loading flag
 * @param[in] value Data 3D loading flag
 */
void H5ObjectToVisualize::setData3DLoadingFlag(bool value)
{
  loadData3DFlag = value;
  if (loadData3DFlag && !currentData3DLoadedFlag)
      load3Ddata();
}

/**
 * @brief Sets hovered point in image XY
 * @param[in] x X position
 * @param[in] y Y position
 */
void H5ObjectToVisualize::setHoveredPointInImageXY(int x, int y)
{
    emit(hoveredPointInImage(getValueAtPointFromXY(x, y)));
}

/**
 * @brief Sets hovered point in image XZ
 * @param[in] x X position
 * @param[in] z Z position
 */
void H5ObjectToVisualize::setHoveredPointInImageXZ(int x, int z)
{
    emit(hoveredPointInImage(getValueAtPointFromXZ(x, z)));
}

/**
 * @brief Sets hovered point in image YZ
 * @param[in] y Y position
 * @param[in] z Z position
 */
void H5ObjectToVisualize::setHoveredPointInImageYZ(int y, int z)
{
    emit(hoveredPointInImage(getValueAtPointFromYZ(y, z)));
}

/**
 * @brief Disconnects all signals
 */
void H5ObjectToVisualize::disconnectSignals()
{
    disconnect(this, SIGNAL(minValueChanged(float)), nullptr, nullptr);
    disconnect(this, SIGNAL(maxValueChanged(float)), nullptr, nullptr);
    disconnect(this, SIGNAL(colormapChanged(ColorMap::Type)), nullptr, nullptr);
    disconnect(this, SIGNAL(opacityChanged(QVector<float>)), nullptr, nullptr);
    disconnect(this, SIGNAL(minMaxValuesTrimChanged(bool)), nullptr, nullptr);

    disconnect(this, SIGNAL(currentSlicesLoaded()), nullptr, nullptr);
    disconnect(this, SIGNAL(currentXYLoaded()), nullptr, nullptr);
    disconnect(this, SIGNAL(currentXZLoaded()), nullptr, nullptr);
    disconnect(this, SIGNAL(currentYZLoaded()), nullptr, nullptr);
    disconnect(this, SIGNAL(currentData3DLoaded()), nullptr, nullptr);
    disconnect(this, SIGNAL(currentStepLoaded()), nullptr, nullptr);

    disconnect(this, SIGNAL(dataYZLoadingStarted()), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXZLoadingStarted()), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXYLoadingStarted()), nullptr, nullptr);
    disconnect(this, SIGNAL(data3DLoadingStarted()), nullptr, nullptr);

    disconnect(this, SIGNAL(xIndexChanged(int)), nullptr, nullptr);
    disconnect(this, SIGNAL(yIndexChanged(int)), nullptr, nullptr);
    disconnect(this, SIGNAL(zIndexChanged(int)), nullptr, nullptr);
    disconnect(this, SIGNAL(stepChanged(int)), nullptr, nullptr);

    disconnect(this, SIGNAL(data3DChanged(const float *)), nullptr, nullptr);
    disconnect(this, SIGNAL(data3DCompressChanged(const float *, const float *)), nullptr, nullptr);
    disconnect(this, SIGNAL(localStep3DCompressChanged(hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXYChanged(const float *, hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXZChanged(const float *, hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataYZChanged(const float *, hsize_t)), nullptr, nullptr);

    disconnect(this, SIGNAL(imageXYChanged(QImage)), nullptr, nullptr);
    disconnect(this, SIGNAL(imageXZChanged(QImage)), nullptr, nullptr);
    disconnect(this, SIGNAL(imageYZChanged(QImage)), nullptr, nullptr);

    disconnect(this, SIGNAL(hoveredPointInImage(float)), nullptr, nullptr);
    disconnect(this, SIGNAL(lastXYReadingTimeNs(qint64)), nullptr, nullptr);
    disconnect(this, SIGNAL(lastXZReadingTimeNs(qint64)), nullptr, nullptr);
    disconnect(this, SIGNAL(lastYZReadingTimeNs(qint64)), nullptr, nullptr);
    disconnect(this, SIGNAL(last3DReadingTimeNs(qint64)), nullptr, nullptr);
}

/**
 * @brief Slice XY loaded slot
 * @param[in] request Request
 */
void H5ObjectToVisualize::sliceXYLoaded(Request *request)
{
    if (index.z() == H5Helper::Vector3D(request->offset).z()) {
        currentXYLoadedFlag = true;
        emit currentXYLoaded();
    } else {
        currentXYLoadedFlag = false;
    }
    emit imageXYChanged(getImageXY());
    emit dataXYChanged(dataXY, H5Helper::Vector3D(request->offset).z());
    emit lastXYReadingTimeNs(request->nsecsElapsed);
    checkCurrentDataIsLoaded();
    threadXY->deleteDoneRequest(request);
}

/**
 * @brief Slice XZ loaded slot
 * @param[in] request Request
 */
void H5ObjectToVisualize::sliceXZLoaded(Request *request)
{
    if (index.y() == H5Helper::Vector3D(request->offset).y()) {
        currentXZLoadedFlag = true;
        emit currentXZLoaded();
    } else {
        currentXZLoadedFlag = false;
    }
    emit imageXZChanged(getImageXZ());
    emit dataXZChanged(dataXZ, H5Helper::Vector3D(request->offset).y());
    emit lastXZReadingTimeNs(request->nsecsElapsed);
    checkCurrentDataIsLoaded();
    threadXZ->deleteDoneRequest(request);
}

/**
 * @brief Slice YZ loaded slot
 * @param[in] request Request
 */
void H5ObjectToVisualize::sliceYZLoaded(Request *request)
{
    if (index.x() == H5Helper::Vector3D(request->offset).x()) {
        currentYZLoadedFlag = true;
        emit currentYZLoaded();
    } else {
        currentYZLoadedFlag = false;
    }
    emit imageYZChanged(getImageYZ());
    emit dataYZChanged(dataYZ, H5Helper::Vector3D(request->offset).x());
    emit lastYZReadingTimeNs(request->nsecsElapsed);
    checkCurrentDataIsLoaded();
    threadYZ->deleteDoneRequest(request);
}

/**
 * @brief Data 3D loaded slot
 * @param[in] request Request
 */
void H5ObjectToVisualize::data3DLoaded(Request *request)
{
    if (index.t() == H5Helper::Vector4D(request->offset).t()) {
        currentData3DLoadedFlag = true;
        emit currentData3DLoaded();
    } else {
        currentData3DLoadedFlag = false;
    }
    if (compressHelper) {
        if (request->dataCCChanged || request->dataLCChanged) {
            emit data3DCompressChanged(data3DLC, data3DCC);
        }
        emit localStep3DCompressChanged(thread3D->getLocalStep());
    } else {
        emit data3DChanged(data3D);
    }
    emit last3DReadingTimeNs(request->nsecsElapsed);
    checkCurrentDataIsLoaded();
    thread3D->deleteDoneRequest(request);
}

/**
 * @brief Inicialization of important variables
 */
void H5ObjectToVisualize::initialize()
{
    // Allocation memory for slices
    dataXY = new float[size.y() * size.x()]();
    dataXZ = new float[size.z() * size.x()]();
    dataYZ = new float[size.z() * size.y()]();
    data3D = new float[size.getSize()]();

    if (compressHelper) {
        dataXYCC = new float[size.y() * size.x() * compressHelper->getStride()]();
        dataXYLC = new float[size.y() * size.x() * compressHelper->getStride()]();
        dataXZCC = new float[size.z() * size.x() * compressHelper->getStride()]();
        dataXZLC = new float[size.z() * size.x() * compressHelper->getStride()]();
        dataYZCC = new float[size.z() * size.y() * compressHelper->getStride()]();
        dataYZLC = new float[size.z() * size.y() * compressHelper->getStride()]();
        data3DCC = new float[size.getSize() * compressHelper->getStride()]();
        data3DLC = new float[size.getSize() * compressHelper->getStride()]();
    }

    index.x(H5Helper::Vector3D(maxValuePosition).x());
    index.y(H5Helper::Vector3D(maxValuePosition).y());
    index.z(H5Helper::Vector3D(maxValuePosition).z());
    index.t(H5Helper::Vector4D(maxValuePosition).t());
}

/**
 * @brief Loads data about dataset or group (size, position, ...)
 */
void H5ObjectToVisualize::loadObjectData()
{
    // Get chunk dimensions
    chunkSize = dataset->getChunkDims();

    // Get global min/max values
    dataset->findAndSetGlobalMinAndMaxValue();
    hsize_t minValueIndex;
    hsize_t maxValueIndex;
    dataset->getGlobalMinValue(minValue, minValueIndex);
    dataset->getGlobalMaxValue(maxValue, maxValueIndex);
    originalMinValue = minValue;
    originalMaxValue = maxValue;

    H5Helper::convertlinearToMultiDim(minValueIndex, minValuePosition, dataset->getDims());
    H5Helper::convertlinearToMultiDim(maxValueIndex, maxValuePosition, dataset->getDims());

    // Size of dataset (can be downsampled)
    size = H5Helper::Vector3D(dataset->getDims());
    originalSize = size;

    // Set 3D frame size
    frameSize = H5Helper::Vector3D(dataset->getFile()->getNDims());;
    originalFrameSize = frameSize;

    // If masked
    if (dataset->hasAttribute(H5Helper::POSITION_X_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Y_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Z_ATTR)) {
        position.z(dataset->readAttributeI(H5Helper::POSITION_Z_ATTR, false));
        position.y(dataset->readAttributeI(H5Helper::POSITION_Y_ATTR, false));
        position.x(dataset->readAttributeI(H5Helper::POSITION_X_ATTR, false));
        originalPosition = position;
    } else if (type == H5OpenedFile::DATASET_4D) {
        // Try to get position from sensor_mask_corners dataset
        if (dataset->getFile()->objExistsByName(H5Helper::SENSOR_MASK_CORNERS_DATASET)) {
            H5Helper::Dataset *sensorMaskCornersDataset = dataset->getFile()->openDataset(H5Helper::SENSOR_MASK_CORNERS_DATASET);
            hsize_t *sensorMaskCornersData = nullptr;
            sensorMaskCornersDataset->readDataset(sensorMaskCornersData);
            // Name of the dataset to index
            hsize_t i = hsize_t(std::stoi(dataset->getOnlyName()));
            dataset->findAndSetGlobalMinAndMaxValue();
            dataset->setAttribute(H5Helper::POSITION_Z_ATTR, sensorMaskCornersData[(i - 1) * 6 + 2] - 1);
            dataset->setAttribute(H5Helper::POSITION_Y_ATTR, sensorMaskCornersData[(i - 1) * 6 + 1] - 1);
            dataset->setAttribute(H5Helper::POSITION_X_ATTR, sensorMaskCornersData[(i - 1) * 6 + 0] - 1);

            position.z(sensorMaskCornersData[(i - 1) * 6 + 2] - 1);
            position.y(sensorMaskCornersData[(i - 1) * 6 + 1] - 1);
            position.x(sensorMaskCornersData[(i - 1) * 6 + 0] - 1);
            originalPosition = position;
            delete[] sensorMaskCornersData;
            sensorMaskCornersData = nullptr;
            dataset->getFile()->closeDataset(sensorMaskCornersDataset);
        }
    }

    // If downsampled
    if (dataset->hasAttribute(H5Helper::SRC_SIZE_X_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_SIZE_Y_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_SIZE_Z_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_DATASET_NAME_ATTR)
            ) {
        originalSize.x(dataset->readAttributeI(H5Helper::SRC_SIZE_X_ATTR, false));
        originalSize.y(dataset->readAttributeI(H5Helper::SRC_SIZE_Y_ATTR, false));
        originalSize.z(dataset->readAttributeI(H5Helper::SRC_SIZE_Z_ATTR, false));

        if (dataset->hasAttribute(H5Helper::SRC_POSITION_Z_ATTR)
                    && dataset->hasAttribute(H5Helper::SRC_POSITION_Y_ATTR)
                    && dataset->hasAttribute(H5Helper::SRC_POSITION_X_ATTR)
                ) {
            originalPosition.x(dataset->readAttributeI(H5Helper::SRC_POSITION_X_ATTR, false));
            originalPosition.y(dataset->readAttributeI(H5Helper::SRC_POSITION_Y_ATTR, false));
            originalPosition.z(dataset->readAttributeI(H5Helper::SRC_POSITION_Z_ATTR, false));
        }

        float ratio = float(qMax(size.x(), qMax(size.y(), size.z()))) / qMax(originalSize.x(), qMax(originalSize.y(), originalSize.z()));
        frameSize.x(qRound(frameSize.x() * ratio));
        frameSize.y(qRound(frameSize.y() * ratio));
        frameSize.z(qRound(frameSize.z() * ratio));
    }

    if (type == H5OpenedFile::DATASET_4D) {
        // Get number of steps
        steps = H5Helper::Vector4D(dataset->getDims()).w();
    }

    if (dataset->getType() == H5Helper::DatasetType::CUBOID_ATTR_C || dataset->getType() == H5Helper::DatasetType::CUBOID_C) {
        hsize_t mos = dataset->hasAttribute(H5Helper::C_MOS_ATTR) ? dataset->readAttributeI(H5Helper::C_MOS_ATTR) : 1;
        hsize_t harmonics = dataset->hasAttribute(H5Helper::C_HARMONICS_ATTR) ? dataset->readAttributeI(H5Helper::C_HARMONICS_ATTR) : 1;
        compressHelper = new H5Helper::CompressHelper(dataset->readAttributeI(H5Helper::C_PERIOD_ATTR), mos, harmonics);

        size.x(size.x() / compressHelper->getStride());
        originalSize.x(originalSize.x() / compressHelper->getStride());

        steps = (steps + 2) * compressHelper->getOSize();
        H5Helper::convertlinearToMultiDim(minValueIndex, minValuePosition, H5Helper::Vector4D(steps, size));
        H5Helper::convertlinearToMultiDim(maxValueIndex, maxValuePosition, H5Helper::Vector4D(steps, size));

        threadXY->setCompressHelper(compressHelper);
        threadXZ->setCompressHelper(compressHelper);
        threadYZ->setCompressHelper(compressHelper);
        thread3D->setCompressHelper(compressHelper);
    }
}

/**
 * @brief Checks current data is loaded
 */
void H5ObjectToVisualize::checkCurrentDataIsLoaded()
{
    if (loadData3DFlag) {
        if (isCurrentData3DLoaded() && areCurrentSlicesLoaded()) {
            emit currentSlicesLoaded();
            emit currentStepLoaded();
        }
    } else if (areCurrentSlicesLoaded()) {
        emit currentSlicesLoaded();
        emit currentStepLoaded();
    }
}

/**
 * @brief Send signals images were changed
 */
void H5ObjectToVisualize::changeImages()
{
    emit imageXYChanged(getImageXY());
    emit imageXZChanged(getImageXZ());
    emit imageYZChanged(getImageYZ());
}

/**
 * @brief Starts loading of 3D data
 */
void H5ObjectToVisualize::load3Ddata()
{
    thread3D->createRequest(dataset, index.t(), data3D, data3DLC, data3DCC);
    emit data3DLoadingStarted();
    thread3D->start();
}
