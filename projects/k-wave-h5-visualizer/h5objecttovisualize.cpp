/**
 * @file        h5subobjecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
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
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "h5objecttovisualize.h"

/**
 * @brief Creates H5ObjectToVisualize object
 * @param[in] dataset Dataset
 * @param[in] type Object type
 * @param[in] openedH5File Opened file
 * @param[in] h5ObjectToVisualize Object to visualize
 * @param[in] parent Parent (optional)
 */
H5ObjectToVisualize::H5ObjectToVisualize(H5Helper::Dataset *dataset, H5OpenedFile::ObjectType type, H5OpenedFile *openedH5File, QObject *parent)
    : QObject(parent)
    , dataset(dataset)
    , type(type)
    , openedH5File(openedH5File)
{
    loadObjectData();
    initialize();
}

/**
 * @brief Inicialization of important variables
 */
void H5ObjectToVisualize::initialize()
{
    // Allocation memory for slices
    dataXY = new float[size.y() * size.x()];
    dataXZ = new float[size.z() * size.x()];
    dataYZ = new float[size.z() * size.y()];

    // Create threads for slices
    threadXY = new H5ReadingThread();
    connect(threadXY, SIGNAL(requestDone(Request *)), this, SLOT(sliceXYLoaded(Request *)));
    threadXZ = new H5ReadingThread();
    connect(threadXZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceXZLoaded(Request *)));
    threadYZ = new H5ReadingThread();
    connect(threadYZ, SIGNAL(requestDone(Request *)), this, SLOT(sliceYZLoaded(Request *)));

    index.x(H5Helper::Vector3D(maxValuePosition).x());
    index.y(H5Helper::Vector3D(maxValuePosition).y());
    index.z(H5Helper::Vector3D(maxValuePosition).z());
    if (type == H5OpenedFile::DATASET_4D)
        currentStep = H5Helper::Vector4D(maxValuePosition).t();
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

    disconnect(this, SIGNAL(imageXYChanged(QImage)), 0, 0);
    disconnect(this, SIGNAL(imageXZChanged(QImage)), 0, 0);
    disconnect(this, SIGNAL(imageYZChanged(QImage)), 0, 0);
    disconnect(this, SIGNAL(dataXYChanged(float *, hsize_t)), 0, 0);
    disconnect(this, SIGNAL(dataXZChanged(float *, hsize_t)), 0, 0);
    disconnect(this, SIGNAL(dataYZChanged(float *, hsize_t)), 0, 0);
    disconnect(this, SIGNAL(opacityChanged(QVector<float>)), 0, 0);
    disconnect(this, SIGNAL(minValueChanged(float)), 0, 0);
    disconnect(this, SIGNAL(maxValueChanged(float)), 0, 0);
    disconnect(this, SIGNAL(colormapChanged(ColorMap::Type)), 0, 0);

    delete[] dataXY;
    delete[] dataXZ;
    delete[] dataYZ;
}

/**
 * @brief Returns subobject name
 * @return Subobject name
 */
QString H5ObjectToVisualize::getName()
{
    return QString::fromStdString(dataset->getName());;
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
H5OpenedFile::ObjectType H5ObjectToVisualize::getType()
{
    return type;
}

/**
 * @brief Returns dataset
 * @return Dataset
 */
H5Helper::Dataset *H5ObjectToVisualize::getDataset()
{
    return dataset;
}

/**
 * @brief Loads data about dataset or group (size, position, ...)
 */
void H5ObjectToVisualize::loadObjectData()
{
    // Size of dataset (can be downsampled)
    size = H5Helper::Vector3D(dataset->getDims());
    originalSize = size;

    // Set 3D frame size
    frameSize = size;
    originalFrameSize = frameSize;

    // If downsampled
    if (dataset->hasAttribute(H5Helper::SRC_SIZE_X_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_SIZE_Y_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_SIZE_Z_ATTR)
            && dataset->hasAttribute(H5Helper::SRC_DATASET_NAME_ATTR)
            ) {
        originalSize.x(dataset->readAttributeI(H5Helper::SRC_SIZE_X_ATTR, false));
        originalSize.y(dataset->readAttributeI(H5Helper::SRC_SIZE_Y_ATTR, false));
        originalSize.z(dataset->readAttributeI(H5Helper::SRC_SIZE_Z_ATTR, false));
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

    H5Helper::convertlinearToMultiDim(minValueIndex, minValuePosition, dataset->getDims());
    H5Helper::convertlinearToMultiDim(maxValueIndex, maxValuePosition, dataset->getDims());


    if (type == H5OpenedFile::DATASET_3D) {
        // Default step
        steps = 1;
        if (dataset->hasAttribute(H5Helper::POSITION_X_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Y_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Z_ATTR)) {
            originalPosition.z(dataset->readAttributeI(H5Helper::POSITION_Z_ATTR, false));
            originalPosition.y(dataset->readAttributeI(H5Helper::POSITION_Y_ATTR, false));
            originalPosition.x(dataset->readAttributeI(H5Helper::POSITION_X_ATTR, false));
            position = originalPosition;

            frameSize = H5Helper::Vector3D(openedH5File->getNDims());;
            originalFrameSize = frameSize;
        }

    } else if (type == H5OpenedFile::DATASET_4D) {

        frameSize = H5Helper::Vector3D(openedH5File->getNDims());;
        originalFrameSize = frameSize;

        // Get position (was defined by sensor mask)
        if (dataset->hasAttribute(H5Helper::POSITION_X_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Y_ATTR) && dataset->hasAttribute(H5Helper::POSITION_Z_ATTR)) {
            originalPosition.z(dataset->readAttributeI(H5Helper::POSITION_Z_ATTR, false));
            originalPosition.y(dataset->readAttributeI(H5Helper::POSITION_Y_ATTR, false));
            originalPosition.x(dataset->readAttributeI(H5Helper::POSITION_X_ATTR, false));
            position = originalPosition;
        } else {
            // Try to get position from sensor_mask_corners dataset
            if (openedH5File->getFile()->objExistsByName(H5Helper::SENSOR_MASK_CORNERS_DATASET)) {
                H5Helper::Dataset *sensorMaskCornersDataset = openedH5File->getFile()->openDataset(H5Helper::SENSOR_MASK_CORNERS_DATASET);
                hsize_t *sensorMaskCornersData;
                sensorMaskCornersDataset->readDataset(sensorMaskCornersData);
                // Name of the dataset to index
                hsize_t i = hsize_t(std::stoi(dataset->getOnlyName()));
                dataset->findAndSetGlobalMinAndMaxValue();
                dataset->setAttribute(H5Helper::POSITION_Z_ATTR, sensorMaskCornersData[(i - 1) * 6 + 2] - 1);
                dataset->setAttribute(H5Helper::POSITION_Y_ATTR, sensorMaskCornersData[(i - 1) * 6 + 1] - 1);
                dataset->setAttribute(H5Helper::POSITION_X_ATTR, sensorMaskCornersData[(i - 1) * 6 + 0] - 1);
                originalPosition.z(sensorMaskCornersData[(i - 1) * 6 + 2] - 1);
                originalPosition.y(sensorMaskCornersData[(i - 1) * 6 + 1] - 1);
                originalPosition.x(sensorMaskCornersData[(i - 1) * 6 + 0] - 1);
                position = originalPosition;
                delete[] sensorMaskCornersData;
                openedH5File->getFile()->closeDataset(sensorMaskCornersDataset);
            }
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
        steps = H5Helper::Vector4D(dataset->getDims()).w();
    }
}

/**
 * @brief Send signals images were changed
 */
void H5ObjectToVisualize::changeImages()
{
    emit imageXYChanged(createImageXY());
    emit imageXZChanged(createImageXZ());
    emit imageYZChanged(createImageYZ());
}

/**
 * @brief Reloads images
 */
void H5ObjectToVisualize::reloadImages()
{
    setZIndex(index.z());
    setYIndex(index.y());
    setXIndex(index.x());
}

/**
 * @brief Slice XY loaded slot
 * @param[in] r Request
 */
void H5ObjectToVisualize::sliceXYLoaded(Request *r)
{
    XYloadedFlag = false;
    // Copy image data from request
    memcpy(dataXY, r->data, static_cast<size_t>(size.y() * size.x()) * sizeof(float));
    XYloadedFlag = true;
    if (index.z() == H5Helper::Vector3D(r->offset).z())
        currentXYLoaded = true;
    else
        currentXYLoaded = false;
    if (areCurrentSlicesLoaded())
        emit slicesLoaded();
    emit imageXYChanged(createImageXY());
    emit dataXYChanged(dataXY, H5Helper::Vector3D(r->offset).z());
    threadXY->deleteDoneRequest(r);
}

/**
 * @brief Slice XZ loaded slot
 * @param[in] r request
 */
void H5ObjectToVisualize::sliceXZLoaded(Request *r)
{
    XZloadedFlag = false;
    // Copy image data from request
    memcpy(dataXZ, r->data, static_cast<size_t>(size.z() * size.x()) * sizeof(float));
    XZloadedFlag = true;
    if (index.y() == H5Helper::Vector3D(r->offset).y())
        currentXZLoaded = true;
    else
        currentXZLoaded = false;
    if (areCurrentSlicesLoaded())
        emit slicesLoaded();
    emit imageXZChanged(createImageXZ());
    emit dataXZChanged(dataXZ, H5Helper::Vector3D(r->offset).y());
    threadXZ->deleteDoneRequest(r);
}

/**
 * @brief Slice YZ loaded slot
 * @param[in] r request
 */
void H5ObjectToVisualize::sliceYZLoaded(Request *r)
{
    YZloadedFlag = false;
    // Copy image data from request
    memcpy(dataYZ, r->data, static_cast<size_t>(size.z() * size.y()) * sizeof(float));
    YZloadedFlag = true;
    if (index.x() == H5Helper::Vector3D(r->offset).x())
        currentYZLoaded = true;
    else
        currentYZLoaded = false;
    if (areCurrentSlicesLoaded())
        emit slicesLoaded();
    emit imageYZChanged(createImageYZ());
    emit dataYZChanged(dataYZ, H5Helper::Vector3D(r->offset).x());
    threadYZ->deleteDoneRequest(r);
}

/**
 * @brief Is current XY Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXYLoaded()
{
    return currentXYLoaded;
}

/**
 * @brief Is current XZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXZLoaded()
{
    return currentXZLoaded;
}

/**
 * @brief Is current YZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentYZLoaded()
{
    return currentYZLoaded;
}

/**
 * @brief Are current slices Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::areCurrentSlicesLoaded()
{
    if (currentXYLoaded && currentXZLoaded && currentYZLoaded)
        return true;
    else
        return false;
}

/**
 * @brief Creates XY image from loaded data
 * @return XY image
 */
QImage H5ObjectToVisualize::createImageXY()
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
QImage H5ObjectToVisualize::createImageXZ()
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
QImage H5ObjectToVisualize::createImageYZ()
{
    QImage qimage;
    if (YZloadedFlag) {
        qimage = QImage(int(size.y()), int(size.z()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.y() * size.z()), minValue, maxValue, dataYZ, qimage.bits(), colormap);
    }
    return qimage;
}

QVector<float> H5ObjectToVisualize::getOpacity() const
{
    return opacity;
}

void H5ObjectToVisualize::setOpacity(const QVector<float> &value)
{
    opacity = value;
    emit(opacityChanged(opacity));
}

bool H5ObjectToVisualize::isSelected() const
{
    return selected;
}

void H5ObjectToVisualize::setSelected(bool value)
{
    selected = value;
}

void H5ObjectToVisualize::toggleSelected()
{
    selected = !selected;
}

/**
 * @brief Returns OpenedH5File
 * @return OpenedH5File
 */
H5OpenedFile *H5ObjectToVisualize::getOpenedH5File() const
{
    return openedH5File;
}

/**
 * @brief Returns data XY
 * @return Float XY data
 */
float *H5ObjectToVisualize::getDataXY()
{
    return dataXY;
}

/**
 * @brief Returns data XZ
 * @return Float XZ data
 */
float *H5ObjectToVisualize::getDataXZ()
{
    return dataXZ;
}

/**
 * @brief Returns data YZ
 * @return Float YZ data
 */
float *H5ObjectToVisualize::getDataYZ()
{
    return dataYZ;
}

QImage H5ObjectToVisualize::getImageXY()
{
    return createImageXY();
}

QImage H5ObjectToVisualize::getImageXZ()
{
    return createImageXZ();
}

QImage H5ObjectToVisualize::getImageYZ()
{
    return createImageYZ();
}

/**
 * @brief Returns current X index
 * @return Current X index
 */
hsize_t H5ObjectToVisualize::getXIndex()
{
    return index.x();
}

/**
 * @brief Returns current Y index
 * @return Current Y index
 */
hsize_t H5ObjectToVisualize::getYIndex()
{
    return index.y();
}

/**
 * @brief Returns current Z index
 * @return Current Z index
 */
hsize_t H5ObjectToVisualize::getZIndex()
{
    return index.z();
}

/**
 * @brief Starts loading image data with new X index
 * @param[in] value X index
 */
void H5ObjectToVisualize::setXIndex(hsize_t value)
{
    index.x(value);
    currentYZLoaded = false;
    if (type == H5OpenedFile::DATASET_3D)
        threadYZ->createRequest(dataset, H5Helper::Vector3D(0, 0, value), H5Helper::Vector3D(size.z(), size.y(), 1));
    else
        threadYZ->createRequest(dataset, H5Helper::Vector4D(currentStep, 0, 0, value), H5Helper::Vector4D(1, size.z(), size.y(), 1));

    threadYZ->start();
}

/**
 * @brief Starts loading image data with new Y index
 * @param[in] value Y index
 */
void H5ObjectToVisualize::setYIndex(hsize_t value)
{
    index.y(value);
    currentXZLoaded = false;
    if (type == H5OpenedFile::DATASET_3D)
        threadXZ->createRequest(dataset, H5Helper::Vector3D(0, value, 0), H5Helper::Vector3D(size.z(), 1, size.x()));
    else
        threadXZ->createRequest(dataset, H5Helper::Vector4D(currentStep, 0, value, 0), H5Helper::Vector4D(1, size.z(), 1, size.x()));
    threadXZ->start();
}

/**
 * @brief Starts loading image data with new Z index
 * @param[in] value Z index
 */
void H5ObjectToVisualize::setZIndex(hsize_t value)
{
    index.z(value);
    currentXYLoaded = false;
    if (type == H5OpenedFile::DATASET_3D)
        threadXY->createRequest(dataset, H5Helper::Vector3D(value, 0, 0), H5Helper::Vector3D(1, size.y(), size.x()));
    else
        threadXY->createRequest(dataset, H5Helper::Vector4D(currentStep, value, 0, 0), H5Helper::Vector4D(1, 1, size.y(), size.x()));
    threadXY->start();
}

/**
 * @brief Gets count (unused)
 * @return Count
 */
int H5ObjectToVisualize::getSlicesCount()
{
    return slicesCount;
}

/**
 * @brief Sets count (unused)
 * @param[in] value Count
 */
void H5ObjectToVisualize::setSlicesCount(int value)
{
    slicesCount = value;
}

/**
 * @brief Returns size
 * @return Size
 */
H5Helper::Vector3D H5ObjectToVisualize::getSize()
{
    return size;
}

/**
 * @brief Returns original size
 * @return Original size
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalSize()
{
    return originalSize;
}

/**
 * @brief Returns frame size
 * @return Frame size
 */
H5Helper::Vector3D H5ObjectToVisualize::getFrameSize()
{
    return frameSize;
}

/**
 * @brief Returns original frame size
 * @return Original frame size
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalFrameSize()
{
    return originalFrameSize;
}

/**
 * @brief Returns Position
 * @return Position
 */
H5Helper::Vector3D H5ObjectToVisualize::getPos()
{
    return position;
}

/**
 * @brief Returns original position
 * @return Original position
 */
H5Helper::Vector3D H5ObjectToVisualize::getOriginalPos()
{
    return originalPosition;
}

/**
 * @brief Returns steps
 * @return Steps
 */
hsize_t H5ObjectToVisualize::getSteps()
{
    return steps;
}

/**
 * @brief Returns current step
 * @return Current step
 */
hsize_t H5ObjectToVisualize::getCurrentStep()
{
    return currentStep;
}

/**
 * @brief Synchronizes loading time series
 * @param[in] step Step
 */
void H5ObjectToVisualize::setCurrentStep(hsize_t step)
{
    if (type == H5OpenedFile::DATASET_4D) {
        try {
            currentStep = step;
            reloadImages();
            emit stepChanged();
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

/**
 * @brief Returns colormap
 * @return Colormap
 */
ColorMap::Type H5ObjectToVisualize::getColormap()
{
    return colormap;
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

void H5ObjectToVisualize::setColormap(int colormap)
{
    setColormap(static_cast<ColorMap::Type>(colormap));
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
 * @brief Returns minimal value
 * @return Minimal global current value
 */
float H5ObjectToVisualize::getMinValue()
{
    return minValue;
}

/**
 * @brief Returns maximal value
 * @return Maximal global current value
 */
float H5ObjectToVisualize::getMaxValue()
{
    return maxValue;
}

/**
 * @brief Returns original minimal value
 * @return Minimal global original value
 */
float H5ObjectToVisualize::getOriginalMinValue()
{
    return originalMinValue;
}

/**
 * @brief Returns original maximal value
 * @return Maximal global original value
 */
float H5ObjectToVisualize::getOriginalMaxValue()
{
    return originalMaxValue;
}

/**
 * @brief Returns value at 2D position on XY image
 * @param[in] x X-coord
 * @param[in] y Y-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromXY(int x, int y)
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
float H5ObjectToVisualize::getValueAtPointFromXZ(int x, int z)
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
float H5ObjectToVisualize::getValueAtPointFromYZ(int y, int z)
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
QList<QPair<QString, QString>> H5ObjectToVisualize::getInfo()
{
    QList<QPair<QString, QString>> info;
    if (type == H5OpenedFile::DATASET_3D) {
        info.append(QPair<QString, QString>("Name", getName()));
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
    } else if (type == H5OpenedFile::DATASET_4D) {
        info.append(QPair<QString, QString>("Name", getName()));
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
