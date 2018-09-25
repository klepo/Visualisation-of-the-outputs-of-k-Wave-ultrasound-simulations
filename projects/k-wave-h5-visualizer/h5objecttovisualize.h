/**
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with H5ObjectToVisualize class declaration.
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

#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include "h5openedfile.h"
#include "h5readingthread.h"
#include "imagewidget.h"
#include "colormap.h"

/**
 * @brief The H5ObjectToVisualize class represents wrapper
 * for subobjects for visualization
 */
class H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(H5Helper::Dataset *dataset, H5OpenedFile::ObjectType type, H5OpenedFile *openedH5File, QObject *parent = 0);
    ~H5ObjectToVisualize();

    // Getters for current settings and min/max values
    hsize_t getXIndex();
    hsize_t getYIndex();
    hsize_t getZIndex();

    float getMinValue();
    float getMaxValue();
    float getOriginalMinValue();
    float getOriginalMaxValue();

    H5Helper::Dataset *getDataset();

    // Simulation info from file
    QList<QPair<QString, QString>> getInfo();

    float *getDataXY();
    float *getDataXZ();
    float *getDataYZ();

    QImage getImageXY();
    QImage getImageXZ();
    QImage getImageYZ();

    float getValueAtPointFromXY(int x, int y);
    float getValueAtPointFromXZ(int x, int z);
    float getValueAtPointFromYZ(int y, int z);

    QString getName();
    QString getOnlyName() const;
    H5OpenedFile::ObjectType getType();
    ColorMap::Type getColormap();

    H5Helper::Vector3D getFrameSize();
    H5Helper::Vector3D getOriginalFrameSize();
    H5Helper::Vector3D getSize();
    H5Helper::Vector3D getOriginalSize();
    H5Helper::Vector3D getPos();
    H5Helper::Vector3D getOriginalPos();
    hsize_t getSteps();
    hsize_t getCurrentStep();

    int getSlicesCount();

    bool isCurrentXYLoaded();
    bool isCurrentXZLoaded();
    bool isCurrentYZLoaded();

    bool areCurrentSlicesLoaded();

    H5OpenedFile *getOpenedH5File() const;

    bool isSelected() const;

    QVector<float> getOpacity() const;

    H5Helper::CompressHelper *getCompressHelper() const;

    bool getData3DloadingFlag() const;

    float *getData3D() const;
    float *getData3DLC() const;
    float *getData3DCC() const;
    hsize_t getLocalStep() const;

    bool areCurrentData3DLoaded() const;

signals:
    void imageXYChanged(QImage image);
    void imageXZChanged(QImage image);
    void imageYZChanged(QImage image);

    void dataYZChanged(float *data, hsize_t index);
    void dataXYChanged(float *data, hsize_t index);
    void dataXZChanged(float *data, hsize_t index);

    void opacityChanged(QVector<float> value);

    void minValueChanged(float value);
    void maxValueChanged(float value);

    void colormapChanged(ColorMap::Type colormap);

    void stepChanged();

    void slicesLoaded();

    void data3DLoading();

    void data3DLoaded(float *data3D);

    void data3DCompressLoaded(float *data3DLC, float *data3DCC, hsize_t localStep);

public slots:
    void setXIndex(hsize_t index);
    void setYIndex(hsize_t index);
    void setZIndex(hsize_t index);

    void setColormap(ColorMap::Type colormap);
    void setColormap(int colormap);

    void setMinValue(float value);
    void setMaxValue(float value);

    void setSlicesCount(int value);

    void setCurrentStep(hsize_t step);

    void reloadSlices();
    void reloadXY();
    void reloadXZ();
    void reloadYZ();

    void setSelected(bool value);
    void toggleSelected();

    void setOpacity(const QVector<float> &value);

    void setData3DloadingFlag(bool value);

private slots:
    void sliceXYLoaded(Request *r);
    void sliceXZLoaded(Request *r);
    void sliceYZLoaded(Request *r);

    void data3DLoaded(Request *request);

private:
    void loadObjectData();
    void initialize();

    void changeImages();

    QImage createImageXY();
    QImage createImageXZ();
    QImage createImageYZ();

    void load3Ddata();

    bool selected = false;

    H5Helper::Dataset *dataset = 0;
    H5OpenedFile *openedH5File = 0;
    H5OpenedFile::ObjectType type;

    bool XYloadedFlag = false;
    bool XZloadedFlag = false;
    bool YZloadedFlag = false;

    bool data3DLoadedFlag = false;

    bool currentXYLoaded = false;
    bool currentXZLoaded = false;
    bool currentYZLoaded = false;

    bool currentData3DLoaded = false;

    bool data3DloadingFlag = false;

    H5Helper::Vector3D index;

    H5ReadingThread *threadXY = nullptr;
    H5ReadingThread *threadXZ = nullptr;
    H5ReadingThread *threadYZ = nullptr;
    H5ReadingThread *thread3D = nullptr;

    float *dataXY = nullptr;
    float *dataXZ = nullptr;
    float *dataYZ = nullptr;

    float *data3D = nullptr;
    float *data3DLC = nullptr;
    float *data3DCC = nullptr;

    float minValue = 0;
    float maxValue = 0;
    float originalMinValue = 0;
    float originalMaxValue = 0;

    ColorMap::Type colormap = ColorMap::JET;

    int slicesCount = 100;

    QVector<float> opacity;

    hsize_t currentStep = 0;

    // Datasets characteristics variables
    H5Helper::Vector3D originalFrameSize;
    H5Helper::Vector3D frameSize;
    H5Helper::Vector3D originalSize;
    H5Helper::Vector3D size;
    H5Helper::Vector3D originalPosition;
    H5Helper::Vector3D position;
    H5Helper::Vector chunkSize;
    H5Helper::Vector minValuePosition;
    H5Helper::Vector maxValuePosition;
    hsize_t steps = 1;

    H5Helper::CompressHelper *compressHelper = 0;
};

#endif // H5OBJECTTOVISUALIZE_H
