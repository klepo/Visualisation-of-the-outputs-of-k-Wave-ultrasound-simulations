/**
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              29 October   2018 (updated)
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

#include "h5openedfile.h"
#include "h5readingthread.h"
#include "colormap.h"

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
    /**
     * @brief Min value changed signal
     * @param[in] value Minimal value
     */
    void minValueChanged(float value);
    /**
     * @brief Max value changed signal
     * @param[in] value Maximal value
     */
    void maxValueChanged(float value);
    /**
     * @brief Colormap changed signal
     * @param[in] colormap Colormap
     */
    void colormapChanged(ColorMap::Type colormap);
    /**
     * @brief Opacity changed signal
     * @param[in] opacity Opacity
     */
    void opacityChanged(QVector<float> opacity);
    /**
     * @brief Min/max values trim changed signal
     * @param[in] value True/False
     */
    void minMaxValuesTrimChanged(bool value);

    /**
     * @brief Current slices loaded signal
     */
    void currentSlicesLoaded();
    /**
     * @brief Current XY loaded signal
     */
    void currentXYLoaded();
    /**
     * @brief Current XZ loaded signal
     */
    void currentXZLoaded();
    /**
     * @brief Current YZ loaded signal
     */
    void currentYZLoaded();
    /**
     * @brief Current data3D loaded signal
     */
    void currentData3DLoaded();
    /**
     * @brief Current step loaded signal
     */
    void currentStepLoaded();

    /**
     * @brief Data XY loading started signal
     */
    void dataXYLoadingStarted();
    /**
     * @brief Data XZ loading started signal
     */
    void dataXZLoadingStarted();
    /**
     * @brief Data YZ loading started signal
     */
    void dataYZLoadingStarted();
    /**
     * @brief Data 3D loading started signal
     */
    void data3DLoadingStarted();

    /**
     * @brief X index changed signal
     * @param[in] value X index
     */
    void xIndexChanged(int value);
    /**
     * @brief Y index changed signal
     * @param[in] value Y index
     */
    void yIndexChanged(int value);
    /**
     * @brief Z index changed signal
     * @param[in] value Z index
     */
    void zIndexChanged(int value);
    /**
     * @brief Step changed signal
     * @param[in] value Step
     */
    void stepChanged(int value);

    /**
     * @brief Data 3D changed signal
     * @param[in] data3D Data 3D
     */
    void data3DChanged(float *data3D);
    /**
     * @brief Data 3D compress changed signal
     * @param[in] data3DLC Data 3D for last compress coefficient
     * @param[in] data3DCC Data 3D for current compress coefficient
     * @param[in] localStep Local step
     */
    void data3DCompressChanged(float *data3DLC, float *data3DCC, hsize_t localStep);
    /**
     * @brief Data XY changed signal
     * @param[in] data Data
     * @param[in] index Index
     */
    void dataXYChanged(float *data, hsize_t index);
    /**
     * @brief Data XZ changed signal
     * @param[in] data Data
     * @param[in] index Index
     */
    void dataXZChanged(float *data, hsize_t index);
    /**
     * @brief Data YZ changed signal
     * @param[in] data Data
     * @param[in] index Index
     */
    void dataYZChanged(float *data, hsize_t index);

    /**
     * @brief Image XY changed signal
     * @param[in] image Image XY
     */
    void imageXYChanged(QImage image);
    /**
     * @brief Image XZ changed signal
     * @param[in] image Image XZ
     */
    void imageXZChanged(QImage image);
    /**
     * @brief Image YZ changed signal
     * @param[in] image Image YZ
     */
    void imageYZChanged(QImage image);

    /**
     * @brief Hovered point in image signal
     * @param[in] value Point value
     */
    void hoveredPointInImage(float value);

public slots:
    void setMinValue(float value);
    void setMaxValue(float value);
    void setColormap(ColorMap::Type colormap);
    void setColormap(int colormap);
    void setOpacity(const QVector<float> &opacity);
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

    /// Selected flag
    bool selectedFlag = false;
    /// HDF5 Dataset
    H5Helper::Dataset *dataset = nullptr;
    /// Dataset type
    H5OpenedFile::ObjectType type;

    /// Current XY loaded flag
    bool currentXYLoadedFlag = false;
    /// Current XZ loaded flag
    bool currentXZLoadedFlag = false;
    /// Current YZ loaded flag
    bool currentYZLoadedFlag = false;
    /// Current data 3D loaded flag
    bool currentData3DLoadedFlag = false;

    /// Load Data 3D Flag
    bool loadData3DFlag = false;
    /// Load Slices flag
    bool loadSlicesFlag = false;

    /// Slices index
    H5Helper::Vector4D index;

    /// XY thread
    H5ReadingThread *threadXY = nullptr;
    /// XZ thread
    H5ReadingThread *threadXZ = nullptr;
    /// YZ thread
    H5ReadingThread *threadYZ = nullptr;
    /// 3D thread
    H5ReadingThread *thread3D = nullptr;

    // Memory for loaded slices and 3D data
    /// Data XY
    float *dataXY = nullptr;
    /// Data XZ
    float *dataXZ = nullptr;
    /// Data YZ
    float *dataYZ = nullptr;
    /// Data 3D
    float *data3D = nullptr;
    //float *data3DLC = nullptr;
    //float *data3DCC = nullptr;

    // Dataset settings
    /// Minimal value
    float minValue = 0;
    /// Maximal value
    float maxValue = 0;
    /// Colormap
    ColorMap::Type colormap = ColorMap::JET;
    /// Opacity
    QVector<float> opacity;
    /// Minimal/maximal value trim flag
    bool minMaxValuesTrimFlag = false;

    // Original min/max values
    /// Original minimal value
    float originalMinValue = 0;
    /// Original maximal value
    float originalMaxValue = 0;

    // Datasets characteristics variables
    /// Original frame size
    H5Helper::Vector3D originalFrameSize;
    /// Frame size
    H5Helper::Vector3D frameSize;
    /// Original size
    H5Helper::Vector3D originalSize;
    /// Size
    H5Helper::Vector3D size;
    /// Original position
    H5Helper::Vector3D originalPosition;
    /// Position
    H5Helper::Vector3D position;
    /// Chunk size
    H5Helper::Vector chunkSize;
    /// Minimal value position
    H5Helper::Vector minValuePosition;
    /// Maximal value position
    H5Helper::Vector maxValuePosition;
    /// Number of steps
    hsize_t steps = 1;

    /// Compression helper
    H5Helper::CompressHelper *compressHelper = nullptr;
};

#endif // H5OBJECTTOVISUALIZE_H
