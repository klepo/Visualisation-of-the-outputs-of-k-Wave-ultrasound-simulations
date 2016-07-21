/*
 * @file        gwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The implementation file containing the GWindow class - 3D scene window.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "gwindow.h"

// Some helper arrays for slices and 3d frame

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

/**
 * @brief GWindow::GWindow
 */
GWindow::GWindow(QMainWindow *qMainWindow)
    : m_program(0)
    , frame(true)
    , trim(0)
    , texture3DInitialized(false)
    , volumeRendering(false)
    , sliceXY(false)
    , sliceXZ(false)
    , sliceYZ(false)
    , initialized(false)

{
    this->qMainWindow = qMainWindow;

    // Init indices for 3D slices
    xYIndex = 0;
    xZIndex = 0;
    yZIndex = 0;

    // Init values for Volume Rendering
    count = 50;
    alpha = 0.5f;
    red = 0.5f;
    green = 0.5f;
    blue = 0.5f;
    zoom = 1.3f;

    // Default sizes for 3D frames
    imageWidth = 1;
    imageHeight = 1;
    imageDepth = 1;

    fullWidth = 1;
    fullHeight = 1;
    fullDepth = 1;

    origImageWidth = imageWidth;
    origImageHeight = imageHeight;
    origImageDepth = imageDepth;

    // Position of sensor mask
    posX = 0;
    posY = 0;
    posZ = 0;

    // Global min/max values
    minG = 0;
    maxG = 0;

    // Default colormap
    colormap = cv::COLORMAP_JET;

    flagSave = false;

    selectedDataset = NULL;

    // Create thread for loading whole dataset
    thread = new HDF5ReadingThread();
    connect(thread, SIGNAL(requestDone(Request *)), this, SLOT(setLoaded(Request *)));
}

/**
 * @brief GWindow::~GWindow
 */
GWindow::~GWindow()
{
    glDeleteVertexArrays(1, &vao);

    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &colormapTexture);
    glDeleteTextures(1, &textureXY);
    glDeleteTextures(1, &textureXZ);
    glDeleteTextures(1, &textureYZ);

    glDeleteBuffers(1, &iboPlaneElements);
    glDeleteBuffers(1, &iboCubeElements);

    vboPlaneVertices.destroy();
    vboCubeVertices.destroy();
    vboSliceXYVertices.destroy();
    vboSliceXZVertices.destroy();
    vboSliceYZVertices.destroy();
    vboSliceTexCoords.destroy();

    m_program->release();
    delete m_program;
    thread->clearRequests();
    //thread->clearDoneRequests();
    thread->wait();
    //thread->deleteLater();
}

/**
 * @brief GWindow::getThread
 * @return 3D data loading thread
 */
HDF5ReadingThread *GWindow::getThread()
{
    return thread;
}

/**
 * @brief GWindow::initialize Initialization of OpenGL
 */
void GWindow::initialize()
{
    //emit setStatusMessage(QString("Slices: %1").arg(count));

    // Load, create and link shaders
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexShader.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentShader.frag");
    m_program->link();

    // Init attribute variables
    m_aPosition = m_program->attributeLocation("aPosition");
    m_aTextureCoord = m_program->attributeLocation("aTextureCoord");

    // Init uniform variables
    m_uFrame = m_program->uniformLocation("uFrame");
    m_uFrameColor = m_program->uniformLocation("uFrameColor");
    m_uXYBorder = m_program->uniformLocation("uXYBorder");
    m_uXZBorder = m_program->uniformLocation("uXZBorder");
    m_uYZBorder = m_program->uniformLocation("uYZBorder");

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

    m_uVolumeRendering = m_program->uniformLocation("uVolumeRendering");

    m_uSliceSampler = m_program->uniformLocation("uSliceSampler");

    //glBindFragDataLocation(m_program->programId(), 0, "colorOut");

    // Generate buffers
    // for slices
    glGenBuffers(1, &iboPlaneElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlaneElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeElements), planeElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // and for 3D frame
    glGenBuffers(1, &iboCubeElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCubeElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeElements), cubeElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Vertex buffer
    vboCubeVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboCubeVertices.create();
    vboCubeVertices.bind();
    vboCubeVertices.allocate(cubeVertices, sizeof(GLfloat) * 8 * 3);
    vboCubeVertices.release();

    vboPlaneVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboPlaneVertices.create();
    vboPlaneVertices.bind();
    vboPlaneVertices.allocate(planeVertices, sizeof(GLfloat) * 4 * 3);
    vboPlaneVertices.release();

    vboSliceXYVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceXYVertices.create();
    vboSliceXYVertices.bind();
    vboSliceXYVertices.allocate(sliceXYVertices, sizeof(GLfloat) * 4 * 3);
    vboSliceXYVertices.release();

    vboSliceXZVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceXZVertices.create();
    vboSliceXZVertices.bind();
    vboSliceXZVertices.allocate(sliceXZVertices, sizeof(GLfloat) * 4 * 3);
    vboSliceXZVertices.release();

    vboSliceYZVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceYZVertices.create();
    vboSliceYZVertices.bind();
    vboSliceYZVertices.allocate(sliceYZVertices, sizeof(GLfloat) * 4 * 3);
    vboSliceYZVertices.release();

    vboSliceTexCoords = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceTexCoords.create();
    vboSliceTexCoords.bind();
    vboSliceTexCoords.allocate(sliceTexCoords, sizeof(GLfloat) * 4 * 2);
    vboSliceTexCoords.release();

    // VAO
    glGenVertexArrays(1, &vao);

    // 3D texture
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_3D, texture);
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

    // 1D texture
    glGenTextures(1, &colormapTexture);

    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // 2D textures
    glGenTextures(1, &textureXY);

    glBindTexture(GL_TEXTURE_2D, textureXY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &textureXZ);

    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &textureYZ);

    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Settings
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    //glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    //glClearColor((float) 16 / 18, (float) 16 / 18, (float) 16 / 18, 0.0f);
    QColor color = qMainWindow->palette().color(QPalette::Window);
    glClearColor((float) color.redF(), (float) color.greenF(), (float) color.blueF(), (float) color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Important for save png image!
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

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
    // Set default colormap
    changeColormap();

    // Default scene rotation
    //rotateXMatrix.rotate(-20, -1, 0, 0);
    //rotateXMatrix.rotate(-45, 0, 1, 0);
    //rotateYMatrix.rotate(-45, 1, 0, 0);

    m_program->bind();

    m_program->setUniformValue(m_uMin, 0.0f);
    m_program->setUniformValue(m_uMax, 0.0f);

    /*glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);*/
    // Set some default uniform values
    m_program->setUniformValue(m_uColormapSampler, 1);
    m_program->setUniformValue(m_uSliceSampler, 2);
    m_program->setUniformValue(m_uSampler, 0);

    m_program->setUniformValue(m_uFrame, false);

    m_program->setUniformValue(m_uFrameColor, (float) (1.0f - color.redF()), (float) (1.0f - color.greenF()), (float) (1.0f - color.blueF()), 1.0f);

    m_program->setUniformValue(m_uSlices, false);

    m_program->setUniformValue(m_uVolumeRendering, false);

    m_program->setUniformValue(m_uXYBorder, false);
    m_program->setUniformValue(m_uXZBorder, false);
    m_program->setUniformValue(m_uYZBorder, false);

    m_program->release();

    initialized = true;
    renderLater();
}

/**
 * @brief GWindow::isTexture3DInitialized
 * @return
 */
bool GWindow::isTexture3DInitialized()
{
    return texture3DInitialized;
}

/**
 * @brief GWindow::setMainSize Set size for main 3D frame - all 3D domain
 * @param depth
 * @param height
 * @param width
 */
void GWindow::setMainSize(unsigned int depth, unsigned int height, unsigned int width)
{
    fullWidth = width;
    fullHeight = height;
    fullDepth = depth;
}

/**
 * @brief GWindow::setSize Set size for original (sensor mask defined) frame
 * @param depth
 * @param height
 * @param width
 */
void GWindow::setSize(unsigned int depth, unsigned int height, unsigned int width)
{
    imageDepth = depth;
    imageHeight = height;
    imageWidth = width;
    origImageWidth = imageWidth;
    origImageHeight = imageHeight;
    origImageDepth = imageDepth;
}

/**
 * @brief GWindow::setPosition Set position of sensor mask defined 3D dataset
 * @param posZ
 * @param posY
 * @param posX
 */
void GWindow::setPosition(unsigned int posZ, unsigned int posY, unsigned int posX)
{
    this->posZ = posZ;
    this->posY = posY;
    this->posX = posX;
}

/**
 * @brief GWindow::load3DTexture Performs loading of 3D data for VR
 * @param dataset
 */
void GWindow::load3DTexture(HDF5Helper::HDF5Dataset *dataset)
{
    // If dataset is already loaded
    if (selectedDataset != NULL && selectedDataset->getName() == dataset->getName()) {
        emit loaded(selectedDataset->getName());
        return;
    }

    //thread->clearRequests();
    //thread->wait();
    //thread->clearDoneRequests();

    selectedDataset = dataset;

    texture3DInitialized = false;

    /*imageWidth = dataset->getDims()[2];
    imageHeight = dataset->getDims()[1];
    imageDepth = dataset->getDims()[0];*/

    //PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
    //glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    // Init 3D texture
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageWidth, imageHeight, imageDepth, 0, GL_RED, GL_FLOAT, NULL);

    // Check OUT_OF_MEMORY, dataset is too big
    if (checkGlError() != GL_NO_ERROR) {
        emit loaded(selectedDataset->getName());
        unload3DTexture();
        return;
    }

    thread->createRequest(selectedDataset);
    // Start loading thread
    thread->start();
}

/**
 * @brief GWindow::unload3DTexture Free 3D texture data
 */
void GWindow::unload3DTexture()
{
    //PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
    //glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageWidth, imageHeight, imageDepth, 0, GL_RED, GL_FLOAT, NULL);
}

/**
 * @brief GWindow::unloadDataset
 */
void GWindow::unloadDataset()
{
    selectedDataset = NULL;
}

/**
 * @brief GWindow::clearData Clear 3D data, slices data and reset sizes
 */
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

        unloadDataset();

        renderLater();
    }
}

/**
 * @brief GWindow::setLoaded Action on part of 3D dataset loaded
 * @param r loading request
 */
void GWindow::setLoaded(Request *r)
{
    //texture//mutex.lock();
    texture3DInitialized = false;
    //PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;
    //glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) wglGetProcAddress("glTexSubImage3D");

    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Set 3D data to 3D texture
    glTexSubImage3D(GL_TEXTURE_3D, 0, r->offset.x(), r->offset.y(), r->offset.z(), r->count.x(), r->count.y(), r->count.z(), GL_RED, GL_FLOAT, r->data);

    //if (checkGlError() != GL_NO_ERROR) return;

    // Last block of 3D data
    if (r->offset.z() + r->count.z() == imageDepth) {
        //thread->deleteLater();
        texture3DInitialized = true;
        changeColormap(colormap);
        renderLater();
        //emit partLoaded((int) ((double) (i + 1) / imageDepth * 100));
        emit loaded(selectedDataset->getName());
    }

    thread->deleteDoneRequest(r);
}

/**
 * @brief GWindow::setXYSlice Set 2D image data for XY 3D slice
 * @param data
 * @param width
 * @param height
 * @param index
 */
void GWindow::setXYSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setXYSlice";
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    xYIndex = index;
    renderLater();
}

/**
 * @brief GWindow::setXZSlice Set 2D image data for XZ 3D slice
 * @param data
 * @param width
 * @param height
 * @param index
 */
void GWindow::setXZSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setXZSlice";
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    xZIndex = index;
    renderLater();
}

/**
 * @brief GWindow::setYZSlice Set 2D image data for YZ 3D slice
 * @param data
 * @param width
 * @param height
 * @param index
 */
void GWindow::setYZSlice(float *data, unsigned int width, unsigned int height, float index)
{
    //qDebug() << "setYZSlice";
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    yZIndex = index;
    renderLater();
}

/**
 * @brief GWindow::clearSlices "Free" or "reset" 2D textures for slices
 */
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

/**
 * @brief GWindow::changeColormap Change colormap
 * @param colormap
 */
void GWindow::changeColormap(int colormap)
{
    this->colormap = colormap;
    glBindTexture(GL_TEXTURE_1D, colormapTexture);

    // Fill 1D texture with colormap values
    cv::Mat colormapImage = cv::Mat::zeros(1, 256, CV_8UC1);
    for (unsigned int i = 0; i < 256; i++)
        colormapImage.data[i] = i;
    cv::applyColorMap(colormapImage, colormapImage, this->colormap);
    cv::cvtColor(colormapImage, colormapImage, cv::COLOR_BGR2RGB);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, colormapImage.data);

    m_program->bind();

    // Set to shader
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    m_program->setUniformValue(m_uColormapSampler, 1);

    m_program->release();

    renderLater();
}

/**
 * @brief GWindow::changeMinValue Set min value for colormaping
 * @param value
 */
void GWindow::changeMinValue(float value)
{
    minG = value;
    m_program->bind();
    m_program->setUniformValue(m_uMin, minG);
    m_program->release();
    renderLater();
}

/**
 * @brief GWindow::changeMaxValue Set max value for colormaping
 * @param value
 */
void GWindow::changeMaxValue(float value)
{
    maxG = value;
    m_program->bind();
    m_program->setUniformValue(m_uMax, maxG);
    m_program->release();
    renderLater();
}

/**
 * @brief GWindow::renderFrame Render 3D frame
 */
void GWindow::renderFrame()
{
    glBindVertexArray(vao);
    vboCubeVertices.bind();
    glEnableVertexAttribArray(m_aPosition);
    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCubeElements);
    glDrawElements(GL_LINE_LOOP,  sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(m_aPosition);
    vboCubeVertices.release();
    glBindVertexArray(0);
}

/**
 * @brief GWindow::convertToOpenGLRelative
 * @param point
 * @return
 */
QPointF GWindow::convertToOpenGLRelative(QPointF point)
{
    QPointF pointOutput;
    pointOutput.setX((point.x() / (float) width() - 0.5f) * 2.0f);
    pointOutput.setY((point.y() / (float) height() - 0.5f) * 2.0f);
    return pointOutput;
}

/**
 * @brief GWindow::round
 * @param number
 * @param precision
 * @return
 */
float GWindow::round(float number, float precision)
{
    return (float) (floor(number * (1.0f / precision) + 0.5) / (1.0f / precision));
}

/**
 * @brief GWindow::render Main render function
 */
void GWindow::render()
{
    checkGlError();

    int actualCount = count;

    // Rotation of scene by left mouse click
    if (leftButton) {
        /*if (lastPos.x() > currentPos.x())
            qDebug() << ">";
        else if (lastPos.x() < currentPos.x())
            qDebug() << "<";
        else
            qDebug() << "=";*/
        // TODO
        rotateXMatrix.rotate((float) (lastPos.y() - currentPos.y()) / 2.0f, -1, 0, 0);
        rotateYMatrix.rotate((float) (lastPos.x() - currentPos.x()) / 2.0f, 0, -1, 0);
        //actualCount = 30;
    }

    // Move of scene by right mouse click
    if (rightButton) {
        QPointF lastPosRl = convertToOpenGLRelative(lastPos);
        QPointF currentPosRl = convertToOpenGLRelative(currentPos);
        position.setX(position.x() - (lastPosRl.x() - currentPosRl.x()));
        position.setY(position.y() + (lastPosRl.y() - currentPosRl.y()));
        //actualCount = 30;
    }

    // Clear viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, width(), height());

    m_program->bind();

    // Perspective projection
    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.perspective(45, (float) width() / (float) height(), 0.1f, 100.0f);
    //perspectiveMatrix.ortho(- (float) width() / (float) height(), (float) width() / (float) height(), -1.0f, 1.0f, -1.0f, 1000.0f);

    // Zoom of scene
    QMatrix4x4 zoomMatrix;
    zoom -= wheelDelta / 2000.0f;
    if (zoom < 0.1f) zoom = 0.1f;
    if (zoom > 1.5f) zoom = 1.5f;
    zoomMatrix.lookAt(QVector3D(0, 0, qPow((float) zoom, 5.0f)), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    // Final matrix
    QMatrix4x4 matrix;
    matrix = perspectiveMatrix * zoomMatrix * rotateXMatrix * rotateYMatrix;

    // Move transformation
    QVector4D pointC(0.0f, 0.0f, 0.0f, 1.0f);
    pointC = matrix * pointC.normalized();
    QMatrix4x4 invMatrix = matrix.inverted();
    QVector4D point0(position.x(), position.y(), pointC.toVector3DAffine().z(), 1.0f);
    point0 = invMatrix * point0.normalized();
    QVector3D point03D = point0.toVector3DAffine();
    QMatrix4x4 moveMatrix;
    moveMatrix.translate(point03D);
    matrix = matrix * moveMatrix;

    // Create vectors for scale 3D frame to the longest size = 1.0f
    QVector3D vecScale((float) imageWidth, (float) imageHeight, (float) imageDepth);
    QVector3D vecFullScale((float) fullWidth, (float) fullHeight, (float) fullDepth);
    float max = qMax(vecScale.x(), qMax(vecScale.y(), vecScale.z()));
    float fullMax = qMax(vecFullScale.x(), qMax(vecFullScale.y(), vecFullScale.z()));
    vecFullScale = vecFullScale / fullMax; // longest size is 1.0f

    // Values to fragment shader for scaling 3D frame ->  operations +, -, /, * are faster here
    QVector3D vecScale0 = vecScale / max;
    m_program->setUniformValue(m_uWidth, (float) 1.0f / (vecScale0.x() / 2.0f));
    m_program->setUniformValue(m_uHeight, (float) 1.0f / (vecScale0.y() / 2.0f));
    m_program->setUniformValue(m_uDepth, (float) 1.0f / (vecScale0.z() / 2.0f));

    m_program->setUniformValue(m_uXMax, 0.5f + ((float) vecScale0.x() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uXMin, 0.5f - ((float) vecScale0.x() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uYMax, 0.5f + ((float) vecScale0.y() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uYMin, 0.5f - ((float) vecScale0.y() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uZMax, 0.5f + ((float) vecScale0.z() / 2.0f) / 2.0f);
    m_program->setUniformValue(m_uZMin, 0.5f - ((float) vecScale0.z() / 2.0f) / 2.0f);

    vecScale = vecScale / fullMax;
    // Translate to the midlle of 3D frame
    matrix.translate(-vecFullScale.x() / 2.0f, -vecFullScale.y() / 2.0f, -vecFullScale.z() / 2.0f);

    // Send matrix to shader
    m_program->setUniformValue(m_uMatrix, matrix);
    m_program->setUniformValue(m_uScalelMatrix, QMatrix4x4());
    m_program->setUniformValue(m_uScaleMatrix, QMatrix4x4());

    // Trim values
    if (trim) {
        m_program->setUniformValue(m_uTrim, true);
    } else {
        m_program->setUniformValue(m_uTrim, false);
    }

    // Frame
    if (frame) {
        glDisable(GL_CULL_FACE);
        m_program->setUniformValue(m_uFrame, true);
        QMatrix4x4 sMmatrix;

        QMatrix4x4 rMatrix;
        rMatrix.scale(1.02f);
        rMatrix.translate(-0.01f, -0.01f, 0);

        // Smaller frame
        sMmatrix.translate((float) posX / fullMax, (float) posY / fullMax, (float) posZ / fullMax);
        sMmatrix.scale(vecScale);
        m_program->setUniformValue(m_uScaleMatrix, sMmatrix * rMatrix);
        renderFrame();

        // Big frame
        if (imageWidth != fullWidth || imageHeight != fullHeight || imageDepth || fullDepth) {
            QMatrix4x4 sMmatrix;
            sMmatrix.scale(vecFullScale);
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * rMatrix);
            renderFrame();
        }

        m_program->setUniformValue(m_uFrame, false);
        glEnable(GL_CULL_FACE);
    }

    // Slices
    if (sliceXY || sliceXZ || sliceYZ) {
        //mutex.lock();
        m_program->setUniformValue(m_uSlices, true);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        //qDebug() << glIsEnabled(GL_DEPTH_TEST);

        QMatrix4x4 sMmatrix;
        sMmatrix.translate((float) posX / fullMax, (float) posY / fullMax, (float) posZ / fullMax);
        sMmatrix.scale(vecScale);

        QMatrix4x4 s2Mmatrix;
        s2Mmatrix.scale(1.02f);
        // Colored frames (along axis) transfomation
        QMatrix4x4 s2XYMmatrix, s2XZMmatrix, s2YZMmatrix;
        s2XYMmatrix = s2Mmatrix;
        s2XZMmatrix = s2Mmatrix;
        s2YZMmatrix = s2Mmatrix;
        s2XYMmatrix.translate(-0.01f, -0.01f, 0);
        s2XZMmatrix.translate(-0.01f, 0, -0.01f);
        s2YZMmatrix.translate(0, -0.01f, -0.01f);

        // Translate by slice index
        QMatrix4x4 xYMmatrix;
        xYMmatrix.translate(0, 0, xYIndex);
        QMatrix4x4 xZMmatrix;
        xZMmatrix.translate(0, xZIndex, 0);
        QMatrix4x4 yZMmatrix;
        yZMmatrix.translate(yZIndex, 0, 0);

        glBindVertexArray(vao);

        vboSliceTexCoords.bind();
        glEnableVertexAttribArray(m_aTextureCoord);
        glVertexAttribPointer(m_aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlaneElements);

        glEnableVertexAttribArray(m_aPosition);

        if (sliceXY) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xYMmatrix);
            vboSliceXYVertices.bind();
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureXY);
            // Because some bug
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            // Draw slice
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXYBorder, true);
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xYMmatrix * s2XYMmatrix);
            // Draw 2D frame
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXYBorder, false);
            vboSliceXYVertices.release();
        }

        if (sliceXZ) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xZMmatrix);
            vboSliceXZVertices.bind();
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureXZ);
            // Because some bug
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXZBorder, true);
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * xZMmatrix * s2XZMmatrix);
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXZBorder, false);
            vboSliceXZVertices.release();
        }

        if (sliceYZ) {
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * yZMmatrix);
            vboSliceYZVertices.bind();
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureYZ);
            // Because some bug
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uYZBorder, true);
            m_program->setUniformValue(m_uScaleMatrix, sMmatrix * yZMmatrix * s2YZMmatrix);
            glDrawElements(GL_LINE_LOOP,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uYZBorder, false);
            vboSliceYZVertices.release();
        }

        glDisableVertexAttribArray(m_aTextureCoord);
        glDisableVertexAttribArray(m_aPosition);

        vboSliceTexCoords.release();

        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        //glDepthMask(GL_TRUE);

        m_program->setUniformValue(m_uSlices, false);
        //mutex.unlock();
    }

    // Volume rendering
    if (volumeRendering) {
        m_program->setUniformValue(m_uVolumeRendering, true);

        //mutex.lock();
        // Because of same opacity from all directions
        glDepthMask(GL_FALSE);
        //glDisable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texture);

        // Recompute alpha and set alpha + colors
        m_program->setUniformValue(m_uAlpha, (GLfloat) (1.0f - pow(1.0f - alpha, 5.0f / (float) actualCount)));
        m_program->setUniformValue(m_uRed, (GLfloat) (1.0f - pow(1.0f - red, 5.0f / (float) actualCount)));
        m_program->setUniformValue(m_uGreen, (GLfloat) (1.0f - pow(1.0f - green, 5.0f / (float) actualCount)));
        m_program->setUniformValue(m_uBlue, (GLfloat) (1.0f - pow(1.0f - blue, 5.0f / (float) actualCount)));

        glBindVertexArray(vao);

        vboPlaneVertices.bind();
        glEnableVertexAttribArray(m_aPosition);
        glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlaneElements);

        // Scale 3D frame for VR
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

        m_program->setUniformValue(m_uMatrix, perspectiveMatrix * zoomMatrix * rotateXMatrix * rotateYMatrix * moveMatrix * rotateYMatrix.inverted() * rotateXMatrix.inverted());

        float step = 1.0f / (actualCount - 1);
        // For number of slices
        for (float i = 0.0f; i <= 1.0f + step; i += step) {
            if (i >= 1.0f + step / 0.5f) break; // round check

            QMatrix4x4 sMmatrixTmp = sMmatrix;
            sMmatrixTmp.translate(0.0f, 0.0f, i);

            // Matrix without scene rotaion
            QMatrix4x4 slMmatrix;
            slMmatrix.translate(0.0f, 0.0f, i);
            slMmatrix = tlMmatrix * slMmatrix;

            m_program->setUniformValue(m_uScaleMatrix, sMmatrixTmp);
            m_program->setUniformValue(m_uScalelMatrix, slMmatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(planeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(m_aPosition);

        vboPlaneVertices.release();

        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        //mutex.unlock();
        m_program->setUniformValue(m_uVolumeRendering, false);
    }

    // Sace 3D scene to png image
    if (flagSave) {
        flagSave = false;
        uchar *data = new uchar[width() * height() * 4];
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width(), height(), GL_BGRA, GL_UNSIGNED_BYTE, data);
        cv::Mat image = cv::Mat(height(), width(), CV_8UC4, data);
        cv::flip(image, image, 0);
        //imshow("image", image);
        cv::imwrite(fileName.toStdString(), image);
        delete [] data;
        renderLater();
    }

    m_program->release();
}

// Some slots for user interaction

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
    //mutex.lock();
    frame = value;
    //mutex.unlock();
    renderLater();
}

void GWindow::setViewVR(bool value)
{
    //mutex.lock();
    volumeRendering = value;
    //mutex.unlock();
    renderLater();
}

void GWindow::setSlicesCount(int value)
{
    //mutex.lock();
    count = value;
    emit setStatusMessage(QString("Slices: %1").arg(count));
    //mutex.unlock();
    renderLater();
}

void GWindow::setViewXYSlice(bool value)
{
    //mutex.lock();
    sliceXY = value;
    //mutex.unlock();
    renderLater();
}

void GWindow::setViewXZSlice(bool value)
{
    //mutex.lock();
    sliceXZ = value;
    //mutex.unlock();
    renderLater();
}

void GWindow::setViewYZSlice(bool value)
{
    //mutex.lock();
    sliceYZ = value;
    //mutex.unlock();
    renderLater();
}

void GWindow::setTrim(bool value)
{
    //mutex.lock();
    trim = value;
    emit setStatusMessage(QString("Trim: %1").arg(trim));
    //mutex.unlock();
    renderLater();
}

void GWindow::alignToXY()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    renderLater();
}

void GWindow::alignToXZ()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(90, 1, 0, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

void GWindow::alignToYZ()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    rotateYMatrix.rotate(90, 0, -1, 0);
    renderLater();
}

void GWindow::alignToXYFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(180, 0, -1, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

void GWindow::alignToXZFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(90 + 180, 1, 0, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

void GWindow::alignToYZFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    rotateYMatrix.rotate(90 + 180, 0, -1, 0);
    renderLater();
}

/**
 * @brief GWindow::event Catch keyboard events on 3D scene
 * @param event
 * @return QWindow::event(event);
 */
bool GWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Plus) {
            //mutex.lock();
            count = count + 2;
            if (count < 3) count = 3;
            emit setStatusMessage(QString("Slices: %1").arg(count));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Minus) {
            //mutex.lock();
            count = count - 2;
            if (count < 3) count = 3;
            emit setStatusMessage(QString("Slices: %1").arg(count));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Up) {
            //mutex.lock();
            imageHeight += 1.0f;
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Down) {
            //mutex.lock();
            imageHeight -= 1.0f;
            if (imageHeight <= 1.0f) imageHeight = 1.0f;
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Right) {
            //mutex.lock();
            imageWidth += 1.0f;
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Left) {
            //mutex.lock();
            imageWidth -= 1.0f;
            if (imageWidth <= 1.0f) imageWidth = 1.0f;
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_S) {
            //mutex.lock();
            imageHeight = origImageHeight;
            imageWidth = origImageWidth;
            emit setStatusMessage(QString("Size was reset"));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_F) {
            //mutex.lock();
            if (frame)
                frame = false;
            else
                frame = true;
            emit setStatusMessage(QString("Frame: %1").arg(frame));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_A) {
            //mutex.lock();
            alpha += 0.01f;
            if (alpha >= 1.0f) alpha = 1.0f;
            emit setStatusMessage(QString("Alpha: %1").arg(alpha));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Q) {
            //mutex.lock();
            alpha -= 0.01f;
            if (alpha <= 0.0f) alpha = 0.0f;
            emit setStatusMessage(QString("Alpha: %1").arg(alpha));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_T) {
            //mutex.lock();
            if (trim)
                trim = false;
            else
                trim = true;
            emit setStatusMessage(QString("Trim: %1").arg(trim));
            //mutex.unlock();
        }
        if (ke->key() == Qt::Key_Z) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
        }
        if (ke->key() == Qt::Key_Y) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(90, 1, 0, 0);
            rotateYMatrix.setToIdentity();
        }
        if (ke->key() == Qt::Key_X) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
            rotateYMatrix.rotate(90, 0, -1, 0);
        }
        if (ke->key() == Qt::Key_B) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(180, 0, -1, 0);
            rotateYMatrix.setToIdentity();
        }
        if (ke->key() == Qt::Key_G) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(90 + 180, 1, 0, 0);
            rotateYMatrix.setToIdentity();
        }
        if (ke->key() == Qt::Key_R) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
            rotateYMatrix.rotate(90 + 180, 0, -1, 0);
        }
        if (ke->key() == Qt::Key_C) {
            position.setX(0);
            position.setY(0);
        }
        renderLater();
    }
    case QEvent::MouseButtonPress:
        if (((QMouseEvent *) event)->buttons() == Qt::MiddleButton) {
            position.setX(0);
            position.setY(0);
            renderLater();
        }
    default:
        return OpenGLWindow::event(event);
    }
}
