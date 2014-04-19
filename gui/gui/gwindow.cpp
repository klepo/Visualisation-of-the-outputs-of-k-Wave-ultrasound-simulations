#include "gwindow.h"
#include "hdf5readingthread.h"

//#include <QOpenGLFunctions_3_3_Compatibility>

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <QtCore/qmath.h>
#include <QEvent>
#include <QKeyEvent>
#include <QMutex>
#include <QLabel>
#include <QFileDialog>
#include <QTime>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>

#include <opencv2/opencv.hpp>

GLfloat planeVertices[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
};

GLfloat cubeVertices[] = {
    // front
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    // back
    0.0, 0.0, 1.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 1.0,
};

GLfloat sliceXYVertices[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
};

GLfloat sliceXZVertices[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
};

GLfloat sliceYZVertices[] = {
    0.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 1.0,
    0.0, 0.0, 1.0,
};

GLfloat sliceTexCoords[] = {
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0,
};

GLint planeElements[] = {
    0, 1, 2,
    2, 3, 0,
};

GLint cubeElements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    4, 0, 3,
    3, 7, 4,
    // back
    5, 4, 7,
    7, 6, 5,
    // bottom
    1, 5, 6,
    6, 2, 1,
    // left
    2, 6, 7,
    7, 3, 2,
    // right
    1, 5, 4,
    4, 0, 1,
};

float round(float f,float pres)
{
    return (float) (floor(f*(1.0f/pres) + 0.5)/(1.0f/pres));
}

void checkGlError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (err == GL_INVALID_ENUM)
            qWarning() << "OpenGL error: GL_INVALID_ENUM " << err;
        else if (err == GL_INVALID_VALUE)
            qWarning() << "OpenGL error: GL_INVALID_VALUE " << err;
        else if (err == GL_INVALID_OPERATION)
            qWarning() << "OpenGL error: GL_INVALID_OPERATION " << err;
        else if (err == GL_STACK_OVERFLOW)
            qWarning() << "OpenGL error: GL_STACK_OVERFLOW " << err;
        else if (err == GL_STACK_UNDERFLOW)
            qWarning() << "OpenGL error: GL_STACK_UNDERFLOW " << err;
        else if (err == GL_OUT_OF_MEMORY)
            qWarning() << "OpenGL error: GL_OUT_OF_MEMORY " << err;
        else if (err == GL_TABLE_TOO_LARGE)
            qWarning() << "OpenGL error: GL_TABLE_TOO_LARGE " << err;
        else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
            qWarning() << "OpenGL error: GL_INVALID_FRAMEBUFFER_OPERATION " << err;
        else
            qWarning() << "OpenGL error: " << err;
    }
}

GWindow::GWindow()
    : m_program(0)
    , trim(0)
    , frame(true)
    , texture3DInitialized(false)
    , volumeRendering(false)
    , sliceXY(false)
    , sliceXZ(false)
    , sliceYZ(false)
    , initialized(false)
{
    xYIndex = 0;
    xZIndex = 0;
    yZIndex = 0;

    count = 50;
    alpha = 0.5f;
    red = 0.5f;
    green = 0.5f;
    blue = 0.5f;
    zoom = -15.0f;

    imageWidth = 1;
    imageHeight = 1;
    imageDepth = 1;

    fullWidth = 1;
    fullHeight = 1;
    fullDepth = 1;

    origImageWidth = imageWidth;
    origImageHeight = imageHeight;
    origImageDepth = imageDepth;

    posX = 0;
    posY = 0;
    posZ = 0;

    minG = 0;
    maxG = 0;

    colormap = cv::COLORMAP_JET;

    datasetName = "no_dataset";
    flagSave = false;
    flagSave2 = false;

    thread = new HDF5ReadingThread();
    connect(thread, SIGNAL(requestDone(Request *)), this, SLOT(setLoaded(Request *)));
}

GWindow::~GWindow()
{
    m_program->release();
    delete m_program;
    thread->clearRequests();
    //thread->clearDoneRequests();
    //QMetaObject::invokeMethod(thread, "stop");
    thread->wait();
    thread->deleteLater();
    //thread->deleteLater();
}

HDF5ReadingThread *GWindow::getThread()
{
    return thread;
}

void GWindow::initialize()
{
    emit setStatusMessage(QString("Slices: %1").arg(count));

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexShader.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentShader.frag");
    m_program->link();

    m_aPosition = m_program->attributeLocation("aPosition");
    m_aTextureCoord = m_program->attributeLocation("aTextureCoord");

    m_uFrame = m_program->uniformLocation("uFrame");

    m_uMatrix = m_program->uniformLocation("uMatrix");
    m_uScaleMatrix = m_program->uniformLocation("uScaleMatrix");
    m_uScalelMatrix = m_program->uniformLocation("uScalelMatrix");

    m_uHeight = m_program->uniformLocation("uHeight");
    m_uWidth = m_program->uniformLocation("uWidth");
    m_uDepth = m_program->uniformLocation("uDepth");

    //m_uPosX = m_program->uniformLocation("uPosX");
    //m_uPosY = m_program->uniformLocation("uPosY");
    //m_uPosZ = m_program->uniformLocation("uPosZ");

    m_uXMax = m_program->uniformLocation("uXMax");
    m_uYMax = m_program->uniformLocation("uYMax");
    m_uZMax = m_program->uniformLocation("uZMax");
    m_uXMin = m_program->uniformLocation("uXMin");
    m_uYMin = m_program->uniformLocation("uYMin");
    m_uZMin = m_program->uniformLocation("uZMin");

    m_uAlpha = m_program->uniformLocation("uAlpha");
    m_uRed = m_program->uniformLocation("uRed");
    m_uGreen = m_program->uniformLocation("uGreen");
    m_uBlue = m_program->uniformLocation("uBlue");

    m_uSampler = m_program->uniformLocation("uSampler");

    m_uColormapSampler = m_program->uniformLocation("uColormapSampler");

    m_uMin = m_program->uniformLocation("uMin");
    m_uMax = m_program->uniformLocation("uMax");

    m_uTrim = m_program->uniformLocation("uTrim");

    m_uSlices = m_program->uniformLocation("uSlices");

    m_uSliceSampler = m_program->uniformLocation("uSliceSampler");

    //glBindFragDataLocation(m_program->programId(), 0, "colorOut");

    glGenBuffers(1, &ibo_plane_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_plane_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeElements), planeElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ibo_cube_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeElements), cubeElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 3D texture
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_3D, texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_3D);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

    // 1D texture
    glEnable(GL_TEXTURE_1D);
    glGenTextures(1, &colormapTexture);

    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_1D);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // 2D textures
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureXY);

    glBindTexture(GL_TEXTURE_2D, textureXY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureXZ);

    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureYZ);

    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Settings
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor((float) 16 / 17, (float) 16 / 17, (float) 16 / 17, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
    //glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    /*cv::Mat colormapImage = cv::Mat::zeros(1, 256, CV_8UC1);
    for (unsigned int i = 0; i < 256; i++)
        colormapImage.data[i] = i;
    cv::applyColorMap(colormapImage, colormapImage, colormap);
    cvtColor(colormapImage, colormapImage, CV_BGR2RGB);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, colormapImage.data);*/
    changeColormap();

    rotateXMatrix.rotate(-30, -1, 0, 0);
    rotateYMatrix.rotate(-15, 0, -1, 0);

    m_program->bind();

    m_program->setUniformValue(m_uMin, 0.0f);
    m_program->setUniformValue(m_uMax, 0.0f);

    /*glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);*/
    m_program->setUniformValue(m_uColormapSampler, 1);
    m_program->setUniformValue(m_uSliceSampler, 2);
    m_program->setUniformValue(m_uSampler, 0);

    m_program->setUniformValue(m_uFrame, false);
    m_program->setUniformValue(m_uSlices, false);

    m_program->release();

    initialized = true;
    renderLater();
}

bool GWindow::isTexture3DInitialized()
{
    return texture3DInitialized;
}

void GWindow::setMainSize(unsigned int depth, unsigned int height, unsigned int width)
{
    fullWidth = width;
    fullHeight = height;
    fullDepth = depth;
}

void GWindow::setSize(unsigned int depth, unsigned int height, unsigned int width)
{
    imageDepth = depth;
    imageHeight = height;
    imageWidth = width;
}

void GWindow::setPosition(unsigned int posZ, unsigned int posY, unsigned int posX)
{
    this->posZ = posZ;
    this->posY = posY;
    this->posX = posX;
}

void GWindow::load3DTexture(HDF5File::HDF5Dataset *dataset)
{
    thread->clearRequests();
    thread->wait();
    //thread->clearDoneRequests();

    datasetName = dataset->getName();
    selectedDataset = dataset;

    texture3DInitialized = false;

    imageWidth = dataset->getDims()[2];
    imageHeight = dataset->getDims()[1];
    imageDepth = dataset->getDims()[0];
    origImageWidth = imageWidth;
    origImageHeight = imageHeight;
    origImageDepth = imageDepth;

    PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
    glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageWidth, imageHeight, imageDepth, 0, GL_RED, GL_FLOAT, NULL);

    thread->createRequest(selectedDataset);
    thread->start();
}

void GWindow::unload3DTexture()
{
    PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
    glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageWidth, imageHeight, imageDepth, 0, GL_RED, GL_FLOAT, NULL);
}

void GWindow::clearData()
{
    if (initialized) {
        imageWidth = 1;
        imageHeight = 1;
        imageDepth = 1;

        fullWidth = 1;
        fullHeight = 1;
        fullDepth = 1;

        origImageWidth = imageWidth;
        origImageHeight = imageHeight;
        origImageDepth = imageDepth;

        posX = 0;
        posY = 0;
        posZ = 0;

        changeMinValue(0.0f);
        changeMaxValue(0.0f);
        clearSlices();
        unload3DTexture();
        renderLater();
    }
}

void GWindow::setLoaded(Request *r)
{
    //textureMutex.lock();
    texture3DInitialized = false;
    PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) wglGetProcAddress("glTexSubImage3D");

    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, r->xO, r->yO, r->zO, r->xC, r->yC, r->zC, GL_RED, GL_FLOAT, r->data);

    if (r->zO + r->zC == imageDepth) {
        //thread->deleteLater();
        texture3DInitialized = true;
        changeColormap(colormap);
        renderLater();
        //emit partLoaded((int) ((double) (i + 1) / imageDepth * 100));
        emit loaded(datasetName);
    }

    thread->deleteDoneRequest(r);
}

void GWindow::setXYSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setXYSlice";
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    xYIndex = index;
    renderLater();
}

void GWindow::setXZSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setXZSlice";
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    xZIndex = index;
    renderLater();
}

void GWindow::setYZSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setYZSlice";
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    yZIndex = index;
    renderLater();
}

void GWindow::clearSlices()
{
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
}

void GWindow::changeColormap(int _colormap)
{
    colormap = _colormap;
    glBindTexture(GL_TEXTURE_1D, colormapTexture);

    cv::Mat colormapImage = cv::Mat::zeros(1, 256, CV_8UC1);
    for (unsigned int i = 0; i < 256; i++)
        colormapImage.data[i] = i;
    cv::applyColorMap(colormapImage, colormapImage, colormap);
    cvtColor(colormapImage, colormapImage, CV_BGR2RGB);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, colormapImage.data);

    m_program->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    m_program->setUniformValue(m_uColormapSampler, 1);

    m_program->release();

    renderLater();
}

void GWindow::changeMinValue(float value)
{
    minG = value;
    m_program->bind();
    m_program->setUniformValue(m_uMin, minG);
    m_program->release();
    renderLater();
}

void GWindow::changeMaxValue(float value)
{
    maxG = value;
    m_program->bind();
    m_program->setUniformValue(m_uMax, maxG);
    m_program->release();
    renderLater();
}

void GWindow::renderFrame()
{
    glEnableVertexAttribArray(m_aPosition);
    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, cubeVertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    glDrawElements(GL_LINE_LOOP,  sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(m_aPosition);
}

void GWindow::render()
{
    checkGlError();

    int actualCount = count;

    if (leftButton) {
        rotateXMatrix.rotate((float) (lastPos.y() - currentPos.y()) / 2.0f, -1, 0, 0);
        rotateYMatrix.rotate((float) (lastPos.x() - currentPos.x()) / 2.0f, 0, -1, 0);
        actualCount = 30;
    }

    if (rightButton) {
        position.setX(position.x() + (float) (lastPos.x() - currentPos.x()) / 100.0f * zoom / 10);
        position.setY(position.y() - (float) (lastPos.y() - currentPos.y()) / 100.0f * zoom / 10);
        actualCount = 30;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, width(), height());

    m_program->bind();

    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.perspective(45, (float) width() / (float) height(), 0.1f, 1000.0f);

    QMatrix4x4 zoomMatrix;

    zoom += (float) wheelDelta / 100.0f;
    if (zoom > 0.0f) zoom = 0.0f;
    zoomMatrix.translate(0, 0, -qExp(qFabs(zoom)/20));
    zoomMatrix.translate(position);
    QMatrix4x4 matrix;
    matrix = perspectiveMatrix * zoomMatrix * rotateXMatrix * rotateYMatrix;

    QVector3D vecScale((float) imageWidth, (float) imageHeight, (float) imageDepth);
    QVector3D vecFullScale((float) fullWidth, (float) fullHeight, (float) fullDepth);
    float max = qMax(vecScale.x(), qMax(vecScale.y(), vecScale.z()));
    float fullMax = qMax(vecFullScale.x(), qMax(vecFullScale.y(), vecFullScale.z()));
    vecFullScale = vecFullScale / fullMax; // longest size is 1.0f

    QVector3D vecScale0 = vecScale / max;
    m_program->setUniformValue(m_uWidth, (float) vecScale0.x() / 2.0f);
    m_program->setUniformValue(m_uHeight, (float) vecScale0.y() / 2.0f);
    m_program->setUniformValue(m_uDepth, (float) vecScale0.z() / 2.0f);

    m_program->setUniformValue(m_uXMax, 0.5f + ((float) vecScale0.x() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uXMin, 0.5f - ((float) vecScale0.x() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uYMax, 0.5f + ((float) vecScale0.y() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uYMin, 0.5f - ((float) vecScale0.y() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uZMax, 0.5f + ((float) vecScale0.z() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uZMin, 0.5f - ((float) vecScale0.z() / 2.0f) / 2.0f);

    vecScale = vecScale / fullMax;

    matrix.translate(-vecFullScale.x() / 2.0f, -vecFullScale.y() / 2.0f, -vecFullScale.z() / 2.0f);

    m_program->setUniformValue(m_uMatrix, matrix);

    // Trim values
    if (trim) {
        m_program->setUniformValue(m_uTrim, true);
    } else {
        m_program->setUniformValue(m_uTrim, false);
    }

    // Frame
    if (frame) {
        mutex.lock();
        m_program->setUniformValue(m_uFrame, true);

        // Recomputing window coords...
        /*QMatrix4x4 sMmatrix0;
        m_program->setUniformValue(m_uMatrix, matrix);
        m_program->setUniformValue(m_uScaleMatrix, sMmatrix0);

        QMatrix4x4 invMatrix = matrix.inverted();

        QVector4D point0(0.5f, 0.5f, 0.0f, 1.0f);
        point0 = invMatrix * point0.normalized();
        QVector3D point03D = point0.toVector3DAffine();
        QVector4D point1(0.5f, -0.5f, 0.0f, 1.0f);
        point1 = invMatrix * point1.normalized();
        QVector3D point13D = point1.toVector3DAffine();
        QVector4D point2(-0.5f, -0.5f, 0.0f, 1.0f);
        point2 = invMatrix * point2.normalized();
        QVector3D point23D = point2.toVector3DAffine();
        QVector4D point3(-0.5f, 0.5f, 0.0f, 1.0f);
        point3 = invMatrix * point3.normalized();
        QVector3D point33D = point3.toVector3DAffine();
        qDebug() << point03D.x() << " " << point03D.y() << " " << point03D.z();

        glBegin(GL_QUADS);
        glVertex3f(point03D.x(), point03D.y(), point03D.z());
        glVertex3f(point13D.x(), point13D.y(), point13D.z());
        glVertex3f(point23D.x(), point23D.y(), point23D.z());
        glVertex3f(point33D.x(), point33D.y(), point33D.z());
        //glVertex3f(1, -1, 1);
        //glVertex3f(-1, -1, 1);
        //glVertex3f(-1, -1, -1);
        //glVertex3f(1, -1, -1);
        glEnd();*/

        QMatrix4x4 sMmatrix;
        // Smaller frame
        sMmatrix.translate((float) posX / fullMax, (float) posY / fullMax, (float) posZ / fullMax);
        sMmatrix.scale(vecScale);
        m_program->setUniformValue(m_uScaleMatrix, sMmatrix);
        renderFrame();

        // Big frame
        if (imageWidth != fullWidth || imageHeight != fullHeight || imageDepth || fullDepth) {
            QMatrix4x4 sMmatrix;
            sMmatrix.scale(vecFullScale);
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix);
            renderFrame();
        }


        m_program->setUniformValue(m_uFrame, false);
        mutex.unlock();
    }

    // Slices
    if (sliceXY || sliceXZ || sliceYZ) {
        mutex.lock();
        m_program->setUniformValue(m_uSlices, true);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        //glDepthMask(GL_FALSE);

        QMatrix4x4 sMmatrix;
        sMmatrix.translate((float) posX / fullMax, (float) posY / fullMax, (float) posZ / fullMax);
        sMmatrix.scale(vecScale);

        QMatrix4x4 xYMmatrix;
        xYMmatrix.translate(0, 0, xYIndex);
        QMatrix4x4 xZMmatrix;
        xZMmatrix.translate(0, xZIndex, 0);
        QMatrix4x4 yZMmatrix;
        yZMmatrix.translate(yZIndex, 0, 0);

        glEnableVertexAttribArray(m_aTextureCoord);
        glVertexAttribPointer(m_aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, sliceTexCoords);

        glEnableVertexAttribArray(m_aPosition);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_plane_elements);

        if (sliceXY) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xYMmatrix);
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, sliceXYVertices);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureXY);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
        }

        if (sliceXZ) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xZMmatrix);
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, sliceXZVertices);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureXZ);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
        }

        if (sliceYZ) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * yZMmatrix);
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, sliceYZVertices);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureYZ);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(m_aTextureCoord);
        glDisableVertexAttribArray(m_aPosition);

        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        //glDepthMask(GL_TRUE);

        m_program->setUniformValue(m_uSlices, false);
        mutex.unlock();
    }

    // Volume rendering
    if (volumeRendering) {
        mutex.lock();
        glDepthMask(GL_FALSE);
        //glDisable(GL_CULL_FACE);

        //textureMutex.lock();
        //if (texture3DInitialized) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, texture);
        //}
        //textureMutex.unlock();

        // Recompute alpha and set alpha + colors
        m_program->setUniformValue(m_uAlpha, 1.0f - pow(1.0f - alpha, 10.0f / (float) actualCount));
        m_program->setUniformValue(m_uRed, 1.0f - pow(1.0f - red, 10.0f / (float) actualCount));
        m_program->setUniformValue(m_uGreen, 1.0f - pow(1.0f - green, 10.0f / (float) actualCount));
        m_program->setUniformValue(m_uBlue, 1.0f - pow(1.0f - blue, 10.0f / (float) actualCount));

        glEnableVertexAttribArray(m_aPosition);
        glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, planeVertices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_plane_elements);

        QVector4D vec((float) posX / fullMax, (float) posY / fullMax, (float) posZ / fullMax, 1.0f);
        vec += QVector4D(-vecFullScale.x() / 2.0f, -vecFullScale.y() / 2.0f, -vecFullScale.z() / 2.0f, 0.0f);
        vec -= QVector4D(-vecScale.x() / 2.0f, -vecScale.y() / 2.0f, -vecScale.z() / 2.0f, 0.0f);
        vec = rotateXMatrix * rotateYMatrix * vec;
        QMatrix4x4 sMmatrix;
        sMmatrix.translate(vec.toVector3DAffine());
        sMmatrix.scale((float) max / fullMax * 2.0f, (float) max / fullMax * 2.0f, (float) max / fullMax * 2.0f);
        sMmatrix.translate(-0.5f, -0.5f, -0.5f);

        QMatrix4x4 t1lMmatrix;
        t1lMmatrix.translate(0.5f, 0.5f, 0.5f);
        QMatrix4x4 t2lMmatrix;
        t2lMmatrix.translate(-0.5f, -0.5f, -0.5f);

        QMatrix4x4 tlMmatrix = t1lMmatrix * rotateYMatrix.inverted() * rotateXMatrix.inverted() * t2lMmatrix;

        m_program->setUniformValue(m_uMatrix, perspectiveMatrix * zoomMatrix);

        float step = 1.0f / (actualCount - 1);
        for (float i = 0.0f; i <= 1.0f + step; i += step) {
            if (i >= 1.0f + step / 0.5f) break; // round check

            QMatrix4x4 sMmatrixTmp = sMmatrix;
            sMmatrixTmp.translate(0.0f, 0.0f, i);

            QMatrix4x4 slMmatrix;
            slMmatrix.translate(0.0f, 0.0f, i);
            slMmatrix = tlMmatrix * slMmatrix;

            m_program->setUniformValue(m_uScaleMatrix, sMmatrixTmp);
            m_program->setUniformValue(m_uScalelMatrix, slMmatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(m_aPosition);

        glDepthMask(GL_TRUE);
        mutex.unlock();
    }

    if (flagSave2) {
        flagSave2 = false;
        uchar *data = new uchar[width() * height() * 4];
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width(), height(), GL_BGRA, GL_UNSIGNED_BYTE, data);
        cv::Mat image = cv::Mat(height(), width(), CV_8UC4, saveData);
        for (int i = 0; i < width() * height() * 4; i += 4) {
            //qDebug() << data[i + 3];
            saveData[i + 3] = data[i + 3];
        }
        cv::flip(image, image, 0);
        //imshow("image", image);
        imwrite(fileName.toStdString(), image);
        delete [] saveData;
        saveData = NULL;
        delete [] data;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderLater();
    }

    if (flagSave) {
        flagSave = false;
        saveData = new uchar[width() * height() * 4];
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width(), height(), GL_BGRA, GL_UNSIGNED_BYTE, saveData);
        //QImage qimage = QImage(data, width(), height(), width() * 4, QImage::Format_ARGB32).mirrored();
        //qimage.save(fileName, 0, 100);
        //glClearColor((float) 16 / 17, (float) 16 / 17, (float) 16 / 17, 0.0f);
        flagSave2 = true;
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderLater();
    }

    m_program->release();
}

void GWindow::saveImage(QString _fileName)
{
    flagSave = true;
    fileName = _fileName;
    renderLater();
}

void GWindow::setAlpha(int value)
{
    alpha = (double) value / 1000;
    renderLater();
}

void GWindow::setRed(int value)
{
    red = (double) value / 1000;
    renderLater();
}

void GWindow::setGreen(int value)
{
    green = (double) value / 1000;
    renderLater();
}

void GWindow::setBlue(int value)
{
    blue = (double) value / 1000;
    renderLater();
}

void GWindow::setAlpha(double value)
{
    alpha = value;
    renderLater();
}

void GWindow::setRed(double value)
{
    red = value;
    renderLater();
}

void GWindow::setGreen(double value)
{
    green = value;
    renderLater();
}

void GWindow::setBlue(double value)
{
    blue = value;
    renderLater();
}

void GWindow::setViewFrame(bool value)
{
    mutex.lock();
    frame = value;
    mutex.unlock();
    renderLater();
}

void GWindow::setViewVR(bool value)
{
    mutex.lock();
    volumeRendering = value;
    mutex.unlock();
    renderLater();
}

void GWindow::setSlicesCount(int value)
{
    mutex.lock();
    count = value;
    emit setStatusMessage(QString("Slices: %1").arg(count));
    mutex.unlock();
    renderLater();
}

void GWindow::setViewXYSlice(bool value)
{
    mutex.lock();
    sliceXY = value;
    mutex.unlock();
    renderLater();
}

void GWindow::setViewXZSlice(bool value)
{
    mutex.lock();
    sliceXZ = value;
    mutex.unlock();
    renderLater();
}

void GWindow::setViewYZSlice(bool value)
{
    mutex.lock();
    sliceYZ = value;
    mutex.unlock();
    renderLater();
}

void GWindow::setTrim(bool value)
{
    mutex.lock();
    trim = value;
    emit setStatusMessage(QString("Trim: %1").arg(trim));
    mutex.unlock();
    renderLater();
}

void GWindow::alignToXY()
{
    rotateXMatrix.setToIdentity();
    //rotateXMatrix.rotate(0, -1, 0, 0);
    rotateYMatrix.setToIdentity();
    //rotateYMatrix.rotate(0, 0, -1, 0);
    renderLater();
}

void GWindow::alignToXZ()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(90, 1, 0, 0);
    rotateYMatrix.setToIdentity();
    //rotateYMatrix.rotate(0, 0, -1, 0);
    renderLater();
}

void GWindow::alignToYZ()
{
    rotateXMatrix.setToIdentity();
    //rotateXMatrix.rotate(0, -1, 0, 0);
    rotateYMatrix.setToIdentity();
    rotateYMatrix.rotate(90, 0, -1, 0);
    renderLater();
}

bool GWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ke = static_cast<QKeyEvent *>(event);
         if (ke->key() == Qt::Key_Plus) {
             mutex.lock();
             count = count + 2;
             if (count < 3) count = 3;
             emit setStatusMessage(QString("Slices: %1").arg(count));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Minus) {
             mutex.lock();
             count = count - 2;
             if (count < 3) count = 3;
             emit setStatusMessage(QString("Slices: %1").arg(count));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Up) {
             mutex.lock();
             imageHeight += 1.0f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Down) {
             mutex.lock();
             imageHeight -= 1.0f;
             if (imageHeight <= 1.0f) imageHeight = 1.0f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Right) {
             mutex.lock();
             imageWidth += 1.0f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Left) {
             mutex.lock();
             imageWidth -= 1.0f;
             if (imageWidth <= 1.0f) imageWidth = 1.0f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_S) {
             mutex.lock();
             imageHeight = origImageHeight;
             imageWidth = origImageWidth;
             emit setStatusMessage(QString("Size was reset"));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_F) {
             mutex.lock();
             if (frame)
                 frame = false;
             else
                 frame = true;
             emit setStatusMessage(QString("Frame: %1").arg(frame));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_A) {
             mutex.lock();
             alpha += 0.01f;
             if (alpha >= 1.0f) alpha = 1.0f;
             emit setStatusMessage(QString("Alpha: %1").arg(alpha));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Q) {
             mutex.lock();
             alpha -= 0.01f;
             if (alpha <= 0.0f) alpha = 0.0f;
             emit setStatusMessage(QString("Alpha: %1").arg(alpha));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_T) {
             mutex.lock();
             if (trim)
                 trim = false;
             else
                 trim = true;
             emit setStatusMessage(QString("Trim: %1").arg(trim));
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Z) {
             rotateXMatrix.setToIdentity();
             //rotateXMatrix.rotate(0, -1, 0, 0);
             rotateYMatrix.setToIdentity();
             //rotateYMatrix.rotate(0, 0, -1, 0);
         }
         if (ke->key() == Qt::Key_Y) {
             rotateXMatrix.setToIdentity();
             rotateXMatrix.rotate(90, 1, 0, 0);
             rotateYMatrix.setToIdentity();
             //rotateYMatrix.rotate(0, 0, -1, 0);
         }
         if (ke->key() == Qt::Key_X) {
             rotateXMatrix.setToIdentity();
             //rotateXMatrix.rotate(0, -1, 0, 0);
             rotateYMatrix.setToIdentity();
             rotateYMatrix.rotate(90, 0, -1, 0);
         }
         if (ke->key() == Qt::Key_B) {
             rotateXMatrix.setToIdentity();
             rotateXMatrix.rotate(180, -1, 0, 0);
             rotateYMatrix.setToIdentity();
             //rotateYMatrix.rotate(0, 0, -1, 0);
         }
         if (ke->key() == Qt::Key_G) {
             rotateXMatrix.setToIdentity();
             rotateXMatrix.rotate(90 + 180, 1, 0, 0);
             rotateYMatrix.setToIdentity();
             //rotateYMatrix.rotate(0, 0, -1, 0);
         }
         if (ke->key() == Qt::Key_R) {
             rotateXMatrix.setToIdentity();
             //rotateXMatrix.rotate(0, -1, 0, 0);
             rotateYMatrix.setToIdentity();
             rotateYMatrix.rotate(90 + 180, 0, -1, 0);
         }
         renderLater();
    }

    if (event->type() == QEvent::UpdateRequest)
        renderNow();

    return QWindow::event(event);
}
