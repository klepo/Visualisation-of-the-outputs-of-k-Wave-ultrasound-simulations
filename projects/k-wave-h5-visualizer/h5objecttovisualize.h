/**
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The header file with H5ObjectToVisualize class declaration.
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

#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include <QImage>

#include <h5openedfile.h>
#include <h5readingthread.h>
#include <colormap.h>

/**
 * @brief The H5ObjectToVisualize class represents wrapper
 * for subobjects for visualization
 */
class H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(H5Helper::Dataset *dataset, H5OpenedFile::ObjectType type, QObject *parent = nullptr);
    ~H5ObjectToVisualize();

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

    H5Helper::File *getFile() const;

    bool isSelected() const;

    QVector<float> getOpacity() const;

    H5Helper::CompressHelper *getCompressHelper() const;

    bool getData3DLoadingFlag() const;

    float *getData3D() const;
    float *getData3DLC() const;
    float *getData3DCC() const;
    hsize_t getLocalStep() const;

    bool areCurrentData3DLoaded() const;

    bool getMinMaxValuesTrim() const;

signals:
    void minValueChanged(float value);
    void maxValueChanged(float value);
    void colormapChanged(ColorMap::Type colormap);
    void opacityChanged(QVector<float> value);
    void minMaxValuesTrimChanged(bool value);

    void currentSlicesLoaded();
    void currentXYLoaded();
    void currentXZLoaded();
    void currentYZLoaded();
    void currentData3DLoaded();
    void currentStepLoaded();

    void dataXYLoadingStarted();
    void dataXZLoadingStarted();
    void dataYZLoadingStarted();
    void data3DLoadingStarted();

    void xIndexChanged(int value);
    void yIndexChanged(int value);
    void zIndexChanged(int value);
    void stepChanged(int value);

    void data3DChanged(float *data3D);
    void data3DCompressChanged(float *data3DLC, float *data3DCC, hsize_t localStep);
    void dataYZChanged(float *data, hsize_t index);
    void dataXYChanged(float *data, hsize_t index);
    void dataXZChanged(float *data, hsize_t index);

    void imageXYChanged(QImage image);
    void imageXZChanged(QImage image);
    void imageYZChanged(QImage image);

    void hoveredPointInImage(float value);

public slots:
    void setMinValue(float value);
    void setMaxValue(float value);
    void setColormap(ColorMap::Type colormap);
    void setColormap(int colormap);
    void setOpacity(const QVector<float> &value);
    void setMinMaxValuesTrim(bool value);

    void setXIndex(int index);
    void setYIndex(int index);
    void setZIndex(int index);

    void setToMaxValuePosition();
    void setToMinValuePosition();

    void setCurrentStep(int step);

    void reloadSlices();
    void reloadXY();
    void reloadXZ();
    void reloadYZ();

    void setSelected(bool value);
    void toggleSelected();

    void setData3DLoadingFlag(bool value);

    void setHoveredPointInImageXY(int x, int y);
    void setHoveredPointInImageXZ(int x, int z);
    void setHoveredPointInImageYZ(int y, int z);

    void disconnectSignals();

private slots:
    void sliceXYLoaded(Request *request);
    void sliceXZLoaded(Request *request);
    void sliceYZLoaded(Request *request);
    void data3DLoaded(Request *request);

private:
    Q_DISABLE_COPY(H5ObjectToVisualize)

    void initialize();
    void loadObjectData();
    void checkCurrentDataIsLoaded();
    void changeImages();
    void load3Ddata();

    QImage createImageXY();
    QImage createImageXZ();
    QImage createImageYZ();

    bool selectedFlag = false;

    H5Helper::Dataset *dataset = nullptr;
    H5OpenedFile::ObjectType type;

    bool currentXYLoadedFlag = false;
    bool currentXZLoadedFlag = false;
    bool currentYZLoadedFlag = false;
    bool currentData3DLoadedFlag = false;

    bool loadData3DFlag = false;
    bool loadSlicesFlag = false;

    H5Helper::Vector4D index;

    H5ReadingThread *threadXY = nullptr;
    H5ReadingThread *threadXZ = nullptr;
    H5ReadingThread *threadYZ = nullptr;
    H5ReadingThread *thread3D = nullptr;

    // Memory for loaded slices and 3D data
    float *dataXY = nullptr;
    float *dataXZ = nullptr;
    float *dataYZ = nullptr;
    float *data3D = nullptr;
    //float *data3DLC = nullptr;
    //float *data3DCC = nullptr;

    // Dataset settings
    float minValue = 0;
    float maxValue = 0;
    ColorMap::Type colormap = ColorMap::JET;
    QVector<float> opacity;
    bool minMaxValuesTrimFlag = false;

    // Original min/max values
    float originalMinValue = 0;
    float originalMaxValue = 0;

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

    // Compression helper
    H5Helper::CompressHelper *compressHelper = nullptr;
};

#endif // H5OBJECTTOVISUALIZE_H
