/**
 * @file        gwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              20 February  2019 (updated)
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
    /**
     * @brief View volume rendering changed signal
     * @param[in] value View volume rendering flag
     */
    void viewVolumeRenderingChanged(bool value);
    /**
     * @brief View XY slice changed signal
     * @param[in] value View XY slice flag
     */
    void viewXYSliceChanged(bool value);
    /**
     * @brief View XZ slice changed signal
     * @param[in] value XZ slice flag
     */
    void viewXZSliceChanged(bool value);
    /**
     * @brief View YZ slice changed signal
     * @param[in] value View YZ slice flag
     */
    void viewYZSliceChanged(bool value);

    /**
     * @brief Step rendered signal
     */
    void stepRendered(qint64, hsize_t);

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
    void setOpacity(QVector<float> opacity = QVector<float>(1, 1));
    void setTrim(bool value = false);

    void setFrameSize(H5Helper::Vector3D size);
    void setFrameSize(QVector3DI size = QVector3DI(1, 1, 1));
    void setDatasetSize(H5Helper::Vector3D size);
    void setDatasetSize(QVector3DI size = QVector3DI(1, 1, 1));
    void setDatasetPosition(H5Helper::Vector3D position);
    void setDatasetPosition(QVector3DI position = QVector3DI(0, 0, 0));

    void setXYSlice(hsize_t step, const float *data = nullptr, hsize_t sliceIndex = 0);
    void setXZSlice(hsize_t step, const float *data = nullptr, hsize_t sliceIndex = 0);
    void setYZSlice(hsize_t step, const float *data = nullptr, hsize_t sliceIndex = 0);
    void set3DData(hsize_t step, const float *data = nullptr);
    void set3DCompressData(hsize_t step, const float *dataLC = nullptr, const float *dataCC = nullptr);
    void set3DCompressLocalStep(hsize_t step, hsize_t localStep);

    void gWindowRendered(qint64 elapsedNs);

private:
    Q_DISABLE_COPY(GWindow)

    void renderBox(bool frame = false);
    void unload3DTexture();
    void unload3DCompressTexture();
    void unloadSlicesTextures();
    QPointF convertPointToOpenGLRelative(QPointF point) const;

    /// Parent
    QWidget *parent = nullptr;
    /// Compress helper
    const H5Helper::CompressHelper *compressHelper = nullptr;

    /// Shader program
    QOpenGLShaderProgram *program = nullptr;

    /// Position attribute handle
    GLint aPosition;

    // Uniforms
    /// Main transformation matrix uniform id
    GLint uMatrix;
    /// Slice matrix uniform id
    GLint uSliceMatrix;

    /// Volume texture uniform id
    GLint uVolumeTexture;
    /// Volume texture for last compression coefficients uniform id
    GLint uVolumeTextureLC;
    /// Volume texture for last current coefficients uniform id
    GLint uVolumeTextureCC;
    /// Complex exponencial window basis texture uniform id
    GLint uTextureBE;
    /// Inverted complex exponencial window basis texture uniform id
    GLint uTextureBE_1;
    /// Colormap texture uniform id
    GLint uColormapTexture;
    /// Opacity texture uniform id
    GLint uOpacityTexture;
    /// Slice texture uniform id
    GLint uSliceTexture;
    /// Back box texture uniform id
    GLint uBoxBackSampler;
    /// Front box texture uniform id
    GLint uBoxFrontSampler;

    /// Local step times 2 uniform id
    GLint uStepLocal2;
    /// Number of harmonics uniform id
    GLint uHarmonics;
    /// Base size times 2 uniform id
    GLint uBSize2;

    /// Render frame uniform id
    GLint uFrame;
    /// Render slices uniform id
    GLint uSlices;
    /// Render XY border uniform id
    GLint uXYBorder;
    /// Render XZ border uniform id
    GLint uXZBorder;
    /// Render YZ border uniform id
    GLint uYZBorder;
    /// Render volume rendering box uniform id
    GLint uVolumeRenderingBox;
    /// Render volume rendering uniform id
    GLint uVolumeRendering;
    /// Render compress rendering uniform id
    GLint uVolumeCompressRendering;
    /// Trim uniform id
    GLint uTrim;
    /// Number of steps uniform id
    GLint uSteps;
    /// Frame color uniform id
    GLint uFrameColor;
    /// Viewport width uniform id
    GLint uWidth;
    /// Viewport height uniform id
    GLint uHeight;
    /// Minimum uniform id
    GLint uMin;
    /// Maximum uniform id
    GLint uMax;
    /// Volume rendering mode uniform id
    GLint uMode;

    /// Interpolation mode 0 - linear/1 - nearest
    GLint uInt;

    /// Slice elements buffer
    QOpenGLBuffer iboSliceElements;
    /// Cube elements buffer
    QOpenGLBuffer iboCubeElements;
    /// Slice vertices buffer
    QOpenGLBuffer vboSliceVertices;
    /// Cube vertices buffer
    QOpenGLBuffer vboCubeVertices;

    // OpenGL handles
    /// Vertex array object handle
    GLuint vao;
    /// Volume texture handle
    GLuint texture;
    /// Volume texture for last compression coefficients handle
    GLuint textureLC;
    /// Volume texture for last current coefficients handle
    GLuint textureCC;
    /// Complex exponencial window basis texture handle
    GLuint textureBE;
    /// Inverted complex exponencial window basis texture handle
    GLuint textureBE_1;
    /// Complex exponencial window basis texture buffer handle
    GLuint textureBufferBE;
    /// Inverted complex exponencial window basis texture buffer handle
    GLuint textureBufferBE_1;
    /// Colormap texture handle
    GLuint colormapTexture;
    /// Opacity texture handle
    GLuint opacityTexture;
    /// XY Slice texture handle
    GLuint textureXY;
    /// XZ Slice texture handle
    GLuint textureXZ;
    /// YZ Slice texture handle
    GLuint textureYZ;
    /// Back box texture handle
    GLuint textureFboBack;
    /// Front box texture handle
    GLuint textureFboFront;
    /// Framebuffer object handle
    GLuint fbo;
    /// Renderbuffer object handle
    GLuint rbo;

    // Scene manipulation
    /// Rotate x axis matrix
    QMatrix4x4 rotateXMatrix;
    /// Rotate y axis matrix
    QMatrix4x4 rotateYMatrix;
    /// Zoom
    float zoom = 1.0f;
    /// Position
    QVector3D position;

    /// Frame size
    QVector3DI frameSize = QVector3DI(1, 1, 1);
    /// Dataset size
    QVector3DI datasetSize = QVector3DI(1, 1, 1);
    /// Dataset position (sensor mask position)
    QVector3DI datasetPosition = QVector3DI(0, 0, 0);
    /// Slice index
    QVector3D sliceIndex = QVector3D(0.5, 0.5, 0.5);
    /// Current step
    hsize_t currentStep = 0;

    /// Trim flag
    bool trim = false;
    /// Number of steps for volume ray casting
    int slicesCount = 500;
    /// Volume rendering mode
    int vRMode = 0;
    /// Iterpolation mode
    int intMode = GL_LINEAR;

    /// View volume rendering flag
    bool volumeRendering = false;
    /// View slice XY flag
    bool sliceXY = false;
    /// View slice XZ flag
    bool sliceXZ = false;
    /// View slice YZ flag
    bool sliceYZ = false;
    /// View frame flag
    bool frame = true;
    /// Fill space (by sensor mask) flag
    bool fillSpace = false;
    /// View orthogonal projection flag
    bool orthogonal = false;

    /// Name of 3D scene for saving an image
    QString sceneName = "no_name";

    /// Initialization flag
    bool initialized = false;
};

#endif // GWINDOW_H
