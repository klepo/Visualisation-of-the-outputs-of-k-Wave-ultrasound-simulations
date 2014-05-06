#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"

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
    dataXY = new float[size[1] * size[2]];
    dataXZ = new float[size[0] * size[2]];
    dataYZ = new float[size[0] * size[1]];
}

OpenedH5File::H5SubobjectToVisualize::H5SubobjectToVisualize(HDF5File::HDF5Group *group, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = QString::fromStdString(group->getName());
    this->group = group;
    this->dataset = openedH5File->file->openDataset(name.toStdString() + "/" + std::to_string(0));
    this->type = OpenedH5File::GROUP_TYPE;
    initialize();
    loadObjectData();
    dataXY = new float[size[1] * size[2]];
    dataXZ = new float[size[0] * size[2]];
    dataYZ = new float[size[0] * size[1]];
}

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

void OpenedH5File::H5SubobjectToVisualize::setGUIInitialized(bool value)
{
    GUIInitialized = value;
}

bool OpenedH5File::H5SubobjectToVisualize::isGUIInitialized()
{
    return GUIInitialized;
}

void OpenedH5File::H5SubobjectToVisualize::setGUIXYInitialized(bool value)
{
    GUIXYInitialized = value;
}

bool OpenedH5File::H5SubobjectToVisualize::isGUIXYInitialized()
{
    return GUIXYInitialized;
}

void OpenedH5File::H5SubobjectToVisualize::setGUIXZInitialized(bool value)
{
    GUIXZInitialized = value;
}

bool OpenedH5File::H5SubobjectToVisualize::isGUIXZInitialized()
{
    return GUIXZInitialized;
}

void OpenedH5File::H5SubobjectToVisualize::setGUIYZInitialized(bool value)
{
    GUIYZInitialized = value;
}

bool OpenedH5File::H5SubobjectToVisualize::isGUIYZInitialized()
{
    return GUIYZInitialized;
}

QString OpenedH5File::H5SubobjectToVisualize::getName()
{
    return name;
}

int OpenedH5File::H5SubobjectToVisualize::getType()
{
    return type;
}

HDF5File::HDF5Dataset *OpenedH5File::H5SubobjectToVisualize::getDataset()
{
    return dataset;
}

HDF5File::HDF5Group *OpenedH5File::H5SubobjectToVisualize::getGroup()
{
    return group;
}

void OpenedH5File::H5SubobjectToVisualize::loadObjectData()
{
    if (type == OpenedH5File::DATASET_TYPE &&  dataset != NULL) {
        if (dataset->hasAttribute("dwnsmpl")) {
            dwnsmpl =  dataset->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
            std::cout << "No downsampling" << std::endl;
        }
        hsize_t *sizeD =  dataset->getDims();
        originalSize[0] = openedH5File->getNZ();
        originalSize[1] = openedH5File->getNY();
        originalSize[2] = openedH5File->getNX();
        size[0] = sizeD[0];
        size[1] = sizeD[1];
        size[2] = sizeD[2];
        originalFrameSize[0] = originalSize[0];
        originalFrameSize[1] = originalSize[1];
        originalFrameSize[2] = originalSize[2];
        frameSize[0] = size[0];
        frameSize[1] = size[1];
        frameSize[2] = size[2];
        originalPos[0] = 0;
        originalPos[1] = 0;
        originalPos[2] = 0;
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;
        sizeD =  dataset->getChunkDims();
        chunkSize[0] = sizeD[0];
        chunkSize[1] = sizeD[1];
        chunkSize[2] = sizeD[2];
        minVG =  dataset->getGlobalMinValueF();
        maxVG =  dataset->getGlobalMaxValueF();
        originalMinVG =  dataset->getGlobalMinValueF();
        originalMaxVG =  dataset->getGlobalMaxValueF();
        steps = 1;
    } else if (type = OpenedH5File::GROUP_TYPE &&  group != NULL &&  dataset != NULL) {
        if (group->hasAttribute("dwnsmpl")) {
            dwnsmpl =  group->readAttributeI("dwnsmpl");
        } else {
            dwnsmpl = 0;
            std::cout << "No downsampling" << std::endl;
        }
        hsize_t *sizeD =  dataset->getDims();
        originalSize[0] = sizeD[0];
        originalSize[1] = sizeD[1];
        originalSize[2] = sizeD[2];
        size[0] = sizeD[0];
        size[1] = sizeD[1];
        size[2] = sizeD[2];
        originalPos[0] =  group->readAttributeI("positionZ");
        originalPos[1] =  group->readAttributeI("positionY");
        originalPos[2] =  group->readAttributeI("positionX");
        pos[0] =  group->readAttributeI("positionZ");
        pos[1] =  group->readAttributeI("positionY");
        pos[2] =  group->readAttributeI("positionX");
        originalFrameSize[0] = openedH5File->getNZ();
        originalFrameSize[1] = openedH5File->getNY();
        originalFrameSize[2] = openedH5File->getNX();
        frameSize[0] = openedH5File->getNZ();
        frameSize[1] = openedH5File->getNY();
        frameSize[2] = openedH5File->getNX();
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
        sizeD =  dataset->getChunkDims();
        chunkSize[0] = sizeD[0];
        chunkSize[1] = sizeD[1];
        chunkSize[2] = sizeD[2];
        minVG =  group->readAttributeF("min");
        maxVG =  group->readAttributeF("max");
        originalMinVG =  group->readAttributeF("min");
        originalMaxVG =  group->readAttributeF("max");
        steps =  group->readAttributeI("count");
    }
}

void OpenedH5File::H5SubobjectToVisualize::changeImages()
{
    emit imageXYChanged(createImageXY(), zIndex);
    emit imageXZChanged(createImageXZ(), yIndex);
    emit imageYZChanged(createImageYZ(), xIndex);
}

void OpenedH5File::H5SubobjectToVisualize::reloadImages()
{
    setZIndex(zIndex);
    setYIndex(yIndex);
    setXIndex(xIndex);
}

void OpenedH5File::H5SubobjectToVisualize::sliceXYLoaded(Request *r)
{
    //QMutexLocker lock(&mutexXY);
    XYloadedFlag = false;
    //delete [] dataXY;
    //dataXY = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataXY = new float[size];
    memcpy(dataXY, r->data, size[1] * size[2] * sizeof(float));

    minVXY = r->min;
    originalMinVXY = minVXY;
    maxVXY = r->max;
    originalMaxVXY = maxVXY;

    XYloadedFlag = true;
    //lock.unlock();
    if (zIndex == r->zO)
        currentXYLodaded = true;
    else
        currentXYLodaded = false;
    emit imageXYChanged(createImageXY(), r->zO);
    threadXY->deleteDoneRequest(r);
}

void OpenedH5File::H5SubobjectToVisualize::sliceXZLoaded(Request *r)
{
    //QMutexLocker lock(&mutexXZ);
    XZloadedFlag = false;
    //delete [] dataXZ;
    //dataXZ = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataXZ = new float[size];
    memcpy(dataXZ, r->data, size[0] * size[2] * sizeof(float));

    minVXZ = r->min;
    originalMinVXZ = minVXZ;
    maxVXZ = r->max;
    originalMaxVXZ = maxVXZ;

    XZloadedFlag = true;
    //lock.unlock();
    if (yIndex == r->yO)
        currentXZLodaded = true;
    else
        currentXZLodaded = false;
    emit imageXZChanged(createImageXZ(), r->yO);
    threadXZ->deleteDoneRequest(r);
}

void OpenedH5File::H5SubobjectToVisualize::sliceYZLoaded(Request *r)
{
    //QMutexLocker lock(&mutexYZ);
    YZloadedFlag = false;
    //delete [] dataYZ;
    //dataYZ = NULL;

    //hsize_t size = r->zC * r->yC * r->xC;
    //dataYZ = new float[size];
    memcpy(dataYZ, r->data, size[0] * size[1] * sizeof(float));

    minVYZ = r->min;
    originalMinVYZ = minVYZ;
    maxVYZ = r->max;
    originalMaxVYZ = maxVYZ;

    YZloadedFlag = true;
    //lock.unlock();
    if (xIndex == r->xO)
        currentYZLodaded = true;
    else
        currentYZLodaded = false;
    emit imageYZChanged(createImageYZ(), r->xO);
    threadYZ->deleteDoneRequest(r);
}

bool OpenedH5File::H5SubobjectToVisualize::isCurrentXYLoaded()
{
    return currentXYLodaded;
}

bool OpenedH5File::H5SubobjectToVisualize::isCurrentXZLoaded()
{
    return currentXZLodaded;
}

bool OpenedH5File::H5SubobjectToVisualize::isCurrentYZLoaded()
{
    return currentYZLodaded;
}

bool OpenedH5File::H5SubobjectToVisualize::areCurrentSlicesLoaded()
{
    if (currentXYLodaded && currentXZLodaded && currentYZLodaded)
        return true;
    else
        return false;
}

cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageXY()
{
    //QMutexLocker lock(&mutexXY);
    cv::Mat image;
    if (XYloadedFlag) {
        image = cv::Mat(size[1], size[2], CV_32FC1, dataXY); // rows, cols (height, width)
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXY - minVXY), - minVXY * 255.0 / (maxVXY - minVXY));
        cv::applyColorMap(image, image, colormap);
        cv::flip(image, image, 0);
    }
    return image;
}

cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageXZ()
{
    //QMutexLocker lock(&mutexXZ);
    cv::Mat image;
    if (XZloadedFlag) {
        image = cv::Mat(size[0], size[2], CV_32FC1, dataXZ); // rows, cols (height, width)
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVXZ - minVXZ), - minVXZ * 255.0 / (maxVXZ - minVXZ));
        cv::applyColorMap(image, image, colormap);
    }
    return image;
}

cv::Mat OpenedH5File::H5SubobjectToVisualize::createImageYZ()
{
    //QMutexLocker lock(&mutexYZ);
    cv::Mat image;
    if (YZloadedFlag) {
        image = cv::Mat(size[0], size[1], CV_32FC1, dataYZ); // rows, cols (height, width)
        if (useGlobal)
            image.convertTo(image, CV_8UC1, 255.0 / (maxVG - minVG), - minVG * 255.0 / (maxVG - minVG));
        else
            image.convertTo(image, CV_8UC1, 255.0 / (maxVYZ - minVYZ), - minVYZ * 255.0 / (maxVYZ - minVYZ));
        cv::applyColorMap(image, image, colormap);
        cv::transpose(image, image);
        cv::flip(image, image, 0);
        cv::flip(image, image, 1);
    }
    return image;
}

float *OpenedH5File::H5SubobjectToVisualize::getDataXY()
{
    return dataXY;
}

float *OpenedH5File::H5SubobjectToVisualize::getDataXZ()
{
    return dataXZ;
}

float *OpenedH5File::H5SubobjectToVisualize::getDataYZ()
{
    return dataYZ;
}

uint64_t OpenedH5File::H5SubobjectToVisualize::getXIndex()
{
    return xIndex;
}

uint64_t OpenedH5File::H5SubobjectToVisualize::getYIndex()
{
    return yIndex;
}

uint64_t OpenedH5File::H5SubobjectToVisualize::getZIndex()
{
    return zIndex;
}

void OpenedH5File::H5SubobjectToVisualize::setXIndex(uint64_t value)
{
    xIndex = value;
    currentXYLodaded = false;
    threadYZ->createRequest(dataset, 0, 0, xIndex, size[0], size[1], 1);
    threadYZ->start();
}

void OpenedH5File::H5SubobjectToVisualize::setYIndex(uint64_t value)
{
    yIndex = value;
    currentXZLodaded = false;
    threadXZ->createRequest(dataset, 0, yIndex, 0, size[0], 1, size[2]);
    threadXZ->start();
}

void OpenedH5File::H5SubobjectToVisualize::setZIndex(uint64_t value)
{
    zIndex = value;
    currentYZLodaded = false;
    threadXY->createRequest(dataset, zIndex, 0, 0, 1, size[1], size[2]);
    threadXY->start();
}

float OpenedH5File::H5SubobjectToVisualize::getAlpha()
{
    return alpha;
}

float OpenedH5File::H5SubobjectToVisualize::getRed()
{
    return red;
}

float OpenedH5File::H5SubobjectToVisualize::getGreen()
{
    return green;
}

float OpenedH5File::H5SubobjectToVisualize::getBlue()
{
    return blue;
}

int OpenedH5File::H5SubobjectToVisualize::getCount()
{
    return count;
}

void OpenedH5File::H5SubobjectToVisualize::setAlpha(float value)
{
    alpha = value;
}

void OpenedH5File::H5SubobjectToVisualize::setRed(float value)
{
    red = value;
}

void OpenedH5File::H5SubobjectToVisualize::setGreen(float value)
{
    green = value;
}

void OpenedH5File::H5SubobjectToVisualize::setBlue(float value)
{
    blue = value;
}

void OpenedH5File::H5SubobjectToVisualize::setCount(int value)
{
    count = value;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getSize()
{
    return size;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalSize()
{
    return originalSize;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getFrameSize()
{
    return frameSize;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalFrameSize()
{
    return originalFrameSize;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getPos()
{
    return pos;
}

uint64_t *OpenedH5File::H5SubobjectToVisualize::getOriginalPos()
{
    return originalPos;
}

uint64_t OpenedH5File::H5SubobjectToVisualize::getSteps()
{
    return steps;
}

uint64_t OpenedH5File::H5SubobjectToVisualize::getCurrentStep()
{
    return currentStep;
}

void OpenedH5File::H5SubobjectToVisualize::setCurrentStep(uint64_t value, HDF5ReadingThread *thread3D)
{
    if (type == OpenedH5File::GROUP_TYPE) {
        try {
            threadXY->clearRequests();
            threadXY->wait();

            threadXZ->clearRequests();
            threadXZ->wait();

            threadYZ->clearRequests();
            threadYZ->wait();

            thread3D->clearRequests();
            thread3D->wait();

            openedH5File->file->closeDataset(this->dataset->getName());
            this->dataset = openedH5File->file->openDataset(name.toStdString() + "/" + std::to_string(value));
            currentStep = value;

            reloadImages();
        } catch(std::exception &) {
            std::cerr << "Wrong step" << std::endl;
        }
    }
}

int OpenedH5File::H5SubobjectToVisualize::getColormap()
{
    return colormap;
}

void OpenedH5File::H5SubobjectToVisualize::setColormap(int value)
{
    colormap = value;
    changeImages();
}

bool OpenedH5File::H5SubobjectToVisualize::getUseGlobal()
{
    return useGlobal;
}

void OpenedH5File::H5SubobjectToVisualize::setUseGlobal(bool value)
{
    useGlobal = value;
    changeImages();
}

void OpenedH5File::H5SubobjectToVisualize::setMinVG(float value)
{
    minVG = value;
    changeImages();
}

void OpenedH5File::H5SubobjectToVisualize::setMaxVG(float value)
{
    maxVG = value;
    changeImages();
}

void OpenedH5File::H5SubobjectToVisualize::setMinVXY(float value)
{
    minVXY = value;
    emit imageXYChanged(createImageXY(), zIndex);
}

void OpenedH5File::H5SubobjectToVisualize::setMaxVXY(float value)
{
    maxVXY = value;
    emit imageXYChanged(createImageXY(), zIndex);
}

void OpenedH5File::H5SubobjectToVisualize::setMinVXZ(float value)
{
    minVXZ = value;
    emit imageXZChanged(createImageXZ(), yIndex);
}

void OpenedH5File::H5SubobjectToVisualize::setMaxVXZ(float value)
{
    maxVXZ = value;
    emit imageXZChanged(createImageXZ(), yIndex);
}

void OpenedH5File::H5SubobjectToVisualize::setMinVYZ(float value)
{
    minVYZ = value;
    emit imageYZChanged(createImageYZ(), xIndex);
}

void OpenedH5File::H5SubobjectToVisualize::setMaxVYZ(float value)
{
    maxVYZ = value;
    emit imageYZChanged(createImageYZ(), xIndex);
}

float OpenedH5File::H5SubobjectToVisualize::getMinVG()
{
    return minVG;
}

float OpenedH5File::H5SubobjectToVisualize::getMaxVG()
{
    return maxVG;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVG()
{
    return originalMinVG;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVG()
{
    return originalMaxVG;
}

float OpenedH5File::H5SubobjectToVisualize::getMinVXY()
{
    return minVXY;
}

float OpenedH5File::H5SubobjectToVisualize::getMaxVXY()
{
    return maxVXY;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXY()
{
    return originalMinVXY;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXY()
{
    return originalMaxVXY;
}

float OpenedH5File::H5SubobjectToVisualize::getMinVXZ()
{
    return minVXZ;
}

float OpenedH5File::H5SubobjectToVisualize::getMaxVXZ()
{
    return maxVXZ;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVXZ()
{
    return originalMinVXZ;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVXZ()
{
    return originalMaxVXZ;
}

float OpenedH5File::H5SubobjectToVisualize::getMinVYZ()
{
    return minVYZ;
}

float OpenedH5File::H5SubobjectToVisualize::getMaxVYZ()
{
    return maxVYZ;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMinVYZ()
{
    return originalMinVYZ;
}

float OpenedH5File::H5SubobjectToVisualize::getOriginalMaxVYZ()
{
    return originalMaxVYZ;
}

float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXY(int x, int y)
{
    //QMutexLocker lock(&mutexXY);
    if (XYloadedFlag)
        return dataXY[x + size[2] - 1 * (size[1] - 1 - y)];
    else
        return 0.0;
}

float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromXZ(int x, int z)
{
    //QMutexLocker lock(&mutexXZ);
    if (XZloadedFlag)
        return dataXZ[x + size[2] - 1 * z];
    else
        return 0.0;
}

float OpenedH5File::H5SubobjectToVisualize::getValueAtPointFromYZ(int y, int z)
{
    //QMutexLocker lock(&mutexYZ);
    if (YZloadedFlag)
        return dataYZ[(size[0] - 1 - z) + size[0] - 1 * (size[1] - 1 - y)];
    else
        return 0.0;
}

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
    } else if (type = OpenedH5File::GROUP_TYPE && group != NULL && dataset != NULL) {
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
