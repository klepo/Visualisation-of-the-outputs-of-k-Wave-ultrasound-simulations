/**
 * @file        h5subobjecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The implementation file containing the H5SubobjectToVisualize class.
 *              Object of class H5ObjectToVisualize is for one dataset or group of datasets
 *              with time series and has own threads and view settings.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */


#include "h5subobjecttovisualize.h"

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize
 * @param dataset
 * @param openedH5File
 * @param parent
 */
OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5Helper::HDF5Dataset *dataset, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->objectName = QString::fromStdString(dataset->getName());
    this->type = H5G_DATASET;

    this->group = 0;
    this->dataset = dataset;

    loadObjectData();
    initialize();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize
 * @param group
 * @param openedH5File
 * @param parent
 */
OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5Helper::HDF5Group *group, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->objectName = QString::fromStdString(group->getName());
    this->type = H5G_GROUP;

    this->group = group;
    // Set first dataset of time series
    this->dataset = group->openDataset(0);
    // TODO
    this->dataset->setMaxNumberOfElmsToLoad(HDF5Helper::HDF5Vector3D(this->dataset->getDims()).getSize());

    loadObjectData();
    initialize();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::initialize Inicialization of important variables
 */
void OpenedH5File::H5SubobjectToVisualize::initialize()
{
    // Allocation memory for slices
    dataXY = new float[size.y() * size.x()];
    dataXZ = new float[size.z() * size.x()];
    dataYZ = new float[size.z() * size.y()];

    // Create threads
    threadXY = new HDF5ReadingThread();
    connect(threadXY, SIGNAL(requestDone(Request *)), this, SLOT(sliceXYLoaded(Request *)));
    threadXZ = new HDF5ReadingThread();
    connect(threadXZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceXZLoaded(Request *)));
    threadYZ = new HDF5ReadingThread();
    connect(threadYZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceYZLoaded(Request *)));
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::~H5SubobjectToVisualize
 */
OpenedH5File::H5SubobjectToVisualize::~H5SubobjectToVisualize()
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

    delete[] dataXY;
    delete[] dataXZ;
    delete[] dataYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setGUIInitialized
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setGUIInitialized(bool value)
{
    GUIInitialized = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isGUIInitialized
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isGUIInitialized()
{
    return GUIInitialized;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getName
 * @return subobject name
 */
QString OpenedH5File::H5SubobjectToVisualize::getName()
{
    return objectName;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getType
 * @return subobject type
 */
H5G_obj_t OpenedH5File::H5SubobjectToVisualize::getType()
{
    return type;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getDataset
 * @return dataset
 */
HDF5Helper::HDF5Dataset *OpenedH5File::H5SubobjectToVisualize::getDataset()
{
    return dataset;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getGroup
 * @return group (can be NULL)
 */
HDF5Helper::HDF5Group *OpenedH5File::H5SubobjectToVisualize::getGroup()
{
    return group;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::loadObjectData Load data about dataset or group (size, position, ...)
 */
void OpenedH5File::H5SubobjectToVisualize::loadObjectData()
{
    if (type == H5G_DATASET && group == 0 && dataset != 0) {
        // Check resolution
        if (dataset->hasAttribute("dwnsmpl")) {
            dwnsmpl = dataset->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
        }

        // Original size withnout downsampling
        originalSize = HDF5Helper::HDF5Vector3D(openedH5File->getNDims());

        // Size of dataset (can be downsampled)
        size = HDF5Helper::HDF5Vector3D(dataset->getDims());

        // Set 3D frame size
        originalFrameSize = originalSize;
        frameSize = size;

        // Get chunk dimensions
        chunkSize = dataset->getChunkDims();

        // Get global min/max values
        dataset->findAndSetGlobalMinAndMaxValue();
        dataset->getGlobalMinValue(minValue);
        dataset->getGlobalMaxValue(maxValue);
        dataset->getGlobalMinValue(originalMinValue);
        dataset->getGlobalMaxValue(originalMaxValue);

        // Default step
        steps = 1;
    } else if (type == H5G_GROUP && group != 0 && dataset != 0) {
        // Check resolution
        if (group->hasAttribute("dwnsmpl")) {
            dwnsmpl = group->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
        }

        // Size of dataset (can be downsampled)
        originalSize = HDF5Helper::HDF5Vector3D(dataset->getDims());
        size = originalSize;

        // Get position (was defined by sensor mask)
        if (dataset->hasAttribute("positionZ") && dataset->hasAttribute("positionY") && dataset->hasAttribute("positionX")) {
            originalPos.z(dataset->readAttributeI("positionZ"));
            originalPos.y(dataset->readAttributeI("positionY"));
            originalPos.x(dataset->readAttributeI("positionX"));
            pos = originalPos;
        }

        // Set frame size
        originalFrameSize = HDF5Helper::HDF5Vector3D(openedH5File->getNDims());
        frameSize = originalFrameSize;

        // If downsamples -> compute original size, position and frame size
        if (dwnsmpl > 0) {
            // TODO better way to get original values...
            uint64_t maxN = qMax(openedH5File->getNX(), qMax(openedH5File->getNY(), openedH5File->getNZ()));
            double ratio = static_cast<double>(maxN) / dwnsmpl;

            frameSize.z(qRound(frameSize.z() / ratio));
            frameSize.y(qRound(frameSize.y() / ratio));
            frameSize.x(qRound(frameSize.x() / ratio));

            originalSize.z(qRound(size.z() * ratio));
            originalSize.y(qRound(size.y() * ratio));
            originalSize.x(qRound(size.x() * ratio));
            // This is not very clean...
            if (size.z() == 1) originalSize.z(1);
            if (size.y() == 1) originalSize.y(1);
            if (size.x() == 1) originalSize.x(1);

            originalPos.z(qRound((pos.z() + 1) * ratio - 1));
            originalPos.y(qRound((pos.y() + 1) * ratio - 1));
            originalPos.x(qRound((pos.x() + 1) * ratio - 1));
            // This is not very clean...
            if (pos.z() == 0) originalPos.z(0);
            if (pos.y() == 0) originalPos.y(0);
            if (pos.x() == 0) originalPos.x(0);
        }

        // Get chunk dims
        chunkSize = dataset->getChunkDims();

        // Get global mi/max/ values
        dataset->findAndSetGlobalMinAndMaxValue();
        dataset->getGlobalMinValue(minValue);
        dataset->getGlobalMaxValue(maxValue);
        dataset->getGlobalMinValue(originalMinValue);
        dataset->getGlobalMaxValue(originalMaxValue);

        // Get number of steps
        steps = HDF5Helper::HDF5Vector4D(dataset->getDims()).w();
    }
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::changeImages Image data was changed -> send signals
 */
void OpenedH5File::H5SubobjectToVisualize::changeImages()
{
    emit imageXYChanged(createImageXY(), index.z());
    emit imageXZChanged(createImageXZ(), index.y());
    emit imageYZChanged(createImageYZ(), index.x());
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::reloadImages
 */
void OpenedH5File::H5SubobjectToVisualize::reloadImages()
{
    setZIndex(index.z());
    setYIndex(index.y());
    setXIndex(index.x());
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded(Request *r)
{
    XYloadedFlag = false;
    // Copy image data from request
    memcpy(dataXY, r->data, static_cast<size_t>(size.y() * size.x()) * sizeof(float));
    XYloadedFlag = true;
    if (index.z() == HDF5Helper::HDF5Vector3D(r->offset).z())
        currentXYLodaded = true;
    else
        currentXYLodaded = false;
    emit imageXYChanged(createImageXY(), HDF5Helper::HDF5Vector3D(r->offset).z());
    threadXY->deleteDoneRequest(r);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded(Request *r)
{
    XZloadedFlag = false;
    // Copy image data from request
    memcpy(dataXZ, r->data, static_cast<size_t>(size.z() * size.x()) * sizeof(float));
    XZloadedFlag = true;
    if (index.y() == HDF5Helper::HDF5Vector3D(r->offset).y())
        currentXZLodaded = true;
    else
        currentXZLodaded = false;
    emit imageXZChanged(createImageXZ(), HDF5Helper::HDF5Vector3D(r->offset).y());
    threadXZ->deleteDoneRequest(r);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded(Request *r)
{
    YZloadedFlag = false;
    // Copy image data from request
    memcpy(dataYZ, r->data, static_cast<size_t>(size.z() * size.y()) * sizeof(float));
    YZloadedFlag = true;
    if (index.x() == HDF5Helper::HDF5Vector3D(r->offset).x())
        currentYZLodaded = true;
    else
        currentYZLodaded = false;
    emit imageYZChanged(createImageYZ(), HDF5Helper::HDF5Vector3D(r->offset).x());
    threadYZ->deleteDoneRequest(r);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isCurrentXYLoaded
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentXYLoaded()
{
    return currentXYLodaded;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isCurrentXZLoaded
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentXZLoaded()
{
    return currentXZLodaded;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isCurrentYZLoaded
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentYZLoaded()
{
    return currentYZLodaded;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::areCurrentSlicesLoaded
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::areCurrentSlicesLoaded()
{
    if (currentXYLodaded && currentXZLodaded && currentYZLodaded)
        return true;
    else
        return false;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::createImageXY Create cv::Mat XY image from loaded data
 * @return XY image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageXY()
{
    QImage qimage;
    if (XYloadedFlag) {
        qimage = QImage(size.x(), size.y(), QImage::Format_RGB32);
        ColorMap::applyColorMap(size.x() * size.y(), minValue, maxValue, dataXY, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::createImageXZ Create cv::Mat XZ image from loaded data
 * @return XZ image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageXZ()
{
    QImage qimage;
    if (XZloadedFlag) {
        qimage = QImage(size.x(), size.z(), QImage::Format_RGB32);
        ColorMap::applyColorMap(size.x() * size.z(), minValue, maxValue, dataXZ, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::createImageYZ Create cv::Mat YZ image from loaded data
 * @return YZ image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageYZ()
{
    QImage qimage;
    if (YZloadedFlag) {
        qimage = QImage(size.y(), size.z(), QImage::Format_RGB32);
        ColorMap::applyColorMap(size.y() * size.z(), minValue, maxValue, dataYZ, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getDataXY
 * @return float XY data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataXY()
{
    return dataXY;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getDataXZ
 * @return float XZ data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataXZ()
{
    return dataXZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getDataYZ
 * @return float YZ data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataYZ()
{
    return dataYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getXIndex
 * @return current X index
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getXIndex()
{
    return index.x();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getYIndex
 * @return current Y index
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getYIndex()
{
    return index.y();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getZIndex
 * @return current Z index
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getZIndex()
{
    return index.z();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setXIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setXIndex(uint64_t value)
{
    index.x(value);
    currentXYLodaded = false;
    if (type == H5G_DATASET)
        threadYZ->createRequest(dataset, HDF5Helper::HDF5Vector3D(0, 0, value), HDF5Helper::HDF5Vector3D(size.z(), size.y(), 1));
    else
        threadYZ->createRequest(dataset, HDF5Helper::HDF5Vector4D(currentStep, 0, 0, value), HDF5Helper::HDF5Vector4D(1, size.z(), size.y(), 1));

    threadYZ->start();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setYIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setYIndex(uint64_t value)
{
    index.y(value);
    currentXZLodaded = false;
    if (type == H5G_DATASET)
        threadXZ->createRequest(dataset, HDF5Helper::HDF5Vector3D(0, value, 0), HDF5Helper::HDF5Vector3D(size.z(), 1, size.x()));
    else
        threadXZ->createRequest(dataset, HDF5Helper::HDF5Vector4D(currentStep, 0, value, 0), HDF5Helper::HDF5Vector4D(1, size.z(), 1, size.x()));
    threadXZ->start();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setZIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setZIndex(uint64_t value)
{
    index.z(value);
    currentYZLodaded = false;
    if (type == H5G_DATASET)
        threadXY->createRequest(dataset, HDF5Helper::HDF5Vector3D(value, 0, 0), HDF5Helper::HDF5Vector3D(1, size.y(), size.x()));
    else
        threadXY->createRequest(dataset, HDF5Helper::HDF5Vector4D(currentStep, value, 0, 0), HDF5Helper::HDF5Vector4D(1, 1, size.y(), size.x()));
    threadXY->start();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getAlpha
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getAlpha()
{
    return alpha;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getRed
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getRed()
{
    return red;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getGreen
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getGreen()
{
    return green;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getBlue
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getBlue()
{
    return blue;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getCount
 * @return count
 */
int OpenedH5File::H5SubobjectToVisualize::getCount()
{
    return count;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setAlpha
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setAlpha(float value)
{
    alpha = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setRed
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setRed(float value)
{
    red = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setGreen
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setGreen(float value)
{
    green = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setBlue
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setBlue(float value)
{
    blue = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setCount
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setCount(int value)
{
    count = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getSize
 * @return size
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getSize()
{
    return size;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalSize
 * @return original size
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalSize()
{
    return originalSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getFrameSize
 * @return frame size
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getFrameSize()
{
    return frameSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize
 * @return original frame size
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize()
{
    return originalFrameSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getPos
 * @return position
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getPos()
{
    return pos;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalPos
 * @return original position
 */
HDF5Helper::HDF5Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalPos()
{
    return originalPos;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getSteps
 * @return number of steps
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getSteps()
{
    return steps;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getCurrentStep
 * @return current step
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getCurrentStep()
{
    return currentStep;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setCurrentStep Synchronize loading time series
 * @param value step
 * @param thread3D 3D data loading thread for sycnhronization
 */
void OpenedH5File::H5SubobjectToVisualize::setCurrentStep(uint64_t value)
{
    if (type == H5G_GROUP) {
        try {
            currentStep = value;
            reloadImages();
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getColormap
 * @return colormap
 */
ColorMap::Type OpenedH5File::H5SubobjectToVisualize::getColormap()
{
    return colormap;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setColormap
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setColormap(ColorMap::Type colormap)
{
    this->colormap = colormap;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMinValue
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinValue(float value)
{
    minValue = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMaxValue
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxValue(float value)
{
    maxValue = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMinValue
 * @return min global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinValue()
{
    return minValue;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMaxValue
 * @return max global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxValue()
{
    return maxValue;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMinValue
 * @return min global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinValue()
{
    return originalMinValue;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMaxValue
 * @return max global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxValue()
{
    return originalMaxValue;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY Get value at 2D position on XY image
 * @param x x-coord
 * @param y y-coord
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY(int x, int y)
{
    if (XYloadedFlag)
        return dataXY[static_cast<hsize_t>(x) + size.x() * static_cast<hsize_t>(y)];
    else
        return 0.0;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXZ Get value at 2D position on XZ image
 * @param x x(x)-coord
 * @param z z(y)-coord
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXZ(int x, int z)
{
    if (XZloadedFlag)
        return dataXZ[static_cast<hsize_t>(x) + size.x() * static_cast<hsize_t>(z)];
    else
        return 0.0;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromYZ Get value at 2D position on YZ image
 * @param y y(x)-coord
 * @param z z(y)-coord
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromYZ(int y, int z)
{
    if (YZloadedFlag)
        return dataYZ[static_cast<hsize_t>(y) + size.y() * static_cast<hsize_t>(z)];
    else
        return 0.0;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getInfo Create simulation info data strusture
 * @return info structure
 */
QList<QPair<QString, QString>> OpenedH5File::H5SubobjectToVisualize::getInfo()
{
    QList<QPair<QString, QString>> info;
    if (type == H5G_DATASET && dataset != 0) {
        info.append(QPair<QString, QString>("Name", objectName));
        info.append(QPair<QString, QString>("Type", "3D dataset"));
        info.append(QPair<QString, QString>("Size", QString::fromStdString(originalSize)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Downsampling size", QString::fromStdString(size)));
        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
    } else if (type == H5G_GROUP && group != 0 && dataset != 0) {
        info.append(QPair<QString, QString>("Name", objectName));
        info.append(QPair<QString, QString>("Type", "Mask type dataset group"));
        info.append(QPair<QString, QString>("Base size", QString::fromStdString(originalFrameSize)));
        if (frameSize.x() != originalFrameSize.x() || frameSize.y() != originalFrameSize.y() || frameSize.z() != originalFrameSize.z())
            info.append(QPair<QString, QString>("Downsampling base size", QString::fromStdString(frameSize)));
        info.append(QPair<QString, QString>("Size", QString::number(steps) + " x " + QString::fromStdString(originalSize)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Downsampling size", QString::fromStdString(dataset->getDims())));
        info.append(QPair<QString, QString>("Position", QString::fromStdString(originalPos)));
        if (pos.x() != originalPos.x() || pos.y() != originalPos.y() || pos.z() != originalPos.z())
            info.append(QPair<QString, QString>("Downsampling position", QString::fromStdString(pos)));
        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
        info.append(QPair<QString, QString>("Steps", QString::number(steps)));
    }
    return info;
}
