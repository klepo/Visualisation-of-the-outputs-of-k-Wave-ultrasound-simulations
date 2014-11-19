/*
 * @file        h5subobjecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with H5SubobjectToVisualize class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#ifndef H5SUBOBJECTTOVISUALIZE_H
#define H5SUBOBJECTTOVISUALIZE_H

#include <QObject>

#include "openedh5file.h"
#include "hdf5readingthread.h"
#include "cvimagewidget.h"

class OpenedH5File::H5SubobjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5SubobjectToVisualize(HDF5File::HDF5Dataset *dataset, OpenedH5File *openedH5File, QObject *parent = 0);
    explicit H5SubobjectToVisualize(HDF5File::HDF5Group *group, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5SubobjectToVisualize();

    // Getters for current settings and min/max values
    uint64_t getXIndex();
    uint64_t getYIndex();
    uint64_t getZIndex();

    float getMinVG();
    float getMaxVG();
    float getOriginalMinVG();
    float getOriginalMaxVG();
    float getMinVXY();
    float getMaxVXY();
    float getOriginalMinVXY();
    float getOriginalMaxVXY();
    float getMinVXZ();
    float getMaxVXZ();
    float getOriginalMinVXZ();
    float getOriginalMaxVXZ();
    float getMinVYZ();
    float getMaxVYZ();
    float getOriginalMinVYZ();
    float getOriginalMaxVYZ();

    HDF5File::HDF5Dataset *getDataset();
    HDF5File::HDF5Group *getGroup();

    // Simulation info from file
    QList<QPair<QString, QString>> getInfo();

    float *getDataXY();
    float *getDataXZ();
    float *getDataYZ();

    float getValueAtPointFromXY(int x, int y);
    float getValueAtPointFromXZ(int x, int z);
    float getValueAtPointFromYZ(int y, int z);

    QString getName();
    int getType();
    // Other setting getters
    int getColormap();
    bool getUseGlobal();

    uint64_t *getFrameSize();
    uint64_t *getOriginalFrameSize();
    uint64_t *getSize();
    uint64_t *getOriginalSize();
    uint64_t *getPos();
    uint64_t *getOriginalPos();
    uint64_t getSteps();
    uint64_t getCurrentStep();

    float getAlpha();
    float getRed();
    float getGreen();
    float getBlue();
    int getCount();

    bool isGUIInitialized();
    bool isGUIXYInitialized();
    bool isGUIXZInitialized();
    bool isGUIYZInitialized();

    bool isCurrentXYLoaded();
    bool isCurrentXZLoaded();
    bool isCurrentYZLoaded();

    bool areCurrentSlicesLoaded();

signals:
    void imageXYChanged(cv::Mat, int index);
    void imageXZChanged(cv::Mat, int index);
    void imageYZChanged(cv::Mat, int index);

private slots:
    void sliceXYLoaded(Request *r);
    void sliceXZLoaded(Request *r);
    void sliceYZLoaded(Request *r);

public slots:
    void setXIndex(uint64_t index);
    void setYIndex(uint64_t index);
    void setZIndex(uint64_t index);

    void setColormap(int value);

    void setUseGlobal(bool value);

    void setMinVG(float value);
    void setMaxVG(float value);
    void setMinVXY(float value);
    void setMaxVXY(float value);
    void setMinVXZ(float value);
    void setMaxVXZ(float value);
    void setMinVYZ(float value);
    void setMaxVYZ(float value);

    void setAlpha(float value);
    void setRed(float value);
    void setGreen(float value);
    void setBlue(float value);
    void setCount(int value);

    void setCurrentStep(uint64_t value, HDF5ReadingThread *thread3D);

    void setGUIInitialized(bool value);
    void setGUIXYInitialized(bool value);
    void setGUIXZInitialized(bool value);
    void setGUIYZInitialized(bool value);

    void reloadImages();

private:
    void loadObjectData();
    void initialize();

    void changeImages();

    cv::Mat createImageXY();
    cv::Mat createImageXZ();
    cv::Mat createImageYZ();

    OpenedH5File *openedH5File;

    QString name;

    HDF5File::HDF5Dataset *dataset;
    HDF5File::HDF5Group *group;

    int type;

    uint64_t xIndex;
    uint64_t yIndex;
    uint64_t zIndex;
    uint64_t lastLoadedXIndex;
    uint64_t lastLoadedYIndex;
    uint64_t lastLoadedZIndex;

    QMutex mutexXY;
    QMutex mutexXZ;
    QMutex mutexYZ;


    HDF5ReadingThread *threadXY;
    HDF5ReadingThread *threadXZ;
    HDF5ReadingThread *threadYZ;

    bool XYloadedFlag;
    bool XZloadedFlag;
    bool YZloadedFlag;

    float minVG, maxVG, originalMinVG, originalMaxVG;
    float minVXY, maxVXY, originalMinVXY, originalMaxVXY;
    float minVXZ, maxVXZ, originalMinVXZ, originalMaxVXZ;
    float minVYZ, maxVYZ, originalMinVYZ, originalMaxVYZ;

    int colormap;

    bool useGlobal;

    float *dataXY;
    float *dataXZ;
    float *dataYZ;

    float alpha;
    float red;
    float green;
    float blue;
    int count;

    // Datasets characteristics variables
    uint64_t originalFrameSize[3];
    uint64_t frameSize[3];
    uint64_t originalSize[3];
    uint64_t size[3];
    uint64_t chunkSize[3];
    uint64_t originalPos[3];
    uint64_t pos[3];
    uint64_t steps;
    uint64_t dwnsmpl;
    uint64_t currentStep;

    // GUI initialization flags
    bool GUIInitialized;
    bool GUIXYInitialized;
    bool GUIXZInitialized;
    bool GUIYZInitialized;

    bool currentXYLodaded;
    bool currentXZLodaded;
    bool currentYZLodaded;
};

#endif // H5SUBOBJECTTOVISUALIZE_H
