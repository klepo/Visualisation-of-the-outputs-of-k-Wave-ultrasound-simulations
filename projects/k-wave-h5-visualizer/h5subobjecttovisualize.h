/**
 * @file        h5subobjecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with OpenedH5File::H5SubobjectToVisualize class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef H5SUBOBJECTTOVISUALIZE_H
#define H5SUBOBJECTTOVISUALIZE_H

#include "openedh5file.h"
#include "h5objecttovisualize.h"
#include "hdf5readingthread.h"
#include "imagewidget.h"
#include "colormap.h"

/**
 * @brief The H5SubobjectToVisualize class represents wrapper
 * for subobjects for visualization
 */
class OpenedH5File::H5SubobjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5SubobjectToVisualize(HDF5Helper::Dataset *dataset, ObjectType type, OpenedH5File *openedH5File, H5ObjectToVisualize *h5ObjectToVisualize, QObject *parent = 0);
    ~H5SubobjectToVisualize();

    // Getters for current settings and min/max values
    hsize_t getXIndex();
    hsize_t getYIndex();
    hsize_t getZIndex();

    float getMinValue();
    float getMaxValue();
    float getOriginalMinValue();
    float getOriginalMaxValue();

    HDF5Helper::Dataset *getDataset();

    // Simulation info from file
    QList<QPair<QString, QString>> getInfo();

    float *getDataXY();
    float *getDataXZ();
    float *getDataYZ();

    float getValueAtPointFromXY(int x, int y);
    float getValueAtPointFromXZ(int x, int z);
    float getValueAtPointFromYZ(int y, int z);

    QString getName();
    QString getOnlyName() const;
    ObjectType getType();
    ColorMap::Type getColormap();

    HDF5Helper::Vector3D getFrameSize();
    HDF5Helper::Vector3D getOriginalFrameSize();
    HDF5Helper::Vector3D getSize();
    HDF5Helper::Vector3D getOriginalSize();
    HDF5Helper::Vector3D getPos();
    HDF5Helper::Vector3D getOriginalPos();
    hsize_t getSteps();
    hsize_t getCurrentStep();

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

    OpenedH5File *getOpenedH5File() const;
    H5ObjectToVisualize *getH5ObjectToVisualize() const;

signals:
    /**
     * @brief Image XY changed signal
     * @param[in] image Image
     * @param[in] index Index
     */
    void imageXYChanged(QImage image, hsize_t index);
    /**
     * @brief image XZ changed signal
     * @param[in] image Image
     * @param[in] index Index
     */
    void imageXZChanged(QImage image, hsize_t index);
    /**
     * @brief image YZ changed signal
     * @param[in] image Image
     * @param[in] index Index
     */
    void imageYZChanged(QImage image, hsize_t index);

private slots:
    void sliceXYLoaded(Request *r);
    void sliceXZLoaded(Request *r);
    void sliceYZLoaded(Request *r);

public slots:
    void setXIndex(hsize_t index);
    void setYIndex(hsize_t index);
    void setZIndex(hsize_t index);

    void setColormap(ColorMap::Type colormap);

    void setMinValue(float value);
    void setMaxValue(float value);

    void setAlpha(float value);
    void setRed(float value);
    void setGreen(float value);
    void setBlue(float value);
    void setCount(int value);

    void setCurrentStep(hsize_t step);

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
    H5ObjectToVisualize *h5ObjectToVisualize = 0;
    QString objectName;
    QString objectOnlyName;
    ObjectType type;
    HDF5Helper::Dataset *dataset = 0;

    bool XYloadedFlag = false;
    bool XZloadedFlag = false;
    bool YZloadedFlag = false;

    HDF5Helper::Vector3D index;
    HDF5Helper::Vector3D lastLoadedIndex;

    HDF5ReadingThread *threadXY = 0;
    HDF5ReadingThread *threadXZ = 0;
    HDF5ReadingThread *threadYZ = 0;


    float minValue = 0;
    float maxValue = 0;
    float originalMinValue = 0;
    float originalMaxValue = 0;
    HDF5Helper::Vector minValuePosition;
    HDF5Helper::Vector maxValuePosition;

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
    HDF5Helper::Vector3D originalFrameSize;
    HDF5Helper::Vector3D frameSize;
    HDF5Helper::Vector3D originalSize;
    HDF5Helper::Vector3D size;
    HDF5Helper::Vector3D originalPosition;
    HDF5Helper::Vector3D position;
    HDF5Helper::Vector chunkSize;

    hsize_t steps = 1;
    hsize_t currentStep = 0;

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
