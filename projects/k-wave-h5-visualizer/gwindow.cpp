/**
 * @file        gwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing GWindow class definition.
 *
 * 3D scene window.
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

#include "gwindow.h"
#include "vertices.h"

/**
 * @brief Creates GWindow
 * @param[in] qMainWindow QMainWindow (optional)
 */
GWindow::GWindow(QMainWindow *qMainWindow)
    : m_program(0)
    , initialized(false)

{
    this->qMainWindow = qMainWindow;

    // Create thread for loading whole dataset
    thread = new H5ReadingThread();
    connect(thread, SIGNAL(requestDone(Request *)), this, SLOT(set3DData(Request *)));
}

/**
 * @brief Destructor of GWindow object
 *
 * Deletes buffers, etc.
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
 * @brief Initializes OpenGL window
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

    setColormap();

    setOpacity();

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

    m_program->setUniformValue(m_uFrameColor, 1.0f - float(colorW.redF()), 1.0f - float(colorW.greenF()), 1.0f - float(colorW.blueF()), 1.0f);

    m_program->setUniformValue(m_uSlices, false);

    m_program->setUniformValue(m_uVolumeRendering, false);
    m_program->setUniformValue(m_uVolumeRenderingBox, false);

    m_program->setUniformValue(m_uXYBorder, false);
    m_program->setUniformValue(m_uXZBorder, false);
    m_program->setUniformValue(m_uYZBorder, false);

    m_program->setUniformValue(m_uSteps, slicesCount);

    m_program->setUniformValue(m_uWidth, float(width()));
    m_program->setUniformValue(m_uHeight, float(height()));

    m_program->release();

    initialized = true;
    renderLater();
}

/**
 * @brief Returns 3D data loading thread
 * @return 3D data loading thread
 */
H5ReadingThread *GWindow::getThread()
{
    return thread;
}

/**
 * @brief Sets size for main 3D frame - whole 3D domain
 * @param[in] size Size
 */
void GWindow::setFrameSize(H5Helper::Vector3D size)
{
    setFrameSize(QVector3DI(size.x(), size.y(), size.z()));
}

void GWindow::setFrameSize(QVector3DI size)
{
    frameSize = size;
}

/**
 * @brief Sets size for original (sensor mask defined) frame
 * @param[in] size Size
 */
void GWindow::setDatasetSize(H5Helper::Vector3D size)
{
    setDatasetSize(QVector3DI(size.x(), size.y(), size.z()));
}

void GWindow::setDatasetSize(QVector3DI size)
{
    datasetSize = size;
}

/**
 * @brief Sets position of sensor mask defined 3D dataset
 * @param[in] position
 */
void GWindow::setDatasetPosition(H5Helper::Vector3D position)
{
    setDatasetPosition(QVector3DI(position.x(), position.y(), position.z()));
}

void GWindow::setDatasetPosition(QVector3DI position)
{
    datasetPosition = position;
}

/**
 * @brief Performs loading of 3D data for volume rendering
 * @param[in] dataset Dataset
 * @param[in] step Step
 */
void GWindow::load3DData()
{
    if (volumeRendering) {
        if (object != 0) {
            thread->createRequest(object->getDataset(), object->getCurrentStep());
            thread->start();
            data3DloadedFlag = false;
            emit data3Dloading();
        }
    }
}

/**
 * @brief Unloads 3D texture data
 */
void GWindow::unload3DTexture()
{
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

/**
 * @brief Clears 3D data, slices data and reset sizes
 */
void GWindow::clear()
{
    object = 0;

    if (initialized) {
        setMinValue(0.0f);
        setMaxValue(0.0f);
        setColormap();
        setOpacity();

        setFrameSize(QVector3DI(1, 1, 1));
        setDatasetSize(QVector3DI(1, 1, 1));
        setDatasetPosition(QVector3DI(0, 0, 0));

        clearSlices();
        unload3DTexture();

        renderLater();
    }
}

/**
 * @brief Sets part of 3D data from loading request to 3D texture
 * @param[in] r Loading request
 */
void GWindow::set3DData(Request *request)
{
    if (object != 0 && request->dataset == object->getDataset()) {
        H5Helper::Vector3D offset = request->offset;
        H5Helper::Vector3D count = request->count;

        if (offset.hasZeros()) {
            // Init 3D texture
            glBindTexture(GL_TEXTURE_3D, texture);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, datasetSize.x(), datasetSize.y(), datasetSize.z(), 0, GL_RED, GL_FLOAT, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
            // Check OUT_OF_MEMORY, dataset is too big
            if (checkGlError() != GL_NO_ERROR) {
                thread->stopCurrentBlockReading();
                thread->deleteDoneRequest(request);
                data3DloadedFlag = true;
                emit data3Dloaded();
                unload3DTexture();
                return;
            }
        }

        // Set 3D data to 3D texture
        glBindTexture(GL_TEXTURE_3D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage3D(GL_TEXTURE_3D, 0, int(offset.x()), int(offset.y()), int(offset.z()), int(count.x()), int(count.y()), int(count.z()), GL_RED, GL_FLOAT, request->data);
        glBindTexture(GL_TEXTURE_3D, 0);

        // Last block of 3D data
        if (offset.z() + count.z() == hsize_t(datasetSize.z())) {
            renderLater();
            data3DloadedFlag = true;
            emit data3Dloaded();
        }
        thread->deleteDoneRequest(request);
    } else {
        //thread->stopCurrentBlockReading();
        thread->deleteDoneRequest(request);
    }
}

/**
 * @brief Frees or resets 2D textures for slices
 */
void GWindow::clearSlices()
{
    index.setX(0.5);
    index.setY(0.5);
    index.setZ(0.5);
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
 * @brief Changes colormap
 * @param[in] colormap Colormap
 */
void GWindow::setColormap(ColorMap::Type colormap)
{
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, ColorMap::data[colormap]);
    glBindTexture(GL_TEXTURE_1D, 0);
    renderLater();
}

/**
 * @brief Changes opacity
 * @param[in] opacity Opacity
 */
void GWindow::setOpacity(QVector<float> value)
{
    glBindTexture(GL_TEXTURE_1D, opacityTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, value.length(), 0, GL_RED, GL_FLOAT, value.data());
    glBindTexture(GL_TEXTURE_1D, 0);
    renderLater();
}

void GWindow::setXYSlice(float *data, hsize_t index)
{
    float indexTmp = index;
    if (datasetSize.z() == 1) // Index -> 0
        indexTmp = 0;
    else
        indexTmp = float(index) / (datasetSize.z() - 1);

    //qDebug() << "setXYSlice";
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, datasetSize.x(), datasetSize.y(), 0, GL_RED, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setZ(indexTmp);
    renderLater();
}

void GWindow::setXZSlice(float *data, hsize_t index)
{
    float indexTmp = index;
    if (datasetSize.y() == 1) // Index -> 0
        indexTmp = float(0);
    else
        indexTmp = float(index) / (datasetSize.y() - 1);

    //qDebug() << "setXZSlice";
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, datasetSize.x(), datasetSize.z(), 0, GL_RED, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setY(indexTmp);
    renderLater();
}

void GWindow::setYZSlice(float *data, hsize_t index)
{
    float indexTmp = index;
    if (datasetSize.x() == 1) // Index -> 0
        indexTmp = float(0);
    else
        indexTmp = float(index) / (datasetSize.x() - 1);

    //qDebug() << "setYZSlice";
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, datasetSize.y(), datasetSize.z(), 0, GL_RED, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    this->index.setX(indexTmp);
    renderLater();
}

/**
 * @brief Sets min value for colormapping
 * @param[in] value Min value
 */
void GWindow::setMinValue(float value)
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
 * @brief Sets max value for colormapping
 * @param[in] value Max value
 */
void GWindow::setMaxValue(float value)
{
    //maxG = value;
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uMax, value);
        m_program->release();
    }
    renderLater();
}

/**
 * @brief Changes volume rendering mode
 * @param[in] mode Volume rendering mode
 */
void GWindow::setVolumeRenderingMode(int mode)
{
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uMode, mode);
        m_program->release();
    }
    renderLater();
}

/**
 * @brief Changes interpolation mode
 * @param[in] mode Interpolation mode (GL_LINEAR == 0 | GL_NEAREST == 1)
 */
void GWindow::setInterpolationMode(int mode)
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
 * @brief Renders 3D frame
 */
void GWindow::renderFrame()
{
    glBindVertexArray(vao);
    vboCubeVertices.bind();
    glEnableVertexAttribArray(GLuint(m_aPosition));
    glVertexAttribPointer(GLuint(m_aPosition), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCubeElements);
    glDrawElements(GL_LINE_LOOP, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0); // 3*12
    glDisableVertexAttribArray(GLuint(m_aPosition));
    vboCubeVertices.release();
    glBindVertexArray(0);
}

/**
 * @brief Renders Box
 */
void GWindow::renderBox()
{
    glBindVertexArray(vao);
    vboCubeVertices.bind();
    glEnableVertexAttribArray(GLuint(m_aPosition));
    glVertexAttribPointer(GLuint(m_aPosition), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCubeElements);
    glDrawElements(GL_TRIANGLES, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, 0); // 3*12
    glDisableVertexAttribArray(GLuint(m_aPosition));
    vboCubeVertices.release();
    glBindVertexArray(0);
}

/**
 * @brief Converts point to OpenGL relative
 * @param[in] point Point to convert
 * @return Converted point
 */
QPointF GWindow::convertPointToOpenGLRelative(QPointF point)
{
    QPointF pointOutput;
    pointOutput.setX((point.x() / double(width()) - 0.5) * 2.0);
    pointOutput.setY((point.y() / double(height()) - 0.5) * 2.0);
    return pointOutput;
}

/**
 * @brief Round function
 * @param[in] number Number
 * @param[in] precision Precision
 * @return Rounded number
 */
float GWindow::round(float number, float precision)
{
    return float((floor(number * (1.0f / precision) + 0.5f) / (1.0f / precision)));
}

bool GWindow::isVolumeRenderingEnabled() const
{
    return volumeRendering;
}

bool GWindow::aredata3Dloaded() const
{
    return data3DloadedFlag;
}

void GWindow::setObject(H5ObjectToVisualize *value)
{
    object = value;
    connect(object, SIGNAL(opacityChanged(QVector<float>)), this, SLOT(setOpacity(QVector<float>)));
    connect(object, SIGNAL(dataXYChanged(float *, hsize_t)), this, SLOT(setXYSlice(float *, hsize_t)));
    connect(object, SIGNAL(dataXZChanged(float *, hsize_t)), this, SLOT(setXZSlice(float *, hsize_t)));
    connect(object, SIGNAL(dataYZChanged(float *, hsize_t)), this, SLOT(setYZSlice(float *, hsize_t)));
    connect(object, SIGNAL(minValueChanged(float)), this, SLOT(setMinValue(float)));
    connect(object, SIGNAL(maxValueChanged(float)), this, SLOT(setMaxValue(float)));
    connect(object, SIGNAL(colormapChanged(ColorMap::Type)), this, SLOT(setColormap(ColorMap::Type)));
    connect(object, SIGNAL(stepChanged()), this, SLOT(load3DData()));

    setMinValue(object->getMinValue());
    setMaxValue(object->getMaxValue());
    setColormap(object->getColormap());
    setOpacity(object->getOpacity());

    setFrameSize(object->getFrameSize());
    setDatasetSize(object->getSize());
    setDatasetPosition(object->getPos());

    load3DData();
}

/**
 * @brief Main render function
 */
void GWindow::render()
{
    if (checkGlError() == GL_OUT_OF_MEMORY) {
        thread->stopCurrentBlockReading();
        data3DloadedFlag = true;
        emit data3Dloaded();
        unload3DTexture();
    }

    // Rotation of scene by left mouse click
    if (this->getLeftButtonPressed()) {
        // TODO
        rotateXMatrix.rotate(float(this->getLastPositionPressed().y() - this->getCurrentPositionPressed().y()) / 2.0f, -1, 0, 0);
        rotateYMatrix.rotate(float(this->getLastPositionPressed().x() - this->getCurrentPositionPressed().x()) / 2.0f, 0, -1, 0);
        //actualCount = 30;
    }

    // Move of scene by right mouse click
    if (this->getRightButtonPressed()) {
        QPointF lastPosRl = convertPointToOpenGLRelative(this->getLastPositionPressed());
        QPointF currentPosRl = convertPointToOpenGLRelative(this->getCurrentPositionPressed());
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
    if (this->getWheelDelta() > 0)
        zoom *= 1.0f / 1.1f;
    else if (this->getWheelDelta() < 0)
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

    QVector3DI size = fillSpace ? datasetSize : frameSize;
    QVector3DI pos = fillSpace ? QVector3DI(0, 0, 0) : datasetPosition;

    // Create vectors for scale 3D frame to the longest size = 1.0f
    float fullMax = qMax(size.x(), qMax(size.y(), size.z()));

    QVector3D fullSizeScaled = size / fullMax; // the longest size is 1.0f
    QVector3D imageSizeScaled = datasetSize / fullMax;
    QVector3D imagePositionScaled = pos / fullMax;

    // Translate to the middle of 3D frame
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
        if (datasetSize != size) {
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

        glEnableVertexAttribArray(GLuint(m_aPosition));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        glActiveTexture(GL_TEXTURE2);

        vboSliceVertices.bind();

        if (sliceXY) {
            glVertexAttribPointer(GLuint(m_aPosition), 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindTexture(GL_TEXTURE_2D, textureXY);

            // Draw slice
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXYMatrix);
            m_program->setUniformValue(m_uSliceMatrix, translateXYMatrix);
            glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);

            if (!trim) {
                m_program->setUniformValue(m_uVolumeRenderingBox, true);
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glDrawElements(GL_TRIANGLES,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                m_program->setUniformValue(m_uVolumeRenderingBox, false);
            }

            // Draw 2D frame
            m_program->setUniformValue(m_uMatrix, matrix * imageMatrix * translateXYMatrix * offsetMatrix);
            m_program->setUniformValue(m_uXYBorder, true);
            glDrawElements(GL_LINE_LOOP,  sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
            m_program->setUniformValue(m_uXYBorder, false);
        }

        if (sliceXZ) {
            glVertexAttribPointer(GLuint(m_aPosition), 3, GL_FLOAT, GL_FALSE, 0, 0);
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
            glVertexAttribPointer(GLuint(m_aPosition), 3, GL_FLOAT, GL_FALSE, 0, 0);
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

        glDisableVertexAttribArray(GLuint(m_aPosition));

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

/**
 * @brief Saves image
 * @param[in] fileName Filename
 */
void GWindow::saveImage()
{
    QString name = "no_name";
    if (object != 0) {
        QString fileName = object->getOpenedH5File()->getFilename();
        QString objectName = object->getOnlyName();
        name = fileName + "_" + objectName + "_" + objectName + "_3Dscene.png";
    }
    QString imagefileName = QFileDialog::getSaveFileName(0, "Save image", name, "Image (*.png)");

    if (imagefileName != 0) {
        // Save 3D scene to png image
        QImage image = getImage();
        image.save(imagefileName);
    }
}

/**
 * @brief Returns image of OpenGL window
 * @return Image
 */
QImage GWindow::getImage()
{
    QImage image(width(), height(), QImage::Format_RGBA8888);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    return image.mirrored();
}

/**
 * @brief Set view frame?
 * @param[in] value True/False
 */
void GWindow::setViewFrame(bool value)
{
    frame = value;
    renderLater();
}

/**
 * @brief Set view Volume rendering?
 * @param[in] value True/False
 */
void GWindow::setViewVolumeRendering(bool value)
{
    volumeRendering = value;
    load3DData();
    renderLater();
}

/**
 * @brief Sets slices count
 * @param[in] value Slice count
 */
void GWindow::setSlicesCount(int value)
{
    slicesCount = value;
    if (initialized) {
        m_program->bind();
        m_program->setUniformValue(m_uSteps, slicesCount);
        m_program->release();
    }
    renderLater();
}

/**
 * @brief Set view XY slice?
 * @param[in] value True/False
 */
void GWindow::setViewXYSlice(bool value)
{
    sliceXY = value;
    renderLater();
}

/**
 * @brief Set view XZ slice?
 * @param[in] value True/False
 */
void GWindow::setViewXZSlice(bool value)
{
    sliceXZ = value;
    renderLater();
}

/**
 * @brief Set view YZ slice?
 * @param[in] value True/False
 */
void GWindow::setViewYZSlice(bool value)
{
    sliceYZ = value;
    renderLater();
}

/**
 * @brief Set trim?
 * @param[in] value True/False
 */
void GWindow::setTrim(bool value)
{
    trim = value;
    renderLater();
}

/**
 * @brief Set orthogonal projection?
 * @param[in] value True/False
 */
void GWindow::setOrthogonal(bool value)
{
    orthogonal = value;
    renderLater();
}

void GWindow::setFillSpace(bool value)
{
    fillSpace = value;
    renderLater();
}

/**
 * @brief Aligns to XY
 */
void GWindow::alignToXY()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    renderLater();
}

/**
 * @brief Aligns to XZ
 */
void GWindow::alignToXZ()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(90, 1, 0, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

/**
 * @brief Aligns to YZ
 */
void GWindow::alignToYZ()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    rotateYMatrix.rotate(90, 0, -1, 0);
    renderLater();
}

/**
 * @brief Aligns to XY from back
 */
void GWindow::alignToXYFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(180, 0, -1, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

/**
 * @brief Aligns to XZ from back
 */
void GWindow::alignToXZFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateXMatrix.rotate(90 + 180, 1, 0, 0);
    rotateYMatrix.setToIdentity();
    renderLater();
}

/**
 * @brief Aligns to YZ from back
 */
void GWindow::alignToYZFromBack()
{
    rotateXMatrix.setToIdentity();
    rotateYMatrix.setToIdentity();
    rotateYMatrix.rotate(90 + 180, 0, -1, 0);
    renderLater();
}

/**
 * @brief Catches keyboard events on 3D scene
 * @param[in] event Event
 * @return QWindow::event(event);
 */
bool GWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
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
        if (dynamic_cast<QMouseEvent *>(event)->buttons() == Qt::MiddleButton) {
            position.setX(0);
            position.setY(0);
            renderLater();
        }
    default:
        return OpenGLWindow::event(event);
    }
}

/**
 * @brief Resize event
 */
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
