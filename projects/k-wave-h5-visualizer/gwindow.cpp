/**
 * @file        gwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 March     2017 (updated)
 *
 * @brief       The implementation file containing the GWindow class - 3D scene window.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "gwindow.h"
#include "vertices.h"

/**
 * @brief GWindow::GWindow
 */
GWindow::GWindow(QMainWindow *qMainWindow)
    : m_program(0)
    , initialized(false)

{
    this->qMainWindow = qMainWindow;

    // Default sizes for 3D frames
    imageSize = QVector3DI(1, 1, 1);
    imageSizeOrig = QVector3DI(1, 1, 1);
    fullSize = QVector3DI(1, 1, 1);

    // Position of sensor mask
    imagePosition = QVector3DI(0, 0, 0);

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
    glDeleteTextures(1, &opacityTexture);
    glDeleteTextures(1, &textureXY);
    glDeleteTextures(1, &textureXZ);
    glDeleteTextures(1, &textureYZ);

    glDeleteBuffers(1, &iboSliceElements);
    glDeleteBuffers(1, &iboCubeElements);

    vboCubeVertices.destroy();
    vboSliceVertices.destroy();

    m_program->release();
    delete m_program;
    thread->clearRequests();
    //thread->clearDoneRequests();
    thread->wait();
    //thread->deleteLater();
}

/**
 * @brief GWindow::initialize Initialization of OpenGL
 */
void GWindow::initialize()
{
    // Load, create and link shaders
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexShader");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentShader");
    m_program->link();

    // Init attribute variables
    m_aPosition = m_program->attributeLocation("aPosition");

    // Init uniform variables
    m_uMatrix = m_program->uniformLocation("uMatrix");
    m_uSliceMatrix = m_program->uniformLocation("uSliceMatrix");

    uVolumeTexture = m_program->uniformLocation("uVolume");
    uColormapTexture = m_program->uniformLocation("uColormap");
    uOpacityTexture = m_program->uniformLocation("uOpacity");
    uSliceTexture = m_program->uniformLocation("uSlice");
    uBoxBackSampler = m_program->uniformLocation("uBoxBackSampler");
    uBoxFrontSampler = m_program->uniformLocation("uBoxFrontSampler");

    m_uFrame = m_program->uniformLocation("uFrame");
    m_uSlices = m_program->uniformLocation("uSlices");
    m_uXYBorder = m_program->uniformLocation("uXYBorder");
    m_uXZBorder = m_program->uniformLocation("uXZBorder");
    m_uYZBorder = m_program->uniformLocation("uYZBorder");
    m_uVolumeRenderingBox = m_program->uniformLocation("uVolumeRenderingBox");
    m_uVolumeRendering = m_program->uniformLocation("uVolumeRendering");

    m_uTrim = m_program->uniformLocation("uTrim");

    m_uSteps = m_program->uniformLocation("uSteps");

    m_uFrameColor = m_program->uniformLocation("uFrameColor");

    m_uHeight = m_program->uniformLocation("uHeight");
    m_uWidth = m_program->uniformLocation("uWidth");

    m_uColor = m_program->uniformLocation("uColor");

    m_uMin = m_program->uniformLocation("uMin");
    m_uMax = m_program->uniformLocation("uMax");

    m_uMode = m_program->uniformLocation("uMode");

    // Generate buffers
    // for slices
    glGenBuffers(1, &iboSliceElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboSliceElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sliceElements), sliceElements, GL_STATIC_DRAW);
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

    vboSliceVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceVertices.create();
    vboSliceVertices.bind();
    vboSliceVertices.allocate(sliceVertices, sizeof(GLfloat) * 4 * 3);
    vboSliceVertices.release();

    // VAO
    glGenVertexArrays(1, &vao);

    glGenTextures(1, &textureFboBack);
    glBindTexture(GL_TEXTURE_2D, textureFboBack);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureFboFront);
    glBindTexture(GL_TEXTURE_2D, textureFboFront);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 3D texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);

    // 1D colormap texture
    glGenTextures(1, &colormapTexture);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_1D, 0);

    // 1D opacity texture
    glGenTextures(1, &opacityTexture);
    glBindTexture(GL_TEXTURE_1D, opacityTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_1D, 0);

    // 2D textures
    glGenTextures(1, &textureXY);
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureXZ);
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureYZ);
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, textureFboBack);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, textureFboFront);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboBack, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //TODO
    //glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Settings
    //glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    QColor colorW = qMainWindow->palette().color(QPalette::Window);
    glClearColor(float(colorW.redF()), float(colorW.greenF()), float(colorW.blueF()), 0.0f/*float(colorW.alphaF())*/);
    //glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_ADD);

    changeColormap();

    changeOpacity();

    // Default scene rotation
    //rotateXMatrix.rotate(-20, -1, 0, 0);
    //rotateXMatrix.rotate(-45, 0, 1, 0);
    //rotateYMatrix.rotate(-45, 1, 0, 0);

    m_program->bind();
    // Set some default uniform values

    m_program->setUniformValue(m_uMin, 0.0f);
    m_program->setUniformValue(m_uMax, 0.0f);

    m_program->setUniformValue(uOpacityTexture, 4);
    m_program->setUniformValue(uBoxBackSampler, 3);
    m_program->setUniformValue(uBoxFrontSampler, 5);
    m_program->setUniformValue(uSliceTexture, 2);
    m_program->setUniformValue(uColormapTexture, 1);
    m_program->setUniformValue(uVolumeTexture, 0);

    m_program->setUniformValue(m_uFrame, false);

    m_program->setUniformValue(m_uFrameColor, float(1.0f - colorW.redF()), float(1.0f - colorW.greenF()), float(1.0f - colorW.blueF()), 1.0f);

    m_program->setUniformValue(m_uSlices, false);

    m_program->setUniformValue(m_uVolumeRendering, false);
    m_program->setUniformValue(m_uVolumeRenderingBox, false);

    m_program->setUniformValue(m_uXYBorder, false);
    m_program->setUniformValue(m_uXZBorder, false);
    m_program->setUniformValue(m_uYZBorder, false);

    m_program->setUniformValue(m_uSteps, steps);

    m_program->setUniformValue(m_uWidth, float(width()));
    m_program->setUniformValue(m_uHeight, float(height()));

    m_program->release();

    initialized = true;
    renderLater();
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
void GWindow::setMainSize(HDF5Helper::HDF5Vector3D size)
{
    fullSize = QVector3DI(size.x(), size.y(), size.z());
}

/**
 * @brief GWindow::setSize Set size for original (sensor mask defined) frame
 * @param depth
 * @param height
 * @param width
 */
void GWindow::setSize(HDF5Helper::HDF5Vector3D size)
{
    imageSize = QVector3DI(size.x(), size.y(), size.z());
    imageSizeOrig = imageSize;
}

/**
 * @brief GWindow::setPosition Set position of sensor mask defined 3D dataset
 * @param posZ
 * @param posY
 * @param posX
 */
void GWindow::setPosition(HDF5Helper::HDF5Vector3D position)
{
    imagePosition = QVector3DI(position.x(), position.y(), position.z());
}

/**
 * @brief GWindow::load3DTexture Performs loading of 3D data for VR
 * @param dataset
 */
void GWindow::load3DTexture(HDF5Helper::HDF5Dataset *dataset, hsize_t index)
{
    selectedDataset = dataset;
    texture3DInitialized = false;

    // Init 3D texture
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageSize.x(), imageSize.y(), imageSize.z(), 0, GL_RED, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    // Check OUT_OF_MEMORY, dataset is too big
    if (checkGlError() != GL_NO_ERROR) {
        emit loaded(selectedDataset->getName());
        unload3DTexture();
        return;
    }

    thread->createRequest(selectedDataset, index);

    // Start loading thread
    thread->start();
}

/**
 * @brief GWindow::unload3DTexture Free 3D texture data
 */
void GWindow::unload3DTexture()
{
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

/**
 * @brief GWindow::unloadDataset
 */
void GWindow::unloadDataset()
{
    selectedDataset = 0;
}

/**
 * @brief GWindow::clearData Clear 3D data, slices data and reset sizes
 */
void GWindow::clearData()
{
    if (initialized) {
        imageSize = QVector3DI(1, 1, 1);
        fullSize = QVector3DI(1, 1, 1);
        imageSizeOrig = QVector3DI(1, 1, 1);
        imagePosition = QVector3DI(0, 0, 0);

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
void GWindow::setLoaded(Request *request)
{
    if (selectedDataset != request->dataset)
        return;

    texture3DInitialized = false;
    HDF5Helper::HDF5Vector3D offset = request->offset;
    HDF5Helper::HDF5Vector3D count = request->count;

    // Set 3D data to 3D texture
    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, offset.x(), offset.y(), offset.z(), count.x(), count.y(), count.z(), GL_RED, GL_FLOAT, request->data);
    glBindTexture(GL_TEXTURE_3D, 0);

    // Last block of 3D data
    if (offset.z() + count.z() == hsize_t(imageSize.z())) {
        texture3DInitialized = true;
        //changeColormap(colormap);
        renderLater();
        emit loaded(selectedDataset->getName());
    }

    thread->deleteDoneRequest(request);
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
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setZ(index);
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
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setY(index);
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
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setX(index);
    renderLater();
}

/**
 * @brief GWindow::clearSlices "Free" or "reset" 2D textures for slices
 */
void GWindow::clearSlices()
{
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, 0);
}

/**
 * @brief GWindow::changeColormap Change colormap
 * @param colormap
 */
void GWindow::changeColormap(ColorMap::Type colormap)
{
    //this->colormap = colormap;
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, ColorMap::data[colormap]);
    glBindTexture(GL_TEXTURE_1D, 0);
    renderLater();
}

void GWindow::changeOpacity(QVector<float> opacity)
{
    //this->colormap = colormap;
    glBindTexture(GL_TEXTURE_1D, opacityTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, opacity.length(), 0, GL_RED, GL_FLOAT, opacity.data());
    glBindTexture(GL_TEXTURE_1D, 0);
    renderLater();
}

/**
 * @brief GWindow::changeMinValue Set min value for colormaping
 * @param value
 */
void GWindow::changeMinValue(float value)
{
    //minG = value;
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uMin, value);
        m_program->release();
    }
    renderLater();
}

/**
 * @brief GWindow::changeMaxValue Set max value for colormaping
 * @param value
 */
void GWindow::changeMaxValue(float value)
{
    //maxG = value;
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uMax, value);
        m_program->release();
    }
    renderLater();
}

void GWindow::changeMode(int mode)
{
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uMode, mode);
        m_program->release();
    }
    renderLater();
}

void GWindow::changeInterpolation(int mode)
{
    int glMode = GL_LINEAR;
    if (mode == 1)
        glMode = GL_NEAREST;

    if (initialized) {
        glBindTexture(GL_TEXTURE_3D, texture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_3D, 0);
    }
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
    glDrawElements(GL_LINE_LOOP, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0); // 3*12
    glDisableVertexAttribArray(m_aPosition);
    vboCubeVertices.release();
    glBindVertexArray(0);
}

void GWindow::renderBox()
{
    glBindVertexArray(vao);
    vboCubeVertices.bind();
    glEnableVertexAttribArray(m_aPosition);
    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCubeElements);
    glDrawElements(GL_TRIANGLES, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0); // 3*12
    glDisableVertexAttribArray(m_aPosition);
    vboCubeVertices.release();
    glBindVertexArray(0);
}

/**
 * @brief GWindow::convertPointToOpenGLRelative
 * @param point
 * @return
 */
QPointF GWindow::convertPointToOpenGLRelative(QPointF point)
{
    QPointF pointOutput;
    pointOutput.setX((point.x() / float(width()) - 0.5f) * 2.0f);
    pointOutput.setY((point.y() / float(height()) - 0.5f) * 2.0f);
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
    return float((floor(number * (1.0f / precision) + 0.5) / (1.0f / precision)));
}

/**
 * @brief GWindow::render Main render function
 */
void GWindow::render()
{
    if (checkGlError() == GL_OUT_OF_MEMORY) {
        thread->stopCurrentBlockReading();
        emit loaded(selectedDataset->getName());
        unload3DTexture();
        volumeRendering = false;
    }

    // Rotation of scene by left mouse click
    if (leftButton) {
        // TODO
        rotateXMatrix.rotate(float(lastPositionPressed.y() - currentPositionPressed.y()) / 2.0f, -1, 0, 0);
        rotateYMatrix.rotate(float(lastPositionPressed.x() - currentPositionPressed.x()) / 2.0f, 0, -1, 0);
        //actualCount = 30;
    }

    // Move of scene by right mouse click
    if (rightButton) {
        QPointF lastPosRl = convertPointToOpenGLRelative(lastPositionPressed);
        QPointF currentPosRl = convertPointToOpenGLRelative(currentPositionPressed);
        position.setX(position.x() - float(lastPosRl.x() - currentPosRl.x()));
        position.setY(position.y() + float(lastPosRl.y() - currentPosRl.y()));
        //actualCount = 30;
    }

    // Clear viewport
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Activate shader program
    m_program->bind();

    // Zoom
    if (wheelDelta > 0)
        zoom *= 1.0f / 1.1f;
    else if (wheelDelta < 0)
        zoom *= 1.1f;

    // Projection
    QMatrix4x4 projectionMatrix;
    float ratio = float(width()) / float(height());

    if (orthogonal) {
        if (zoom > 200.0f) {
            zoom = 200.0f;
        }
        if (zoom < 0.00001f) {
            zoom = 0.00001f;
        }
        projectionMatrix.translate(position.x(), position.y(), 0);
        projectionMatrix.ortho(-ratio * zoom, ratio * zoom, -zoom, zoom, -1.0f, 1000.0f);

    } else {
        float angle = zoom * 45.0f;
        if (angle > 179.9999f) {
            angle = 179.9999f;
            zoom = 179.9999f / 45.0f;
        }
        if (angle < 0.00001f) {
            angle = 0.00001f;
            zoom = 0.00001f / 45.0f;
        }
        projectionMatrix.translate(position.x(), position.y(), 0);
        projectionMatrix.perspective(angle, ratio, 0.1f, 100.0f);
        projectionMatrix.translate(0, 0, -2.0f);
    }

    // Final matrix
    QMatrix4x4 matrix;
    matrix = projectionMatrix * rotateXMatrix * rotateYMatrix;


    // Create vectors for scale 3D frame to the longest size = 1.0f
    float fullMax = qMax(fullSize.x(), qMax(fullSize.y(), fullSize.z()));

    QVector3D fullSizeScaled = fullSize / fullMax; // the longest size is 1.0f
    QVector3D imageSizeScaled = imageSize / fullMax;
    QVector3D imagePositionScaled = imagePosition / fullMax;

    // Translate to the midlle of 3D frame
    matrix.translate(-fullSizeScaled.x() / 2.0f, -fullSizeScaled.y() / 2.0f, -fullSizeScaled.z() / 2.0f);

    float offset = 0.01f;

    // Send matrix to shader
    m_program->setUniformValue(m_uMatrix, matrix);
    m_program->setUniformValue(m_uSliceMatrix, QMatrix4x4());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboFront, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboBack, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Trim values?
    if (trim) {
        m_program->setUniformValue(m_uTrim, true);
    } else {
        m_program->setUniformValue(m_uTrim, false);
    }

    // Frame
    if (frame) {
        m_program->setUniformValue(m_uFrame, true);

        QMatrix4x4 sMatrix;
        QMatrix4x4 rMatrix;

        rMatrix.scale(1 + 2 * offset);
        rMatrix.translate(-offset, -offset, -offset);

        // Smaller frame
        sMatrix.translate(imagePositionScaled);
        sMatrix.scale(imageSizeScaled);
        m_program->setUniformValue(m_uMatrix, matrix * sMatrix * rMatrix);
        renderFrame();

        // Big frame
        if (imageSize != fullSize) {
            QMatrix4x4 sMatrix;
            sMatrix.scale(fullSizeScaled);
            m_program->setUniformValue(m_uMatrix, matrix * sMatrix * rMatrix);
            renderFrame();
        }

        m_program->setUniformValue(m_uFrame, false);
    }

    // Slices
    if (sliceXY || sliceXZ || sliceYZ) {
        m_program->setUniformValue(m_uSlices, true);

        QMatrix4x4 imageMatrix;
        imageMatrix.translate(imagePositionScaled);
        imageMatrix.scale(imageSizeScaled);

        QMatrix4x4 offsetMatrix;
        offsetMatrix.scale(1 + 2 * offset);
        offsetMatrix.translate(-offset, -offset, 0);

        // Rotate and translate by slice index
        QMatrix4x4 translateXYMatrix;
        translateXYMatrix.translate(0, 0, index.z());
        QMatrix4x4 translateXZMatrix;
        translateXZMatrix.translate(0, index.y(), 0);
        translateXZMatrix.rotate(90, 1, 0, 0);
        QMatrix4x4 translateYZMatrix;
        translateYZMatrix.translate(index.x(), 0, 0);
        translateYZMatrix.rotate(90, 1, 0, 0);
        translateYZMatrix.rotate(90, 0, 1, 0);
        //translateYZMatrix.scale(-1, 1, 1);

        glBindVertexArray(vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboSliceElements);

        glEnableVertexAttribArray(m_aPosition);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        glActiveTexture(GL_TEXTURE2);

        vboSliceVertices.bind();

        if (sliceXY) {
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindTexture(GL_TEXTURE_2D, textureXY);

            // Draw slice
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXYMatrix);
            m_program->setUniformValue(m_uSliceMatrix, translateXYMatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);

            m_program->setUniformValue(m_uVolumeRenderingBox, true);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_program->setUniformValue(m_uVolumeRenderingBox, false);

            // Draw 2D frame
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXYMatrix * offsetMatrix);
            m_program->setUniformValue(m_uXYBorder, true);
            glDrawElements(GL_LINE_LOOP,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXYBorder, false);
        }

        if (sliceXZ) {
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindTexture(GL_TEXTURE_2D, textureXZ);

            // Draw slice
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXZMatrix);
            m_program->setUniformValue(m_uSliceMatrix, translateXZMatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);

            m_program->setUniformValue(m_uVolumeRenderingBox, true);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_program->setUniformValue(m_uVolumeRenderingBox, false);

            // Draw 2D frame
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXZMatrix * offsetMatrix);
            m_program->setUniformValue(m_uXZBorder, true);
            glDrawElements(GL_LINE_LOOP,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXZBorder, false);
        }

        if (sliceYZ) {
            glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindTexture(GL_TEXTURE_2D, textureYZ);

            // Draw slice
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateYZMatrix);
            m_program->setUniformValue(m_uSliceMatrix, translateYZMatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);

            m_program->setUniformValue(m_uVolumeRenderingBox, true);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_program->setUniformValue(m_uVolumeRenderingBox, false);

            // Draw 2D frame
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateYZMatrix * offsetMatrix);
            m_program->setUniformValue(m_uYZBorder, true);
            glDrawElements(GL_LINE_LOOP,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uYZBorder, false);
        }

        vboSliceVertices.release();

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_1D, 0);

        glDisableVertexAttribArray(m_aPosition);

        glBindVertexArray(0);

        m_program->setUniformValue(m_uSliceMatrix, QMatrix4x4());
        m_program->setUniformValue(m_uSlices, false);
    }

    // Volume rendering
    if (volumeRendering) {
        QMatrix4x4 sMatrix;
        sMatrix.translate(imagePositionScaled);
        sMatrix.scale(imageSizeScaled);
        m_program->setUniformValue(m_uMatrix, matrix * sMatrix);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        m_program->setUniformValue(m_uVolumeRenderingBox, true);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        renderBox();
        glCullFace(GL_FRONT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboFront, 0);
        renderBox();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_program->setUniformValue(m_uVolumeRenderingBox, false);


        m_program->setUniformValue(m_uVolumeRendering, true);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D, opacityTexture);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureFboBack);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textureFboFront);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texture);

        renderBox();

        glBindTexture(GL_TEXTURE_3D, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_1D, 0);

        m_program->setUniformValue(m_uVolumeRendering, false);

        glDisable(GL_CULL_FACE);
    }

    m_program->release();
}

// Some slots for user interaction

void GWindow::saveImage(QString fileName)
{
    // Save 3D scene to png image
    QImage image(width(), height(), QImage::Format_RGBA8888);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    image = image.mirrored();
    image.save(fileName);
    //QLabel myLabel;
    //myLabel.setPixmap(QPixmap::fromImage(image));
    //myLabel.show();
}

QImage GWindow::getImage()
{
    QImage image(width(), height(), QImage::Format_RGB888);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, image.bits());
    return image.mirrored();
}

void GWindow::setViewFrame(bool value)
{
    frame = value;
    renderLater();
}

void GWindow::setViewVR(bool value)
{
    volumeRendering = value;
    renderLater();
}

void GWindow::setSlicesCount(int value)
{
    steps = value;
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uSteps, steps);
        m_program->release();
    }
    renderLater();
}

void GWindow::setViewXYSlice(bool value)
{
    sliceXY = value;
    renderLater();
}

void GWindow::setViewXZSlice(bool value)
{
    sliceXZ = value;
    renderLater();
}

void GWindow::setViewYZSlice(bool value)
{
    sliceYZ = value;
    renderLater();
}

void GWindow::setTrim(bool value)
{
    trim = value;
    renderLater();
}

void GWindow::setOrthogonal(bool value)
{
    orthogonal = value;
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
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (key->key() == Qt::Key_Plus) {
            steps = steps + 2;
            if (steps < 3) steps = 3;
            emit setStatusMessage(QString("Slices: %1").arg(steps));
        }
        if (key->key() == Qt::Key_Minus) {
            steps = steps - 2;
            if (steps < 3) steps = 3;
            emit setStatusMessage(QString("Slices: %1").arg(steps));
        }
        if (key->key() == Qt::Key_F) {
            if (frame)
                frame = false;
            else
                frame = true;
            emit setStatusMessage(QString("Frame: %1").arg(frame));
        }
        if (key->key() == Qt::Key_T) {
            if (trim)
                trim = false;
            else
                trim = true;
            emit setStatusMessage(QString("Trim: %1").arg(trim));
        }
        if (key->key() == Qt::Key_Z) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
        }
        if (key->key() == Qt::Key_Y) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(90, 1, 0, 0);
            rotateYMatrix.setToIdentity();
        }
        if (key->key() == Qt::Key_X) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
            rotateYMatrix.rotate(90, 0, -1, 0);
        }
        if (key->key() == Qt::Key_B) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(180, 0, -1, 0);
            rotateYMatrix.setToIdentity();
        }
        if (key->key() == Qt::Key_G) {
            rotateXMatrix.setToIdentity();
            rotateXMatrix.rotate(90 + 180, 1, 0, 0);
            rotateYMatrix.setToIdentity();
        }
        if (key->key() == Qt::Key_R) {
            rotateXMatrix.setToIdentity();
            rotateYMatrix.setToIdentity();
            rotateYMatrix.rotate(90 + 180, 0, -1, 0);
        }
        if (key->key() == Qt::Key_C) {
            position.setX(0);
            position.setY(0);
        }
        if (key->key() == Qt::Key_O) {
            setOrthogonal(!orthogonal);
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

void GWindow::resizeEvent(QResizeEvent *)
{
    if (initialized) {
        // Resize framebuffer texture
        glBindTexture(GL_TEXTURE_2D, textureFboBack);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, textureFboFront);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        m_program->bind();
        m_program->setUniformValue(m_uWidth, float(width()));
        m_program->setUniformValue(m_uHeight, float(height()));
        m_program->release();

    }

    if (isExposed())
        renderNow();
}
