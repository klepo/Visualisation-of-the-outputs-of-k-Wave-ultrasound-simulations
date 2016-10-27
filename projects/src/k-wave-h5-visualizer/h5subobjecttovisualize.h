/**
 * @file        h5subobjecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with H5SubobjectToVisualize class declaration.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef H5SUBOBJECTTOVISUALIZE_H
#define H5SUBOBJECTTOVISUALIZE_H

#include "openedh5file.h"
#include "h5objecttovisualize.h"
#include "hdf5readingthread.h"
#include "cvimagewidget.h"
#include "colormap.h"

class OpenedH5File::H5SubobjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5SubobjectToVisualize(HDF5Helper::HDF5Dataset *dataset, OpenedH5File *openedH5File, QObject *parent = 0);
    explicit H5SubobjectToVisualize(HDF5Helper::HDF5Group *group, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5SubobjectToVisualize();

    // Getters for current settings and min/max values
    uint64_t getXIndex();
    uint64_t getYIndex();
    uint64_t getZIndex();

    float getMinValue();
    float getMaxValue();
    float getOriginalMinValue();
    float getOriginalMaxValue();

    HDF5Helper::HDF5Dataset *getDataset();
    HDF5Helper::HDF5Group *getGroup();

    // Simulation info from file
    QList<QPair<QString, QString>> getInfo();

    float *getDataXY();
    float *getDataXZ();
    float *getDataYZ();

    float getValueAtPointFromXY(int x, int y);
    float getValueAtPointFromXZ(int x, int z);
    float getValueAtPointFromYZ(int y, int z);

    QString getName();
    H5G_obj_t getType();
    ColorMap::Type getColormap();

    HDF5Helper::HDF5Vector3D getFrameSize();
    HDF5Helper::HDF5Vector3D getOriginalFrameSize();
    HDF5Helper::HDF5Vector3D getSize();
    HDF5Helper::HDF5Vector3D getOriginalSize();
    HDF5Helper::HDF5Vector3D getPos();
    HDF5Helper::HDF5Vector3D getOriginalPos();
    uint64_t getSteps();
    uint64_t getCurrentStep();

    float getAlpha();
    float getRed();
    float getGreen();
    float getBlue();
    int getCount();

    bool isGUIInitialized();

    bool isCurrentXYLoaded();
    bool isCurrentXZLoaded();
    bool isCurrentYZLoaded();

    bool areCurrentSlicesLoaded();

signals:
    void imageXYChanged(QImage, uint64_t index);
    void imageXZChanged(QImage, uint64_t index);
    void imageYZChanged(QImage, uint64_t index);

private slots:
    void sliceXYLoaded(Request *r);
    void sliceXZLoaded(Request *r);
    void sliceYZLoaded(Request *r);

public slots:
    void setXIndex(uint64_t index);
    void setYIndex(uint64_t index);
    void setZIndex(uint64_t index);

    void setColormap(ColorMap::Type colormap);

    void setMinValue(float value);
    void setMaxValue(float value);

    void setAlpha(float value);
    void setRed(float value);
    void setGreen(float value);
    void setBlue(float value);
    void setCount(int value);

    void setCurrentStep(uint64_t value);

    void setGUIInitialized(bool value);

    void reloadImages();

private:
    void loadObjectData();
    void initialize();

    void changeImages();

    QImage createImageXY();
    QImage createImageXZ();
    QImage createImageYZ();

    OpenedH5File *openedH5File = 0;
    QString objectName;
    H5G_obj_t type;
    HDF5Helper::HDF5Group *group = 0;
    HDF5Helper::HDF5Dataset *dataset = 0;

    bool XYloadedFlag = false;
    bool XZloadedFlag = false;
    bool YZloadedFlag = false;

    HDF5Helper::HDF5Vector3D index;
    HDF5Helper::HDF5Vector3D lastLoadedIndex;

    HDF5ReadingThread *threadXY = 0;
    HDF5ReadingThread *threadXZ = 0;
    HDF5ReadingThread *threadYZ = 0;


    float minValue = 0;
    float maxValue = 0;
    float originalMinValue = 0;
    float originalMaxValue = 0;

    ColorMap::Type colormap = ColorMap::JET;

    float *dataXY = 0;
    float *dataXZ = 0;
    float *dataYZ = 0;

    float alpha = 1;
    float red = 1;
    float green = 1;
    float blue = 1;
    int count = 100;

    // Datasets characteristics variables
    HDF5Helper::HDF5Vector3D originalFrameSize;
    HDF5Helper::HDF5Vector3D frameSize;
    HDF5Helper::HDF5Vector3D originalSize;
    HDF5Helper::HDF5Vector3D size;
    HDF5Helper::HDF5Vector3D originalPos;
    HDF5Helper::HDF5Vector3D pos;
    HDF5Helper::HDF5Vector chunkSize;

    uint64_t steps = 1;
    uint64_t dwnsmpl = 0;
    uint64_t currentStep = 0;

    // GUI initialization flags
    bool GUIInitialized = false;
    bool GUIXYInitialized = false;
    bool GUIXZInitialized = false;
    bool GUIYZInitialized = false;

    bool currentXYLodaded = false;
    bool currentXZLodaded = false;
    bool currentYZLodaded = false;
};

#endif // H5SUBOBJECTTOVISUALIZE_H
