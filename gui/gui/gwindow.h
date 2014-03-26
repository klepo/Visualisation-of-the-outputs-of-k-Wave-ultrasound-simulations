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

    HDF5ReadingThread *getThread();

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
    void setViewVR(bool);
    void setViewXYSlice(bool);
    void setViewXZSlice(bool);
    void setViewYZSlice(bool);

    void load3DTexture(HDF5File::HDF5Dataset *dataset, float min, float max, int colormap = cv::COLORMAP_JET);
    void changeColormap(int colormap = cv::COLORMAP_JET);
    void changeMinValue(float value);
    void changeMaxValue(float value);

    void setXYSlice(float *data, unsigned int width, unsigned int height, float index);
    void setXZSlice(float *data, unsigned int width, unsigned int height, float index);
    void setYZSlice(float *data, unsigned int width, unsigned int height, float index);

    void saveImage(QString fileName);

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

    GLuint m_uColormapSampler;
    GLuint colormapTexture;

    GLuint m_uMin;
    GLuint m_uMax;

    bool texture3DInitialized;

    bool volumeRendering;

    bool sliceXY;
    bool sliceXZ;
    bool sliceYZ;

    GLuint m_uSlices;
    GLuint m_uSliceSampler;
    GLuint textureXY;
    GLuint textureXZ;
    GLuint textureYZ;

    float xYIndex;
    float xZIndex;
    float yZIndex;

    std::string datasetName;

    bool flagSave;
    QString fileName;
};

#endif // GWINDOW_H
