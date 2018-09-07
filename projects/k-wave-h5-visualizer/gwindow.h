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
 *              license. A copy of the LGPL license should have been received with this file.
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

#include <qvector3di.h>
#include <colormap.h>
#include <openglwindow.h>
#include <abstractobjectwidget.h>

/**
 * @brief The GWindow class represents wrapper for the 3D rendering window
 */
class GWindow : public OpenGLWindow, AbstractObjectWidget
{
    Q_OBJECT

public:
    GWindow(QMainWindow *qMainWindow = 0);
    ~GWindow();

    void initialize();
    void render();
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *);

    QImage getImage();

    /// Slice vertices
    static const GLfloat sliceVertices[];
    /// Slice indices
    static const GLint sliceElements[] ;
    /// Cube vertices
    static const GLfloat cubeVertices[];
    /// Cube elements
    static const GLint cubeElements[];

    bool isVolumeRenderingEnabled() const;

    bool areData3DLoaded() const;

public slots:
    void setViewFrame(bool value);
    void setSlicesCount(int value);
    void setViewVolumeRendering(bool value);
    void setViewXYSlice(bool value);
    void setViewXZSlice(bool value);
    void setViewYZSlice(bool value);

    void setTrim(bool value);
    void setOrthogonal(bool value);
    void setFillSpace(bool value);

    void setVolumeRenderingMode(int mode = 0);
    void setInterpolationMode(int mode = 0);

    void clear();

    void alignToXY();
    void alignToXZ();
    void alignToYZ();
    void alignToXYFromBack();
    void alignToXZFromBack();
    void alignToYZFromBack();

    void saveImage();

    void setObject(H5ObjectToVisualize *value);

    void set3DData(float *data3d);

private slots:
    void setOpacity(QVector<float> value = QVector<float>(5, 1));
    void setMinValue(float value);
    void setMaxValue(float value);
    void setColormap(ColorMap::Type colormap = ColorMap::JET);
    void setFrameSize(H5Helper::Vector3D size);
    void setFrameSize(QVector3DI size);
    void setDatasetSize(H5Helper::Vector3D size);
    void setDatasetSize(QVector3DI size);
    void setDatasetPosition(H5Helper::Vector3D position);
    void setDatasetPosition(QVector3DI position);

    void setXYSlice(float *data, hsize_t index);
    void setXZSlice(float *data, hsize_t index);
    void setYZSlice(float *data, hsize_t index);

private:
    void renderFrame();
    void renderBox();

    void unload3DTexture();
    void clearSlices();
    QPointF convertPointToOpenGLRelative(QPointF point);
    //float round(float number, float precision);

    QMainWindow *qMainWindow = 0;

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

    QVector3DI frameSize = QVector3DI(1, 1, 1);
    QVector3DI datasetSize = QVector3DI(1, 1, 1);
    QVector3DI datasetPosition = QVector3DI(0, 0, 0);

    int slicesCount = 500;

    QVector3D index = QVector3D(0.5, 0.5, 0.5);

    bool frame = true;
    bool trim = false;
    bool orthogonal = false;
    bool volumeRendering = false;
    bool sliceXY = false;
    bool sliceXZ = false;
    bool sliceYZ = false;
    bool fillSpace = false;

    int initialized;
};

#endif // GWINDOW_H
