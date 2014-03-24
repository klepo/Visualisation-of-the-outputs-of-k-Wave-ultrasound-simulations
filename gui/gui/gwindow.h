#ifndef GWINDOW_H
#define GWINDOW_H

#include "openglwindow.h"
#include "hdf5readingthread.h"
#include <opencv2/opencv.hpp>

#include <QtCore/qmath.h>
#include <QtGui/QOpenGLShaderProgram>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>


class GWindow : public OpenGLWindow
{
    Q_OBJECT

public:
    GWindow();
    ~GWindow();

    void initialize();
    void render();

    bool event(QEvent *event);

public slots:
    void setAlpha(int);
    void setRed(int);
    void setGreen(int);
    void setBlue(int);
    void setAlpha(double);
    void setRed(double);
    void setGreen(double);
    void setBlue(double);

    void load3DTexture(HDF5File::HDF5Dataset *dataset, int colormap = cv::COLORMAP_JET);
    void changeColormap(int colormap = cv::COLORMAP_JET);
    void changeMinValue(float value);
    void changeMaxValue(float value);

private slots:
    void setLoaded(hsize_t i, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *data, float, float);

private:
    GLuint m_uFrame;
    GLuint m_aPosition;
    GLuint m_aColor;
    GLuint m_aTextureCoord;
    GLuint m_uMatrix;
    GLuint m_uScaleMatrix;
    GLuint m_uScalelMatrix;
    GLuint m_uHeightDepthRatio;
    GLuint m_uWidthDepthRatio;
    GLuint m_uSampler;

    GLuint m_uAlpha;
    GLuint m_uRed;
    GLuint m_uGreen;
    GLuint m_uBlue;

    QOpenGLShaderProgram *m_program;
    int m_frame;
    QMatrix4x4 rotateXMatrix;
    QMatrix4x4 rotateYMatrix;
    float zoom;
    int count;
    bool frame;

    float alpha;
    float red;
    float green;
    float blue;

    GLuint ibo_plane_elements;
    GLuint texture;

    unsigned int imageDepth;
    unsigned int imageWidth;
    unsigned int imageHeight;

    float imageHeightDepthRatio;
    float imageWidthDepthRatio;

    QMutex mutex, textureMutex;
    HDF5ReadingThread *thread;

    float minG, maxG;
    int colormap;

    GLint redC[256];
    GLint greenC[256];
    GLint blueC[256];

    GLuint m_uRedC;
    GLuint m_uGreenC;
    GLuint m_uBlueC;

    GLuint m_uColormapSampler;
    GLuint colormapTexture;

    GLuint m_uMin;
    GLuint m_uMax;

    bool texture3DInitialized;
};

#endif // GWINDOW_H
