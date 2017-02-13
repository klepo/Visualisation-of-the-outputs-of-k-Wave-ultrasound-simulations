/**
 * @file        gwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with GWindow class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef GWINDOW_H
#define GWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "qvector3di.h"
#include "colormap.h"
#include "openglwindow.h"
#include "hdf5readingthread.h"

#include <hdf5helper.h>

class GWindow : public OpenGLWindow
{
    Q_OBJECT

public:
    GWindow(QMainWindow *qMainWindow = 0);
    ~GWindow();

    void initialize();
    void render();
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *event);

    HDF5ReadingThread *getThread();
    bool isTexture3DInitialized();

    QImage getImage();

    static const GLfloat sliceVertices[];
    static const GLint sliceElements[] ;
    static const GLfloat cubeVertices[];
    static const GLint cubeElements[];

signals:
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

    void setMainSize(HDF5Helper::HDF5Vector3D size);
    void setSize(HDF5Helper::HDF5Vector3D size);
    void setPosition(HDF5Helper::HDF5Vector3D position);

    void load3DTexture(HDF5Helper::HDF5Dataset *dataset, hsize_t index);
    void changeColormap(ColorMap::Type colormap = ColorMap::JET);
    void changeOpacity(QVector<float> opacity = QVector<float>(5, 1));
    void changeMinValue(float value);
    void changeMaxValue(float value);
    void changeMode(int mode = 0);

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
    HDF5Helper::HDF5Dataset *selectedDataset = 0;
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
