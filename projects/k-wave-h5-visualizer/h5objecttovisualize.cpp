/**
 * @file        h5subobjecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              23 October   2018 (updated)
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
 * @param[in] openedH5File Opened file
 * @param[in] h5ObjectToVisualize Object to visualize
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
 * @brief Inicialization of important variables
 */
void H5ObjectToVisualize::initialize()
{
    // Allocation memory for slices
    dataXY = new float[size.y() * size.x()];
    dataXZ = new float[size.z() * size.x()];
    dataYZ = new float[size.z() * size.y()];

    data3D = new float[size.getSize()];

    index.x(H5Helper::Vector3D(maxValuePosition).x());
    index.y(H5Helper::Vector3D(maxValuePosition).y());
    index.z(H5Helper::Vector3D(maxValuePosition).z());
    index.t(H5Helper::Vector4D(maxValuePosition).t());
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
    if (dataXZ) {
        delete[] dataXZ;
        dataXZ = nullptr;
    }
    if (dataYZ) {
        delete[] dataYZ;
        dataYZ = nullptr;
    }
    if (data3D) {
        delete[] data3D;
        data3D = nullptr;
    }

    if (compressHelper) {
        delete compressHelper;
        compressHelper = nullptr;
    }
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
void H5ObjectToVisualize::reloadSlices()
{
    reloadXY();
    reloadXZ();
    reloadYZ();
}

void H5ObjectToVisualize::reloadXY()
{
    setZIndex(int(index.z()));
}

void H5ObjectToVisualize::reloadXZ()
{
    setYIndex(int(index.y()));
}

void H5ObjectToVisualize::reloadYZ()
{
    setXIndex(int(index.x()));
}

/**
 * @brief Slice XY loaded slot
 * @param[in] r Request
 */
void H5ObjectToVisualize::sliceXYLoaded(Request *request)
{
    if (index.z() == H5Helper::Vector3D(request->offset).z()) {
        currentXYLoadedFlag = true;
        emit currentXYLoaded();
    } else {
        currentXYLoadedFlag = false;
    }
    emit imageXYChanged(createImageXY());
    emit dataXYChanged(dataXY, H5Helper::Vector3D(request->offset).z());
    checkCurrentDataIsLoaded();
    threadXY->deleteDoneRequest(request);
}

/**
 * @brief Slice XZ loaded slot
 * @param[in] r request
 */
void H5ObjectToVisualize::sliceXZLoaded(Request *request)
{
    if (index.y() == H5Helper::Vector3D(request->offset).y()) {
        currentXZLoadedFlag = true;
        emit currentXZLoaded();
    } else {
        currentXZLoadedFlag = false;
    }
    emit imageXZChanged(createImageXZ());
    emit dataXZChanged(dataXZ, H5Helper::Vector3D(request->offset).y());
    checkCurrentDataIsLoaded();
    threadXZ->deleteDoneRequest(request);
}

/**
 * @brief Slice YZ loaded slot
 * @param[in] r request
 */
void H5ObjectToVisualize::sliceYZLoaded(Request *request)
{
    if (index.x() == H5Helper::Vector3D(request->offset).x()) {
        currentYZLoadedFlag = true;
        emit currentYZLoaded();
    } else {
        currentYZLoadedFlag = false;
    }
    emit imageYZChanged(createImageYZ());
    emit dataYZChanged(dataYZ, H5Helper::Vector3D(request->offset).x());
    checkCurrentDataIsLoaded();
    threadYZ->deleteDoneRequest(request);
}

void H5ObjectToVisualize::data3DLoaded(Request *request)
{
    if (index.t() == H5Helper::Vector4D(request->offset).t()) {
        currentData3DLoadedFlag = true;
        emit currentData3DLoaded();
    } else {
        currentData3DLoadedFlag = false;
    }
    if (compressHelper) {
        emit data3DCompressChanged(thread3D->getDataLC(), thread3D->getDataCC(), thread3D->getLocalStep());
    } else {
        emit data3DChanged(data3D);
    }
    checkCurrentDataIsLoaded();
    thread3D->deleteDoneRequest(request);
}

void H5ObjectToVisualize::checkCurrentDataIsLoaded()
{
    if (loadData3DFlag) {
        if (areCurrentData3DLoaded() && areCurrentSlicesLoaded()) {
            emit currentSlicesLoaded();
            emit currentStepLoaded();
        }
    } else if (areCurrentSlicesLoaded()) {
        emit currentSlicesLoaded();
        emit currentStepLoaded();
    }
}

/**
 * @brief Is current XY Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXYLoaded()
{
    return currentXYLoadedFlag;
}

/**
 * @brief Is current XZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentXZLoaded()
{
    return currentXZLoadedFlag;
}

/**
 * @brief Is current YZ Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::isCurrentYZLoaded()
{
    return currentYZLoadedFlag;
}

/**
 * @brief Are current slices Loaded?
 * @return True/False
 */
bool H5ObjectToVisualize::areCurrentSlicesLoaded()
{
    if (currentXYLoadedFlag && currentXZLoadedFlag && currentYZLoadedFlag)
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
    //if (XYloadedFlag) {
        qimage = QImage(int(size.x()), int(size.y()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.x() * size.y()), minValue, maxValue, dataXY, qimage.bits(), colormap);
    //}
    return qimage;
}

/**
 * @brief Creates XZ image from loaded data
 * @return XZ image
 */
QImage H5ObjectToVisualize::createImageXZ()
{
    QImage qimage;
    //if (XZloadedFlag) {
        qimage = QImage(int(size.x()), int(size.z()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.x() * size.z()), minValue, maxValue, dataXZ, qimage.bits(), colormap);
    //}
    return qimage;
}

/**
 * @brief Creates YZ image from loaded data
 * @return YZ image
 */
QImage H5ObjectToVisualize::createImageYZ()
{
    QImage qimage;
    //if (YZloadedFlag) {
        qimage = QImage(int(size.y()), int(size.z()), QImage::Format_RGB32);
        ColorMap::applyColorMap(int(size.y() * size.z()), minValue, maxValue, dataYZ, qimage.bits(), colormap);
    //}
    return qimage;
}

void H5ObjectToVisualize::load3Ddata()
{
    thread3D->createRequest(dataset, index.t(), data3D);
    emit data3DLoadingStarted();
    thread3D->start();
}

bool H5ObjectToVisualize::getMinMaxValuesTrim() const
{
    return minMaxValuesTrimFlag;
}

bool H5ObjectToVisualize::areCurrentData3DLoaded() const
{
    return currentData3DLoadedFlag;
}

float *H5ObjectToVisualize::getData3D() const
{
    return data3D;
}

float *H5ObjectToVisualize::getData3DLC() const
{
    return thread3D->getDataLC();
}

float *H5ObjectToVisualize::getData3DCC() const
{
    return thread3D->getDataCC();
}

hsize_t H5ObjectToVisualize::getLocalStep() const
{
    return thread3D->getLocalStep();
}

bool H5ObjectToVisualize::getData3DLoadingFlag() const
{
    return loadData3DFlag;
}

void H5ObjectToVisualize::setData3DLoadingFlag(bool value)
{
  loadData3DFlag = value;
  if (loadData3DFlag && !currentData3DLoadedFlag)
      load3Ddata();
}

void H5ObjectToVisualize::setHoveredPointInImageXY(int x, int y)
{
    emit(hoveredPointInImage(getValueAtPointFromXY(x, y)));
}

void H5ObjectToVisualize::setHoveredPointInImageXZ(int x, int z)
{
    emit(hoveredPointInImage(getValueAtPointFromXZ(x, z)));
}

void H5ObjectToVisualize::setHoveredPointInImageYZ(int y, int z)
{
    emit(hoveredPointInImage(getValueAtPointFromYZ(y, z)));
}

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

    disconnect(this, SIGNAL(data3DChanged(float *)), nullptr, nullptr);
    disconnect(this, SIGNAL(data3DCompressChanged(float *, float *, hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXYChanged(float *, hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataXZChanged(float *, hsize_t)), nullptr, nullptr);
    disconnect(this, SIGNAL(dataYZChanged(float *, hsize_t)), nullptr, nullptr);

    disconnect(this, SIGNAL(imageXYChanged(QImage)), nullptr, nullptr);
    disconnect(this, SIGNAL(imageXZChanged(QImage)), nullptr, nullptr);
    disconnect(this, SIGNAL(imageYZChanged(QImage)), nullptr, nullptr);

    disconnect(this, SIGNAL(hoveredPointInImage(float)), nullptr, nullptr);
}

H5Helper::CompressHelper *H5ObjectToVisualize::getCompressHelper() const
{
  return compressHelper;
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
    return selectedFlag;
}

void H5ObjectToVisualize::setSelected(bool value)
{
    selectedFlag = value;
}

void H5ObjectToVisualize::toggleSelected()
{
    setSelected(!selectedFlag);
}

/**
 * @brief Returns OpenedH5File
 * @return OpenedH5File
 */
H5Helper::File *H5ObjectToVisualize::getFile() const
{
    return dataset->getFile();
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
void H5ObjectToVisualize::setXIndex(int value)
{
    index.x(value);
    emit xIndexChanged(value);
    currentYZLoadedFlag = false;
    if (type == H5OpenedFile::DATASET_3D) {
        threadYZ->createRequest(dataset, H5Helper::Vector3D(0, 0, hsize_t(value)), H5Helper::Vector3D(size.z(), size.y(), 1), dataYZ);
    } else {
        threadYZ->createRequest(dataset, H5Helper::Vector4D(index.t(), 0, 0, hsize_t(value)), H5Helper::Vector4D(1, size.z(), size.y(), 1), dataYZ);
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
        threadXZ->createRequest(dataset, H5Helper::Vector3D(0, hsize_t(value), 0), H5Helper::Vector3D(size.z(), 1, size.x()), dataXZ);
    else
        threadXZ->createRequest(dataset, H5Helper::Vector4D(index.t(), 0, hsize_t(value), 0), H5Helper::Vector4D(1, size.z(), 1, size.x()), dataXZ);
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
        threadXY->createRequest(dataset, H5Helper::Vector3D(hsize_t(value), 0, 0), H5Helper::Vector3D(1, size.y(), size.x()), dataXY);
    else
        threadXY->createRequest(dataset, H5Helper::Vector4D(index.t(), hsize_t(value), 0, 0), H5Helper::Vector4D(1, 1, size.y(), size.x()), dataXY);
    emit dataXYLoadingStarted();
    threadXY->start();
}

void H5ObjectToVisualize::setToMaxValuePosition()
{
    setXIndex(int(H5Helper::Vector3D(maxValuePosition).x()));
    setYIndex(int(H5Helper::Vector3D(maxValuePosition).y()));
    setZIndex(int(H5Helper::Vector3D(maxValuePosition).z()));
    setCurrentStep(int(H5Helper::Vector4D(maxValuePosition).t()));
}

void H5ObjectToVisualize::setToMinValuePosition()
{
    setXIndex(int(H5Helper::Vector3D(minValuePosition).x()));
    setYIndex(int(H5Helper::Vector3D(minValuePosition).y()));
    setZIndex(int(H5Helper::Vector3D(minValuePosition).z()));
    setCurrentStep(int(H5Helper::Vector4D(minValuePosition).t()));
}

void H5ObjectToVisualize::setMinMaxValuesTrim(bool value)
{
    minMaxValuesTrimFlag = value;
    emit minMaxValuesTrimChanged(value);
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
    return index.t();
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

/**
 * @brief Sets colormap
 * @param[in] colormap Colormap
 */
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
    return dataXY[hsize_t(x) + size.x() * hsize_t(y)];
}

/**
 * @brief Returns value at 2D position on XZ image
 * @param[in] x X(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromXZ(int x, int z)
{
    return dataXZ[hsize_t(x) + size.x() * hsize_t(z)];
}

/**
 * @brief Returns value at 2D position on YZ image
 * @param[in] y Y(X)-coord
 * @param[in] z Z(Y)-coord
 * @return Float value
 */
float H5ObjectToVisualize::getValueAtPointFromYZ(int y, int z)
{
    return dataYZ[hsize_t(y) + size.y() * hsize_t(z)];
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
