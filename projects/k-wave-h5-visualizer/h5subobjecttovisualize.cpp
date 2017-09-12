/**
 * @file        h5subobjecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing OpenedH5File::H5SubobjectToVisualize
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
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */


#include "h5subobjecttovisualize.h"

/**
 * @brief Creates H5SubobjectToVisualize object
 * @param[in] dataset Dataset
 * @param[in] type Object type
 * @param[in] openedH5File Opened file
 * @param[in] h5ObjectToVisualize Object to visualize
 * @param[in] parent Parent (optional)
 */
OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5Helper::Dataset *dataset, ObjectType type, OpenedH5File *openedH5File, H5ObjectToVisualize *h5ObjectToVisualize, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->h5ObjectToVisualize = h5ObjectToVisualize;
    this->objectName = QString::fromStdString(dataset->getName());
    this->objectOnlyName = QString::fromStdString(dataset->getOnlyName());
    this->type = type;
    this->dataset = dataset;

    loadObjectData();
    initialize();
}

/**
 * @brief Inicialization of important variables
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
 * @brief Destructor of H5SubobjectToVisualize
 *
 * Deletes threads and slice data.
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
 * @brief Set GUI initialized or not
 * @param[in] value True/False
 */
void OpenedH5File::H5SubobjectToVisualize::setGUIInitialized(bool value)
{
    GUIInitialized = value;
}

/**
 * @brief Is GUI Initialized?
 * @return True/False
 */
bool OpenedH5File::H5SubobjectToVisualize::isGUIInitialized()
{
    return GUIInitialized;
}

/**
 * @brief Returns subobject name
 * @return Subobject name
 */
QString OpenedH5File::H5SubobjectToVisualize::getName()
{
    return objectName;
}

/**
 * @brief Returns subobject only name
 * @return Subobject only name
 */
QString OpenedH5File::H5SubobjectToVisualize::getOnlyName() const
{
    return objectOnlyName;
}

/**
 * @brief Returns subobject type
 * @return Subobject type
 */
OpenedH5File::ObjectType OpenedH5File::H5SubobjectToVisualize::getType()
{
    return type;
}

/**
 * @brief Returns dataset
 * @return Dataset
 */
HDF5Helper::Dataset *OpenedH5File::H5SubobjectToVisualize::getDataset()
{
    return dataset;
}

/**
 * @brief Loads data about dataset or group (size, position, ...)
 */
void OpenedH5File::H5SubobjectToVisualize::loadObjectData()
{
    // Size of dataset (can be downsampled)
    size = HDF5Helper::Vector3D(dataset->getDims());
    originalSize = size;

    // Set 3D frame size
    frameSize = size;
    originalFrameSize = frameSize;

    // If downsampled
    if (dataset->hasAttribute(HDF5Helper::SRC_SIZE_X_ATTR)
            && dataset->hasAttribute(HDF5Helper::SRC_SIZE_Y_ATTR)
            && dataset->hasAttribute(HDF5Helper::SRC_SIZE_Z_ATTR)
            && dataset->hasAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR)
            ) {
        originalSize.x(dataset->readAttributeI(HDF5Helper::SRC_SIZE_X_ATTR, false));
        originalSize.y(dataset->readAttributeI(HDF5Helper::SRC_SIZE_Y_ATTR, false));
        originalSize.z(dataset->readAttributeI(HDF5Helper::SRC_SIZE_Z_ATTR, false));
        originalFrameSize = originalSize;
    }

    // Get chunk dimensions
    chunkSize = dataset->getChunkDims();

    // Get global min/max values
    dataset->findAndSetGlobalMinAndMaxValue();
    hsize_t minValueIndex;
    hsize_t maxValueIndex;
    dataset->getGlobalMinValue(minValue, minValueIndex);
    dataset->getGlobalMaxValue(maxValue, maxValueIndex);
    dataset->getGlobalMinValue(originalMinValue, minValueIndex);
    dataset->getGlobalMaxValue(originalMaxValue, maxValueIndex);

    HDF5Helper::convertlinearToMultiDim(minValueIndex, minValuePosition, dataset->getDims());
    HDF5Helper::convertlinearToMultiDim(maxValueIndex, maxValuePosition, dataset->getDims());


    if (type == dataset3D_t) {
        // Default step
        steps = 1;
        if (dataset->hasAttribute(HDF5Helper::POSITION_X_ATTR) && dataset->hasAttribute(HDF5Helper::POSITION_Y_ATTR) && dataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)) {
            originalPosition.z(dataset->readAttributeI(HDF5Helper::POSITION_Z_ATTR, false));
            originalPosition.y(dataset->readAttributeI(HDF5Helper::POSITION_Y_ATTR, false));
            originalPosition.x(dataset->readAttributeI(HDF5Helper::POSITION_X_ATTR, false));
            position = originalPosition;

            frameSize = HDF5Helper::Vector3D(openedH5File->getNDims());;
            originalFrameSize = frameSize;
        }

    } else if (type == dataset4D_t) {

        frameSize = HDF5Helper::Vector3D(openedH5File->getNDims());;
        originalFrameSize = frameSize;

        // Get position (was defined by sensor mask)
        if (dataset->hasAttribute(HDF5Helper::POSITION_X_ATTR) && dataset->hasAttribute(HDF5Helper::POSITION_Y_ATTR) && dataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)) {
            originalPosition.z(dataset->readAttributeI(HDF5Helper::POSITION_Z_ATTR, false));
            originalPosition.y(dataset->readAttributeI(HDF5Helper::POSITION_Y_ATTR, false));
            originalPosition.x(dataset->readAttributeI(HDF5Helper::POSITION_X_ATTR, false));
            position = originalPosition;
        }

        // Downsampled position
        if (size != originalSize) {
            float ratio = float(qMax(size.x(), qMax(size.y(), size.z()))) / qMax(originalSize.x(), qMax(originalSize.y(), originalSize.z()));
            position.x(hsize_t(originalPosition.x() * ratio));
            position.y(hsize_t(originalPosition.y() * ratio));
            position.z(hsize_t(originalPosition.z() * ratio));

            originalFrameSize.x(hsize_t(originalFrameSize.x() * ratio));
            originalFrameSize.y(hsize_t(originalFrameSize.y() * ratio));
            originalFrameSize.z(hsize_t(originalFrameSize.z() * ratio));
        }

        // Get number of steps
        steps = HDF5Helper::Vector4D(dataset->getDims()).w();
    }
}

/**
 * @brief Send signals images were changed
 */
void OpenedH5File::H5SubobjectToVisualize::changeImages()
{
    emit imageXYChanged(createImageXY(), index.z());
    emit imageXZChanged(createImageXZ(), index.y());
    emit imageYZChanged(createImageYZ(), index.x());
}

/**
 * @brief Reloads images
 */
void OpenedH5File::H5SubobjectToVisualize::reloadImages()
{
    setZIndex(index.z());
    setYIndex(index.y());
    setXIndex(index.x());
}

/**
 * @brief Slice XY loaded slot
 * @param[in] r Request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded(Request *r)
{
    XYloadedFlag = false;
    // Copy image data from request
    memcpy(dataXY, r->data, static_cast<size_t>(size.y() * size.x()) * sizeof(float));
    XYloadedFlag = true;
    if (index.z() == HDF5Helper::Vector3D(r->offset).z())
        currentXYLodaded = true;
    else
        currentXYLodaded = false;
    emit imageXYChanged(createImageXY(), HDF5Helper::Vector3D(r->offset).z());
    threadXY->deleteDoneRequest(r);
}

/**
 * @brief Slice XZ loaded slot
 * @param[in] r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded(Request *r)
{
    XZloadedFlag = false;
    // Copy image data from request
    memcpy(dataXZ, r->data, static_cast<size_t>(size.z() * size.x()) * sizeof(float));
    XZloadedFlag = true;
    if (index.y() == HDF5Helper::Vector3D(r->offset).y())
        currentXZLodaded = true;
    else
        currentXZLodaded = false;
    emit imageXZChanged(createImageXZ(), HDF5Helper::Vector3D(r->offset).y());
    threadXZ->deleteDoneRequest(r);
}

/**
 * @brief Slice YZ loaded slot
 * @param[in] r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded(Request *r)
{
    YZloadedFlag = false;
    // Copy image data from request
    memcpy(dataYZ, r->data, static_cast<size_t>(size.z() * size.y()) * sizeof(float));
    YZloadedFlag = true;
    if (index.x() == HDF5Helper::Vector3D(r->offset).x())
        currentYZLodaded = true;
    else
        currentYZLodaded = false;
    emit imageYZChanged(createImageYZ(), HDF5Helper::Vector3D(r->offset).x());
    threadYZ->deleteDoneRequest(r);
}

/**
 * @brief Is current XY Loaded?
 * @return True/False
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentXYLoaded()
{
    return currentXYLodaded;
}

/**
 * @brief Is current XZ Loaded?
 * @return True/False
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentXZLoaded()
{
    return currentXZLodaded;
}

/**
 * @brief Is current YZ Loaded?
 * @return True/False
 */
bool OpenedH5File::H5SubobjectToVisualize::isCurrentYZLoaded()
{
    return currentYZLodaded;
}

/**
 * @brief Are current slices Loaded?
 * @return True/False
 */
bool OpenedH5File::H5SubobjectToVisualize::areCurrentSlicesLoaded()
{
    if (currentXYLodaded && currentXZLodaded && currentYZLodaded)
        return true;
    else
        return false;
}

/**
 * @brief Creates XY image from loaded data
 * @return XY image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageXY()
{
    QImage qimage;
    if (XYloadedFlag) {
        qimage = QImage(int(size.x()), int(size.y()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.x() * size.y()), minValue, maxValue, dataXY, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief Creates XZ image from loaded data
 * @return XZ image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageXZ()
{
    QImage qimage;
    if (XZloadedFlag) {
        qimage = QImage(int(size.x()), int(size.z()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.x() * size.z()), minValue, maxValue, dataXZ, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief Creates YZ image from loaded data
 * @return YZ image
 */
QImage OpenedH5File::H5SubobjectToVisualize::createImageYZ()
{
    QImage qimage;
    if (YZloadedFlag) {
        qimage = QImage(int(size.y()), int(size.z()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.y() * size.z()), minValue, maxValue, dataYZ, qimage.bits(), colormap);
    }
    return qimage;
}

/**
 * @brief Returns H5ObjectToVisualize
 * @return H5ObjectToVisualize
 */
OpenedH5File::H5ObjectToVisualize *OpenedH5File::H5SubobjectToVisualize::getH5ObjectToVisualize() const
{
    return h5ObjectToVisualize;
}

/**
 * @brief Returns OpenedH5File
 * @return OpenedH5File
 */
OpenedH5File *OpenedH5File::H5SubobjectToVisualize::getOpenedH5File() const
{
    return openedH5File;
}

/**
 * @brief Returns data XY
 * @return Float XY data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataXY()
{
    return dataXY;
}

/**
 * @brief Returns data XZ
 * @return Float XZ data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataXZ()
{
    return dataXZ;
}

/**
 * @brief Returns data YZ
 * @return Float YZ data
 */
float *OpenedH5File::H5SubobjectToVisualize::getDataYZ()
{
    return dataYZ;
}

/**
 * @brief Returns current X index
 * @return Current X index
 */
hsize_t OpenedH5File::H5SubobjectToVisualize::getXIndex()
{
    return index.x();
}

/**
 * @brief Returns current Y index
 * @return Current Y index
 */
hsize_t OpenedH5File::H5SubobjectToVisualize::getYIndex()
{
    return index.y();
}

/**
 * @brief Returns current Z index
 * @return Current Z index
 */
hsize_t OpenedH5File::H5SubobjectToVisualize::getZIndex()
{
    return index.z();
}

/**
 * @brief Starts loading image data with new X index
 * @param[in] value X index
 */
void OpenedH5File::H5SubobjectToVisualize::setXIndex(hsize_t value)
{
    index.x(value);
    currentXYLodaded = false;
    if (type == dataset3D_t)
        threadYZ->createRequest(dataset, HDF5Helper::Vector3D(0, 0, value), HDF5Helper::Vector3D(size.z(), size.y(), 1));
    else
        threadYZ->createRequest(dataset, HDF5Helper::Vector4D(currentStep, 0, 0, value), HDF5Helper::Vector4D(1, size.z(), size.y(), 1));

    threadYZ->start();
}

/**
 * @brief Starts loading image data with new Y index
 * @param[in] value Y index
 */
void OpenedH5File::H5SubobjectToVisualize::setYIndex(hsize_t value)
{
    index.y(value);
    currentXZLodaded = false;
    if (type == dataset3D_t)
        threadXZ->createRequest(dataset, HDF5Helper::Vector3D(0, value, 0), HDF5Helper::Vector3D(size.z(), 1, size.x()));
    else
        threadXZ->createRequest(dataset, HDF5Helper::Vector4D(currentStep, 0, value, 0), HDF5Helper::Vector4D(1, size.z(), 1, size.x()));
    threadXZ->start();
}

/**
 * @brief Starts loading image data with new Z index
 * @param[in] value Z index
 */
void OpenedH5File::H5SubobjectToVisualize::setZIndex(hsize_t value)
{
    index.z(value);
    currentYZLodaded = false;
    if (type == dataset3D_t)
        threadXY->createRequest(dataset, HDF5Helper::Vector3D(value, 0, 0), HDF5Helper::Vector3D(1, size.y(), size.x()));
    else
        threadXY->createRequest(dataset, HDF5Helper::Vector4D(currentStep, value, 0, 0), HDF5Helper::Vector4D(1, 1, size.y(), size.x()));
    threadXY->start();
}

/**
 * @brief Returns alpha value (unused)
 * @return Alpha value
 */
float OpenedH5File::H5SubobjectToVisualize::getAlpha()
{
    return alpha;
}

/**
 * @brief Returns red value (unused)
 * @return Red value
 */
float OpenedH5File::H5SubobjectToVisualize::getRed()
{
    return red;
}

/**
 * @brief Returns green value (unused)
 * @return Green value
 */
float OpenedH5File::H5SubobjectToVisualize::getGreen()
{
    return green;
}

/**
 * @brief Returns blue value (unused)
 * @return Blue value
 */
float OpenedH5File::H5SubobjectToVisualize::getBlue()
{
    return blue;
}

/**
 * @brief Gets count (unused)
 * @return Count
 */
int OpenedH5File::H5SubobjectToVisualize::getCount()
{
    return count;
}

/**
 * @brief Sets alpha (unused)
 * @param[in] value Alpha value
 */
void OpenedH5File::H5SubobjectToVisualize::setAlpha(float value)
{
    alpha = value;
}

/**
 * @brief Sets red (unused)
 * @param[in] value Red value
 */
void OpenedH5File::H5SubobjectToVisualize::setRed(float value)
{
    red = value;
}

/**
 * @brief Sets green (unused)
 * @param[in] value Green value
 */
void OpenedH5File::H5SubobjectToVisualize::setGreen(float value)
{
    green = value;
}

/**
 * @brief Sets blue (unused)
 * @param[in] value Blue value
 */
void OpenedH5File::H5SubobjectToVisualize::setBlue(float value)
{
    blue = value;
}

/**
 * @brief Sets count (unused)
 * @param[in] value Count
 */
void OpenedH5File::H5SubobjectToVisualize::setCount(int value)
{
    count = value;
}

/**
 * @brief Returns size
 * @return Size
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getSize()
{
    return size;
}

/**
 * @brief Returns original size
 * @return Original size
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalSize()
{
    return originalSize;
}

/**
 * @brief Returns frame size
 * @return Frame size
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getFrameSize()
{
    return frameSize;
}

/**
 * @brief Returns original frame size
 * @return Original frame size
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize()
{
    return originalFrameSize;
}

/**
 * @brief Returns Position
 * @return Position
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getPos()
{
    return position;
}

/**
 * @brief Returns original position
 * @return Original position
 */
HDF5Helper::Vector3D OpenedH5File::H5SubobjectToVisualize::getOriginalPos()
{
    return originalPosition;
}

/**
 * @brief Returns steps
 * @return Steps
 */
hsize_t OpenedH5File::H5SubobjectToVisualize::getSteps()
{
    return steps;
}

/**
 * @brief Returns current step
 * @return Current step
 */
hsize_t OpenedH5File::H5SubobjectToVisualize::getCurrentStep()
{
    return currentStep;
}

/**
 * @brief Synchronizes loading time series
 * @param[in] step Step
 */
void OpenedH5File::H5SubobjectToVisualize::setCurrentStep(hsize_t step)
{
    if (type == dataset4D_t) {
        try {
            currentStep = step;
            reloadImages();
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

/**
 * @brief Returns colormap
 * @return Colormap
 */
ColorMap::Type OpenedH5File::H5SubobjectToVisualize::getColormap()
{
    return colormap;
}

/**
 * @brief Sets colormap
 * @param[in] colormap Colormap
 */
void OpenedH5File::H5SubobjectToVisualize::setColormap(ColorMap::Type colormap)
{
    this->colormap = colormap;
    changeImages();
}

/**
 * @brief Sets minimal value
 * @param[in] value Value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinValue(float value)
{
    minValue = value;
    changeImages();
}

/**
 * @brief Sets maximal value
 * @param[in] value Value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxValue(float value)
{
    maxValue = value;
    changeImages();
}

/**
 * @brief Returns minimal value
 * @return Minimal global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinValue()
{
    return minValue;
}

/**
 * @brief Returns maximal value
 * @return Maximal global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxValue()
{
    return maxValue;
}

/**
 * @brief Returns original minimal value
 * @return Minimal global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinValue()
{
    return originalMinValue;
}

/**
 * @brief Returns original maximal value
 * @return Maximal global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxValue()
{
    return originalMaxValue;
}

/**
 * @brief Returns value at 2D position on XY image
 * @param[in] x X-coord
 * @param[in] y Y-coord
 * @return Float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY(int x, int y)
{
    if (XYloadedFlag)
        return dataXY[static_cast<hsize_t>(x) + size.x() * static_cast<hsize_t>(y)];
    else
        return 0.0;
}

/**
 * @brief Returns value at 2D position on XZ image
 * @param[in] x X(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXZ(int x, int z)
{
    if (XZloadedFlag)
        return dataXZ[static_cast<hsize_t>(x) + size.x() * static_cast<hsize_t>(z)];
    else
        return 0.0;
}

/**
 * @brief Returns value at 2D position on YZ image
 * @param[in] y Y(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromYZ(int y, int z)
{
    if (YZloadedFlag)
        return dataYZ[static_cast<hsize_t>(y) + size.y() * static_cast<hsize_t>(z)];
    else
        return 0.0;
}

/**
 * @brief Creates simulation info data structure
 * @return Info structure
 */
QList<QPair<QString, QString>> OpenedH5File::H5SubobjectToVisualize::getInfo()
{
    QList<QPair<QString, QString>> info;
    if (type == dataset3D_t) {
        info.append(QPair<QString, QString>("Name", objectName));
        info.append(QPair<QString, QString>("Type", "3D dataset"));
        if (frameSize.x() != size.x() || frameSize.y() != size.y() || frameSize.z() != size.z())
            info.append(QPair<QString, QString>("Base size", QString::fromStdString(frameSize)));
        info.append(QPair<QString, QString>("Size", QString::fromStdString(originalSize)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Downsampling size", QString::fromStdString(size)));
        if (frameSize.x() != size.x() || frameSize.y() != size.y() || frameSize.z() != size.z())
            info.append(QPair<QString, QString>("Position", QString::fromStdString(position)));
        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
        info.append(QPair<QString, QString>("Min value position", QString::fromStdString(minValuePosition)));
        info.append(QPair<QString, QString>("Max value position", QString::fromStdString(maxValuePosition)));
    } else if (type == dataset4D_t) {
        info.append(QPair<QString, QString>("Name", objectName));
        info.append(QPair<QString, QString>("Type", "4D dataset"));
        info.append(QPair<QString, QString>("Base size", QString::fromStdString(originalFrameSize)));
        if (frameSize.x() != originalFrameSize.x() || frameSize.y() != originalFrameSize.y() || frameSize.z() != originalFrameSize.z())
            info.append(QPair<QString, QString>("Downsampling base size", QString::fromStdString(frameSize)));
        info.append(QPair<QString, QString>("Size", QString::number(steps) + " x " + QString::fromStdString(originalSize)));
        if (size.x() != originalSize.x() || size.y() != originalSize.y() || size.z() != originalSize.z())
            info.append(QPair<QString, QString>("Downsampling size", QString::fromStdString(dataset->getDims())));
        info.append(QPair<QString, QString>("Position", QString::fromStdString(originalPosition)));
        if (position.x() != originalPosition.x() || position.y() != originalPosition.y() || position.z() != originalPosition.z())
            info.append(QPair<QString, QString>("Downsampling position", QString::fromStdString(position)));
        info.append(QPair<QString, QString>("Chunk size", QString::fromStdString(chunkSize)));
        info.append(QPair<QString, QString>("Min value position", QString::fromStdString(minValuePosition)));
        info.append(QPair<QString, QString>("Max value position", QString::fromStdString(maxValuePosition)));
        info.append(QPair<QString, QString>("Steps", QString::number(steps)));
    }
    return info;
}
