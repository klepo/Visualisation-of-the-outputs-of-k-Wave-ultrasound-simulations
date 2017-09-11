/**
 * @file        gwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with GWindow class declaration.
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

#ifndef GWINDOW_H
#define GWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>

#include "qvector3di.h"
#include "colormap.h"
#include "openglwindow.h"
#include "hdf5readingthread.h"

#include <hdf5helper.h>

/**
 * @brief The GWindow class represents wrapper for the 3D rendering window
 */
class GWindow : public OpenGLWindow
{
    Q_OBJECT

public:
    GWindow(QMainWindow *qMainWindow = 0);
    ~GWindow();

    void initialize();
    void render();
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *);

    HDF5ReadingThread *getThread();
    bool isTexture3DInitialized();

    QImage getImage();

    /// Slice vertices
    static const GLfloat sliceVertices[];
    /// Slice indices
    static const GLint sliceElements[] ;
    /// Cude vertices
    static const GLfloat cubeVertices[];
    /// Cube elements
    static const GLint cubeElements[];

signals:
    /**
     * @brief Data loaded signal
     * @param[in] datasetName Dataset name
     */
    void loaded(std::string datasetName);

public slots:
    void setViewFrame(bool);
    void setSlicesCount(int);
    void setViewVR(bool);
    void setViewXYSlice(bool);
    void setViewXZSlice(bool);
    void setViewYZSlice(bool);

    void setTrim(bool);
    void setOrthogonal(bool);

    void setMainSize(HDF5Helper::Vector3D size);
    void setSize(HDF5Helper::Vector3D size);
    void setPosition(HDF5Helper::Vector3D position);

    void load3DTexture(HDF5Helper::Dataset *dataset, hsize_t step);
    void changeColormap(ColorMap::Type colormap = ColorMap::JET);
    void changeOpacity(QVector<float> opacity = QVector<float>(5, 1));
    void changeMinValue(float value);
    void changeMaxValue(float value);
    void changeMode(int mode = 0);
    void changeInterpolation(int mode = 0);

    void clearData();
    void unloadDataset();

    void setXYSlice(float *data, unsigned int width, unsigned int height, float index);
    void setXZSlice(float *data, unsigned int width, unsigned int height, float index);
    void setYZSlice(float *data, unsigned int width, unsigned int height, float index);

    void alignToXY();
    void alignToXZ();
    void alignToYZ();
    void alignToXYFromBack();
    void alignToXZFromBack();
    void alignToYZFromBack();

    void saveImage(QString fileName);

private slots:
    void setLoaded(Request *request);

private:
    void renderFrame();
    void renderBox();

    void unload3DTexture();
    void clearSlices();
    QPointF convertPointToOpenGLRelative(QPointF point);
    float round(float number, float precision);

    QMainWindow *qMainWindow = 0;
    HDF5Helper::Dataset *selectedDataset = 0;
    HDF5ReadingThread *thread = 0;

    GLint uVolumeTexture;
    GLint uColormapTexture;
    GLint uOpacityTexture;
    GLint uSliceTexture;
    GLint uBoxBackSampler;
    GLint uBoxFrontSampler;

    GLint m_uFrame;
    GLint m_uSlices;
    GLint m_uXYBorder;
    GLint m_uXZBorder;
    GLint m_uYZBorder;
    GLint m_uVolumeRendering;
    GLint m_uVolumeRenderingBox;

    GLint m_uTrim;

    GLint m_uSteps;

    GLint m_uFrameColor;

    GLint m_uWidth;
    GLint m_uHeight;

    GLint m_uColor;

    GLint m_uMin;
    GLint m_uMax;

    GLint m_uMode;

    GLint m_aPosition;

    GLint m_uMatrix;
    GLint m_uSliceMatrix;

    QOpenGLShaderProgram *m_program;

    GLuint iboCubeElements;
    GLuint iboSliceElements;
    QOpenGLBuffer vboCubeVertices;
    QOpenGLBuffer vboSliceVertices;

    GLuint vao;
    GLuint texture;
    GLuint textureFboBack;
    GLuint textureFboFront;
    GLuint fbo;
    GLuint rbo;

    GLuint textureXY;
    GLuint textureXZ;
    GLuint textureYZ;
    GLuint colormapTexture;
    GLuint opacityTexture;

    QMatrix4x4 rotateXMatrix;
    QMatrix4x4 rotateYMatrix;
    float zoom = 1.0f;
    QVector3D position;

    QVector3DI imageSize;
    QVector3DI imageSizeOrig;
    QVector3DI fullSize;
    QVector3DI imagePosition;

    float minG = 0.0f;
    float maxG = 1.0f;

    //ColorMap::Type colormap = ColorMap::JET;
    int steps = 500;

    QVector3D index;

    bool frame = true;
    bool trim = false;
    bool orthogonal = false;
    bool volumeRendering = false;
    bool sliceXY = false;
    bool sliceXZ = false;
    bool sliceYZ = false;

    bool texture3DInitialized = false;

    int initialized;
};

#endif // GWINDOW_H
