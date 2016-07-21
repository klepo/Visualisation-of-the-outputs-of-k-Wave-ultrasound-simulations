/*
 * @file        gwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The header file with GWindow class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#ifndef GWINDOW_H
#define GWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "openglwindow.h"
#include "hdf5readingthread.h"

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#ifdef HAVE_OPENCV_CONTRIB
    #include <opencv2/contrib/contrib.hpp>
#endif

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

    HDF5ReadingThread *getThread();
    bool isTexture3DInitialized();

signals:
    void loaded(std::string datasetName);

public slots:
    void setAlpha(int);
    void setRed(int);
    void setGreen(int);
    void setBlue(int);
    void setAlpha(double);
    void setRed(double);
    void setGreen(double);
    void setBlue(double);

    void setViewFrame(bool);
    void setSlicesCount(int);
    void setViewVR(bool);
    void setViewXYSlice(bool);
    void setViewXZSlice(bool);
    void setViewYZSlice(bool);

    void setTrim(bool);

    void setMainSize(unsigned int depth, unsigned int height, unsigned int width);
    void setSize(unsigned int depth, unsigned int height, unsigned int width);
    void setPosition(unsigned int posZ, unsigned int posY, unsigned int posX);

    void load3DTexture(HDF5Helper::HDF5Dataset *dataset);
    void changeColormap(int colormap = cv::COLORMAP_JET);
    void changeMinValue(float value);
    void changeMaxValue(float value);

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
    void setLoaded(Request *r);

private:
    void renderFrame();
    void unload3DTexture();
    void clearSlices();
    QPointF convertToOpenGLRelative(QPointF point);
    float round(float number, float precision);

    QMainWindow *qMainWindow;

    GLuint m_uFrame;

    GLuint m_uFrameColor;
    GLuint m_aPosition;
    GLuint m_aColor;
    GLuint m_aTextureCoord;
    GLuint m_uMatrix;
    GLuint m_uScaleMatrix;
    GLuint m_uScalelMatrix;
    GLuint m_uSampler;

    GLuint m_uHeight;
    GLuint m_uWidth;
    GLuint m_uDepth;

    //GLuint m_uPosX;
    //GLuint m_uPosY;
    //GLuint m_uPosZ;
    GLuint m_uXMax;
    GLuint m_uXMin;
    GLuint m_uYMax;
    GLuint m_uYMin;
    GLuint m_uZMax;
    GLuint m_uZMin;

    GLuint m_uAlpha;
    GLuint m_uRed;
    GLuint m_uGreen;
    GLuint m_uBlue;

    int m_frame;

    GLuint m_uSlices;
    GLuint m_uSliceSampler;

    QOpenGLShaderProgram *m_program;

    QMatrix4x4 rotateXMatrix;
    QMatrix4x4 rotateYMatrix;
    float zoom;
    QVector3D position;

    int count;
    bool frame;

    float alpha;
    float red;
    float green;
    float blue;

    GLuint iboPlaneElements;
    GLuint iboCubeElements;
    QOpenGLBuffer vboPlaneVertices;
    QOpenGLBuffer vboCubeVertices;
    QOpenGLBuffer vboSliceXYVertices;
    QOpenGLBuffer vboSliceXZVertices;
    QOpenGLBuffer vboSliceYZVertices;
    QOpenGLBuffer vboSliceTexCoords;

    GLuint vao;
    GLuint texture;

    unsigned int imageDepth;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int origImageDepth;
    unsigned int origImageWidth;
    unsigned int origImageHeight;

    unsigned int fullDepth;
    unsigned int fullWidth;
    unsigned int fullHeight;

    unsigned int posZ;
    unsigned int posY;
    unsigned int posX;

    float imageHeightDepthRatio;
    float imageWidthDepthRatio;

    QMutex mutex, textureMutex;
    HDF5ReadingThread *thread;

    float minG, maxG;
    int colormap;

    GLuint m_uColormapSampler;
    GLuint colormapTexture;

    GLuint m_uMin;
    GLuint m_uMax;

    GLuint m_uTrim;
    bool trim;

    bool texture3DInitialized;

    bool volumeRendering;
    GLuint m_uVolumeRendering;

    bool sliceXY;
    bool sliceXZ;
    bool sliceYZ;

    GLuint m_uXYBorder;
    GLuint m_uXZBorder;
    GLuint m_uYZBorder;

    GLuint textureXY;
    GLuint textureXZ;
    GLuint textureYZ;

    float xYIndex;
    float xZIndex;
    float yZIndex;

    std::string datasetName;
    HDF5Helper::HDF5Dataset *selectedDataset;

    bool flagSave;
    QString fileName;

    int initialized;
};

#endif // GWINDOW_H