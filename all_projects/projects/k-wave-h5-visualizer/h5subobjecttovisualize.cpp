/*
 * @file        h5subobjecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing the H5SubobjectToVisualize class.
 *              Object of class H5ObjectToVisualize is for one dataset or group of datasets
 *              with time series and has own threads and view settings.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */


#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize
 * @param dataset
 * @param openedH5File
 * @param parent
 */
OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5File::HDF5Dataset *dataset, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = QString::fromStdString(dataset->getName());
    this->group = NULL;
    this->dataset = dataset;
    this->type = OpenedH5File::DATASET_TYPE;
    initialize();
    loadObjectData();
    // Allocation memory for slices
    dataXY = new float[size[1] * size[2]];
    dataXZ = new float[size[0] * size[2]];
    dataYZ = new float[size[0] * size[1]];
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize
 * @param group
 * @param openedH5File
 * @param parent
 */
OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5File::HDF5Group *group, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = QString::fromStdString(group->getName());
    this->group = group;
    // Set first dataset of time series
    this->dataset = openedH5File->file->openDataset(name.toStdString() + "/" + std::to_string(0));
    this->type = OpenedH5File::GROUP_TYPE;
    initialize();
    loadObjectData();
    dataXY = new float[size[1] * size[2]];
    dataXZ = new float[size[0] * size[2]];
    dataYZ = new float[size[0] * size[1]];
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::initialize Inicialization of important variables
 */
void OpenedH5File::H5SubobjectToVisualize::initialize()
{
    // Initialize
    xIndex = 0;
    yIndex = 0;
    zIndex = 0;

    lastLoadedXIndex = 0;
    lastLoadedYIndex = 0;
    lastLoadedZIndex = 0;

    XYloadedFlag = false;
    XZloadedFlag = false;
    YZloadedFlag = false;

    minVG = 0;
    maxVG = 0;
    minVXY = 0;
    maxVXY = 0;
    minVXZ = 0;
    maxVXZ = 0;
    minVYZ = 0;
    maxVYZ = 0;
    originalMinVG = 0;
    originalMaxVG = 0;
    originalMinVXY = 0;
    originalMaxVXY = 0;
    originalMinVXZ = 0;
    originalMaxVXZ = 0;
    originalMinVYZ = 0;
    originalMaxVYZ = 0;

    colormap = 2;

    useGlobal = true;

    dataXY = NULL;
    dataXZ = NULL;
    dataYZ = NULL;

    alpha = 0.5;
    red = 0.5;
    green = 0.5;
    blue = 0.5;

    count = 50;

    originalFrameSize[0] = 0;
    originalFrameSize[1] = 0;
    originalFrameSize[2] = 0;
    frameSize[0] = 0;
    frameSize[1] = 0;
    frameSize[2] = 0;
    originalSize[0] = 0;
    originalSize[1] = 0;
    originalSize[2] = 0;
    size[0] = 0;
    size[1] = 0;
    size[2] = 0;
    chunkSize[0] = 0;
    chunkSize[1] = 0;
    chunkSize[2] = 0;
    originalPos[0] = 0;
    originalPos[1] = 0;
    originalPos[2] = 0;
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    steps = 1;
    dwnsmpl = 0;
    currentStep = 0;

    GUIInitialized = false;
    GUIXYInitialized = false;
    GUIXZInitialized = false;
    GUIYZInitialized = false;

    currentXYLodaded = false;
    currentXZLodaded = false;
    currentYZLodaded = false;

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

    delete [] dataXY;
    delete [] dataXZ;
    delete [] dataYZ;
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
 * @brief OpenedH5File::H5SubobjectToVisualize::setGUIXYInitialized
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setGUIXYInitialized(bool value)
{
    GUIXYInitialized = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isGUIXYInitialized
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isGUIXYInitialized()
{
    return GUIXYInitialized;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setGUIXZInitialized
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setGUIXZInitialized(bool value)
{
    GUIXZInitialized = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isGUIXZInitialized
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isGUIXZInitialized()
{
    return GUIXZInitialized;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setGUIYZInitialized
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setGUIYZInitialized(bool value)
{
    GUIYZInitialized = value;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::isGUIYZInitialized
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::isGUIYZInitialized()
{
    return GUIYZInitialized;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getName
 * @return subobject name
 */
QString OpenedH5File::H5SubobjectToVisualize::getName()
{
    return name;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getType
 * @return subobject type
 */
int OpenedH5File::H5SubobjectToVisualize::getType()
{
    return type;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getDataset
 * @return dataset
 */
HDF5File::HDF5Dataset *OpenedH5File::H5SubobjectToVisualize::getDataset()
{
    return dataset;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getGroup
 * @return group (can be NULL)
 */
HDF5File::HDF5Group *OpenedH5File::H5SubobjectToVisualize::getGroup()
{
    return group;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::loadObjectData Load data about dataset or group (size, position, ...)
 */
void OpenedH5File::H5SubobjectToVisualize::loadObjectData()
{
    if (type == OpenedH5File::DATASET_TYPE &&  dataset != NULL) {
        // Check resolution
        if (dataset->hasAttribute("dwnsmpl")) {
            dwnsmpl =  dataset->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
            std::cout << "No downsampling" << std::endl;
        }
        HDF5File::HDF5Vector3D sizeD =  dataset->getDims();

        // Original size withnout downsampling
        originalSize[0] = openedH5File->getNZ();
        originalSize[1] = openedH5File->getNY();
        originalSize[2] = openedH5File->getNX();
        // Size of dataset (can be downsampled)
        size[0] = sizeD[0];
        size[1] = sizeD[1];
        size[2] = sizeD[2];

        // Set 3D frame size
        originalFrameSize[0] = originalSize[0];
        originalFrameSize[1] = originalSize[1];
        originalFrameSize[2] = originalSize[2];
        frameSize[0] = size[0];
        frameSize[1] = size[1];
        frameSize[2] = size[2];

        // Position is 0
        originalPos[0] = 0;
        originalPos[1] = 0;
        originalPos[2] = 0;
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;

        // Get chunk dimensions
        sizeD =  dataset->getChunkDims();
        chunkSize[0] = sizeD[0];
        chunkSize[1] = sizeD[1];
        chunkSize[2] = sizeD[2];

        // Get global mi/max/ values
        dataset->findAndSetGlobalMinAndMaxValue();
        minVG =  dataset->getGlobalMinValueF();
        maxVG =  dataset->getGlobalMaxValueF();
        originalMinVG =  dataset->getGlobalMinValueF();
        originalMaxVG =  dataset->getGlobalMaxValueF();

        // Default step
        steps = 1;
    } else if (type == OpenedH5File::GROUP_TYPE && group != NULL && dataset != NULL) {
        // Check resolution
        if (group->hasAttribute("dwnsmpl")) {
            dwnsmpl =  group->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
            std::cout << "No downsampling" << std::endl;
        }

        // Size of dataset (can be downsampled)
        HDF5File::HDF5Vector3D sizeD =  dataset->getDims();
        originalSize[0] = sizeD[0];
        originalSize[1] = sizeD[1];
        originalSize[2] = sizeD[2];
        size[0] = sizeD[0];
        size[1] = sizeD[1];
        size[2] = sizeD[2];

        // Get position (was defined by sensor mask)
        originalPos[0] =  group->readAttributeI("positionZ");
        originalPos[1] =  group->readAttributeI("positionY");
        originalPos[2] =  group->readAttributeI("positionX");
        pos[0] =  group->readAttributeI("positionZ");
        pos[1] =  group->readAttributeI("positionY");
        pos[2] =  group->readAttributeI("positionX");

        // Set frame size
        originalFrameSize[0] = openedH5File->getNZ();
        originalFrameSize[1] = openedH5File->getNY();
        originalFrameSize[2] = openedH5File->getNX();
        frameSize[0] = openedH5File->getNZ();
        frameSize[1] = openedH5File->getNY();
        frameSize[2] = openedH5File->getNX();

        // If downsamples -> compute oroginal size and frame size
        if (dwnsmpl > 0) {
            // TODO better way to get original values...
            uint64_t maxN = qMax(openedH5File->getNX(), qMax(openedH5File->getNY(), openedH5File->getNZ()));
            double ratio = (double) maxN / dwnsmpl;

            frameSize[0] = qRound(frameSize[0] / ratio);
            frameSize[1] = qRound(frameSize[1] / ratio);
            frameSize[2] = qRound(frameSize[2] / ratio);

            originalSize[0] = qRound(size[0] * ratio);
            originalSize[1] = qRound(size[1] * ratio);
            originalSize[2] = qRound(size[2] * ratio);

            // This is not very clean...
            if (size[0] == 1) originalSize[0] = 1;
            if (size[1] == 1) originalSize[1] = 1;
            if (size[2] == 1) originalSize[2] = 1;
            originalPos[0] = qRound((pos[0] + 1) * ratio - 1);
            originalPos[1] = qRound((pos[1] + 1) * ratio - 1);
            originalPos[2] = qRound((pos[2] + 1) * ratio - 1);
            // This is not very clean...
            if (pos[0] == 0) originalPos[0] = 0;
            if (pos[1] == 0) originalPos[1] = 0;
            if (pos[2] == 0) originalPos[2] = 0;
        }

        // Get chunk dims
        sizeD =  dataset->getChunkDims();
        chunkSize[0] = sizeD[0];
        chunkSize[1] = sizeD[1];
        chunkSize[2] = sizeD[2];

        // Get min/max values
        minVG =  group->readAttributeF("min");
        maxVG =  group->readAttributeF("max");
        originalMinVG =  group->readAttributeF("min");
        originalMaxVG =  group->readAttributeF("max");

        // Get number of steps
        steps =  group->getNumObjs();//group->readAttributeI("count");
    }
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::changeImages Image data was changed -> send signals
 */
void OpenedH5File::H5SubobjectToVisualize::changeImages()
{
    emit imageXYChanged(createImageXY(), zIndex);
    emit imageXZChanged(createImageXZ(), yIndex);
    emit imageYZChanged(createImageYZ(), xIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::reloadImages
 */
void OpenedH5File::H5SubobjectToVisualize::reloadImages()
{
    setZIndex(zIndex);
    setYIndex(yIndex);
    setXIndex(xIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded(Request *r)
{
    //QMutexLocker lock(&mutexXY);
    XYloadedFlag = false;
    //delete [] dataXY;
    //dataXY = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataXY = new float[size];
    // Copy image data form request
    memcpy(dataXY, r->data, size[1] * size[2] * sizeof(float));

    // Set local min/max values
    minVXY = r->min;
    originalMinVXY = minVXY;
    maxVXY = r->max;
    originalMaxVXY = maxVXY;

    XYloadedFlag = true;
    //lock.unlock();
    if (zIndex == r->offset.z())
        currentXYLodaded = true;
    else
        currentXYLodaded = false;
    emit imageXYChanged(createImageXY(), r->offset.z());
    threadXY->deleteDoneRequest(r);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded(Request *r)
{
    //QMutexLocker lock(&mutexXZ);
    XZloadedFlag = false;
    //delete [] dataXZ;
    //dataXZ = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataXZ = new float[size];
    // Copy image data form request
    memcpy(dataXZ, r->data, size[0] * size[2] * sizeof(float));

    // Set local min/max values
    minVXZ = r->min;
    originalMinVXZ = minVXZ;
    maxVXZ = r->max;
    originalMaxVXZ = maxVXZ;

    XZloadedFlag = true;
    //lock.unlock();
    if (yIndex == r->offset.y())
        currentXZLodaded = true;
    else
        currentXZLodaded = false;
    emit imageXZChanged(createImageXZ(), r->offset.y());
    threadXZ->deleteDoneRequest(r);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded
 * @param r request
 */
void OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded(Request *r)
{
    //QMutexLocker lock(&mutexYZ);
    YZloadedFlag = false;
    //delete [] dataYZ;
    //dataYZ = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataYZ = new float[size];
    // Copy image data form request
    memcpy(dataYZ, r->data, size[0] * size[1] * sizeof(float));

    // Set local min/max values
    minVYZ = r->min;
    originalMinVYZ = minVYZ;
    maxVYZ = r->max;
    originalMaxVYZ = maxVYZ;

    YZloadedFlag = true;
    //lock.unlock();
    if (xIndex == r->offset.x())
        currentYZLodaded = true;
    else
        currentYZLodaded = false;
    emit imageYZChanged(createImageYZ(), r->offset.x());
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
cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageXY()
{
    //QMutexLocker lock(&mutexXY);
    cv::Mat image;
    if (XYloadedFlag) {
        image = cv::Mat(size[1], size[2], CV_32FC1, dataXY); // rows, cols (height, width)
        // Map values to 0..255
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXY - minVXY), - minVXY * 255.0 / (maxVXY - minVXY));
        cv::applyColorMap(image, image, colormap);
        // Correct orientation
        cv::flip(image, image, 0);
    }
    return image;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::createImageXZ Create cv::Mat XZ image from loaded data
 * @return XZ image
 */
cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageXZ()
{
    //QMutexLocker lock(&mutexXZ);
    cv::Mat image;
    if (XZloadedFlag) {
        image = cv::Mat(size[0], size[2], CV_32FC1, dataXZ); // rows, cols (height, width)
        // Map values to 0..255
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXZ - minVXZ), - minVXZ * 255.0 / (maxVXZ - minVXZ));
        cv::applyColorMap(image, image, colormap);
    }
    return image;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::createImageYZ Create cv::Mat YZ image from loaded data
 * @return YZ image
 */
cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageYZ()
{
    //QMutexLocker lock(&mutexYZ);
    cv::Mat image;
    if (YZloadedFlag) {
        image = cv::Mat(size[0], size[1], CV_32FC1, dataYZ); // rows, cols (height, width)
        // Map values to 0..255
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVYZ - minVYZ), - minVYZ * 255.0 / (maxVYZ - minVYZ));
        cv::applyColorMap(image, image, colormap);
        // Correct orientation for view
        cv::transpose(image, image);
        cv::flip(image, image, 0);
        cv::flip(image, image, 1);
    }
    return image;
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
    return xIndex;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getYIndex
 * @return current Y index
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getYIndex()
{
    return yIndex;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getZIndex
 * @return current Z index
 */
uint64_t OpenedH5File::H5SubobjectToVisualize::getZIndex()
{
    return zIndex;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setXIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setXIndex(uint64_t value)
{
    xIndex = value;
    currentXYLodaded = false;
    threadYZ->createRequest(dataset, 0, 0, xIndex, size[0], size[1], 1);
    threadYZ->start();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setYIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setYIndex(uint64_t value)
{
    yIndex = value;
    currentXZLodaded = false;
    threadXZ->createRequest(dataset, 0, yIndex, 0, size[0], 1, size[2]);
    threadXZ->start();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setZIndex Start loading image data with new index
 * @param value index
 */
void OpenedH5File::H5SubobjectToVisualize::setZIndex(uint64_t value)
{
    zIndex = value;
    currentYZLodaded = false;
    threadXY->createRequest(dataset, zIndex, 0, 0, 1, size[1], size[2]);
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
uint64_t *OpenedH5File::H5SubobjectToVisualize::getSize()
{
    return size;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalSize
 * @return original size
 */
uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalSize()
{
    return originalSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getFrameSize
 * @return frame size
 */
uint64_t *OpenedH5File::H5SubobjectToVisualize::getFrameSize()
{
    return frameSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize
 * @return original frame size
 */
uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize()
{
    return originalFrameSize;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getPos
 * @return position
 */
uint64_t *OpenedH5File::H5SubobjectToVisualize::getPos()
{
    return pos;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalPos
 * @return original position
 */
uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalPos()
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
void OpenedH5File::H5SubobjectToVisualize::setCurrentStep(uint64_t value, HDF5ReadingThread *thread3D)
{
    if (type == OpenedH5File::GROUP_TYPE) {
        try {
            thread3D->clearRequests();
            thread3D->wait();

            threadXY->clearRequests();
            threadXY->wait();

            threadXZ->clearRequests();
            threadXZ->wait();

            threadYZ->clearRequests();
            threadYZ->wait();

            openedH5File->file->closeDataset(this->dataset->getName());
            // Open new dataset
            this->dataset = openedH5File->file->openDataset(name.toStdString() + "/" + std::to_string(value));
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
int OpenedH5File::H5SubobjectToVisualize::getColormap()
{
    return colormap;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setColormap
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setColormap(int value)
{
    colormap = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getUseGlobal
 * @return true/false
 */
bool OpenedH5File::H5SubobjectToVisualize::getUseGlobal()
{
    return useGlobal;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setUseGlobal
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setUseGlobal(bool value)
{
    useGlobal = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMinVG
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinVG(float value)
{
    minVG = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMaxVG
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxVG(float value)
{
    maxVG = value;
    changeImages();
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMinVXY
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinVXY(float value)
{
    minVXY = value;
    emit imageXYChanged(createImageXY(), zIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMaxVXY
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxVXY(float value)
{
    maxVXY = value;
    emit imageXYChanged(createImageXY(), zIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMinVXZ
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinVXZ(float value)
{
    minVXZ = value;
    emit imageXZChanged(createImageXZ(), yIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMaxVXZ
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxVXZ(float value)
{
    maxVXZ = value;
    emit imageXZChanged(createImageXZ(), yIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMinVYZ
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMinVYZ(float value)
{
    minVYZ = value;
    emit imageYZChanged(createImageYZ(), xIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::setMaxVYZ
 * @param value
 */
void OpenedH5File::H5SubobjectToVisualize::setMaxVYZ(float value)
{
    maxVYZ = value;
    emit imageYZChanged(createImageYZ(), xIndex);
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMinVG
 * @return min global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinVG()
{
    return minVG;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMaxVG
 * @return max global current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxVG()
{
    return maxVG;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMinVG
 * @return min global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVG()
{
    return originalMinVG;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVG
 * @return max global original value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVG()
{
    return originalMaxVG;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMinVXY
 * @return min local XY current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinVXY()
{
    return minVXY;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMaxVXY
 * @return max local XY current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxVXY()
{
    return maxVXY;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXY
 * @return min original local XY current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXY()
{
    return originalMinVXY;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXY
 * @return max original local XY current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXY()
{
    return originalMaxVXY;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMinVXZ
 * @return min local XZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinVXZ()
{
    return minVXZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMaxVXZ
 * @return max local XZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxVXZ()
{
    return maxVXZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXZ
 * @return min original local XZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXZ()
{
    return originalMinVXZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXZ
 * @return max original local XZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXZ()
{
    return originalMaxVXZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMinVYZ
 * @return min local YZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMinVYZ()
{
    return minVYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getMaxVYZ
 * @return max local YZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getMaxVYZ()
{
    return maxVYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMinVYZ
 * @return min original local YZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVYZ()
{
    return originalMinVYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVYZ
 * @return max original local YZ current value
 */
float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVYZ()
{
    return originalMaxVYZ;
}

/**
 * @brief OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY Get value at 2D position on XY image
 * @param x x-coord
 * @param y y-coord
 * @return float value
 */
float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY(int x, int y)
{
    //QMutexLocker lock(&mutexXY);
    if (XYloadedFlag)
        return dataXY[x + size[2] * (size[1] - 1 - y)];
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
    //QMutexLocker lock(&mutexXZ);
    if (XZloadedFlag)
        return dataXZ[x + size[2] * z];
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
    //QMutexLocker lock(&mutexYZ);
    if (YZloadedFlag)
        return dataYZ[(size[0] - 1 - z) + size[0] * (size[1] - 1 - y)];
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
    if (type == OpenedH5File::DATASET_TYPE && dataset != NULL) {
        info.append(QPair<QString, QString>("Name", name));
        info.append(QPair<QString, QString>("Type", "3D dataset"));
        info.append(QPair<QString, QString>("Size", QString::number(originalSize[0]) + " x " + QString::number(originalSize[1]) + " x " + QString::number(originalSize[2])));
        if (size[2] != originalSize[2] || size[1] != originalSize[1] || size[0] != originalSize[0])
            info.append(QPair<QString, QString>("Downsampling size", QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));
        info.append(QPair<QString, QString>("Chunk size", QString::number(chunkSize[0]) + " x " + QString::number(chunkSize[1]) + " x " + QString::number(chunkSize[2])));
    } else if (type == OpenedH5File::GROUP_TYPE && group != NULL && dataset != NULL) {
        info.append(QPair<QString, QString>("Name", name));
        info.append(QPair<QString, QString>("Type", "Mask type dataset group"));
        info.append(QPair<QString, QString>("Size", QString::number(originalSize[0]) + " x " + QString::number(originalSize[1]) + " x " + QString::number(originalSize[2])));
        if (size[2] != originalSize[2] || size[1] != originalSize[1] || size[0] != originalSize[0])
            info.append(QPair<QString, QString>("Downsampling size", QString::number(size[0]) + " x " + QString::number(size[1]) + " x " + QString::number(size[2])));
        info.append(QPair<QString, QString>("Position", QString::number(originalPos[0]) + " x " + QString::number(originalPos[1]) + " x " + QString::number(originalPos[2])));
        if (pos[2] != originalPos[2] || pos[1] != originalPos[1] || pos[0] != originalPos[0])
            info.append(QPair<QString, QString>("Downsampling position", QString::number(pos[0]) + " x " + QString::number(pos[1]) + " x " + QString::number(pos[2])));
        info.append(QPair<QString, QString>("Chunk size", QString::number(chunkSize[0]) + " x " + QString::number(chunkSize[1]) + " x " + QString::number(chunkSize[2])));
        info.append(QPair<QString, QString>("Steps", QString::number(steps)));
    }
    return info;
}
