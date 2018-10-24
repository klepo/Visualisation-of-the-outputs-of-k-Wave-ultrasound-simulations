/**
 * @file        gwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The header file with GWindow class declaration.
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

#ifndef GWINDOW_H
#define GWINDOW_H

#include <QFileDialog>

#include "qvector3di.h"
#include "colormap.h"
#include "openglwindow.h"
#include "abstractwidget.h"
#include "h5objecttovisualize.h"

/**
 * @brief The GWindow class represents wrapper for the 3D rendering window
 */
class GWindow : public OpenGLWindow, AbstractWidget
{
    Q_OBJECT

public:
    GWindow(QWidget *parent);
    ~GWindow();

    /// Slice vertices
    static const GLfloat sliceVertices[];
    /// Slice indices
    static const GLint sliceElements[] ;
    /// Cube vertices
    static const GLfloat cubeVertices[];
    /// Cube elements
    static const GLint cubeElements[];

    QImage getImage();

signals:
    void viewVolumeRenderingChanged(bool value);

public slots:
    void setViewVolumeRendering(bool value);
    void setViewXYSlice(bool value);
    void setViewXZSlice(bool value);
    void setViewYZSlice(bool value);
    void setViewFrame(bool value);
    void setFillSpace(bool value);
    void setOrthogonal(bool value);
    void alignToXY();
    void alignToXZ();
    void alignToYZ();
    void alignToXYFromBack();
    void alignToXZFromBack();
    void alignToYZFromBack();
    void setSlicesCount(int value);
    void setVolumeRenderingMode(int mode = 0);
    void setInterpolationMode(int mode = 0);

    void setObject(H5ObjectToVisualize *object);
    void clear();

    void saveImage(); // TODO move outside

protected:
    void initialize();
    void render();
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void setCompressRendering(bool value);
    void setMinValue(float value = 0.0f);
    void setMaxValue(float value = 0.0f);
    void setColormap(ColorMap::Type colormap = ColorMap::JET);
    void setOpacity(QVector<float> value = QVector<float>(1, 1));
    void setTrim(bool value = false);

    void setFrameSize(H5Helper::Vector3D size);
    void setFrameSize(QVector3DI size = QVector3DI(1, 1, 1));
    void setDatasetSize(H5Helper::Vector3D size);
    void setDatasetSize(QVector3DI size = QVector3DI(1, 1, 1));
    void setDatasetPosition(H5Helper::Vector3D position);
    void setDatasetPosition(QVector3DI position = QVector3DI(0, 0, 0));

    void setXYSlice(float *data = nullptr, hsize_t sliceIndex = 0);
    void setXZSlice(float *data = nullptr, hsize_t sliceIndex = 0);
    void setYZSlice(float *data = nullptr, hsize_t sliceIndex = 0);
    void set3DData(float *data = nullptr);
    void set3DCompressData(float *dataLC = nullptr, float *dataCC = nullptr, hsize_t localStep = 0);

private:
    Q_DISABLE_COPY(GWindow)

    void renderBox(bool frame = false);
    void unload3DTexture();
    void unload3DCompressTexture();
    void unloadSlicesTextures();
    QPointF convertPointToOpenGLRelative(QPointF point);

    QWidget *parent = nullptr;
    H5Helper::CompressHelper *compressHelper = nullptr;

    QOpenGLShaderProgram *program = nullptr;

    GLint aPosition;

    GLint uMatrix;
    GLint uSliceMatrix;

    GLint uVolumeTexture;
    GLint uVolumeTextureLC;
    GLint uVolumeTextureCC;
    GLint uTextureBE;
    GLint uTextureBE_1;
    GLint uColormapTexture;
    GLint uOpacityTexture;
    GLint uSliceTexture;
    GLint uBoxBackSampler;
    GLint uBoxFrontSampler;

    GLint uStepLocal2;
    GLint uHarmonics;
    GLint uBSize2;

    GLint uFrame;
    GLint uSlices;
    GLint uXYBorder;
    GLint uXZBorder;
    GLint uYZBorder;
    GLint uVolumeRenderingBox;
    GLint uVolumeRendering;
    GLint uVolumeCompressRendering;
    GLint uTrim;
    GLint uSteps;
    GLint uFrameColor;
    GLint uWidth;
    GLint uHeight;
    GLint uMin;
    GLint uMax;
    GLint uMode;

    QOpenGLBuffer iboSliceElements;
    QOpenGLBuffer iboCubeElements;
    QOpenGLBuffer vboSliceVertices;
    QOpenGLBuffer vboCubeVertices;

    GLuint vao;
    GLuint texture;
    GLuint textureLC;
    GLuint textureCC;
    GLuint textureBE;
    GLuint textureBE_1;
    GLuint textureBufferBE;
    GLuint textureBufferBE_1;
    GLuint textureFboBack;
    GLuint textureFboFront;
    GLuint colormapTexture;
    GLuint opacityTexture;
    GLuint textureXY;
    GLuint textureXZ;
    GLuint textureYZ;
    GLuint fbo;
    GLuint rbo;

    QMatrix4x4 rotateXMatrix;
    QMatrix4x4 rotateYMatrix;
    float zoom = 1.0f;
    QVector3D position;

    QVector3DI frameSize = QVector3DI(1, 1, 1);
    QVector3DI datasetSize = QVector3DI(1, 1, 1);
    QVector3DI datasetPosition = QVector3DI(0, 0, 0);
    QVector3D sliceIndex = QVector3D(0.5, 0.5, 0.5);

    bool trim = false;
    int slicesCount = 500;
    int vRMode = 0;

    bool volumeRendering = false;
    bool sliceXY = false;
    bool sliceXZ = false;
    bool sliceYZ = false;
    bool frame = true;
    bool fillSpace = false;
    bool orthogonal = false;

    int intMode = GL_LINEAR;

    bool initialized = false;
};

#endif // GWINDOW_H
