/**
 * @file        gwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
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
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <gwindow.h>
#include <vertices.h>

/**
 * @brief Creates GWindow
 * @param[in] qMainWindow QMainWindow (optional)
 */
GWindow::GWindow(QWidget *parent)
{
    this->parent = parent;
}

/**
 * @brief Destructor of GWindow object
 *
 * Deletes buffers, etc.
 */
GWindow::~GWindow()
{
    iboSliceElements.destroy();
    iboCubeElements.destroy();
    vboCubeVertices.destroy();
    vboSliceVertices.destroy();

    glDeleteVertexArrays(1, &vao);

    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &textureLC);
    glDeleteTextures(1, &textureCC);
    glDeleteTextures(1, &textureCC);
    glDeleteTextures(1, &textureBE);
    glDeleteTextures(1, &textureBE_1);
    glDeleteTextures(1, &textureBufferBE);
    glDeleteTextures(1, &textureBufferBE_1);
    glDeleteTextures(1, &colormapTexture);
    glDeleteTextures(1, &opacityTexture);
    glDeleteTextures(1, &textureXY);
    glDeleteTextures(1, &textureXZ);
    glDeleteTextures(1, &textureYZ);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);

    program->release();
    delete program;
    program = nullptr;
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
 * @brief Set view Volume rendering?
 * @param[in] value True/False
 */
void GWindow::setViewVolumeRendering(bool value)
{
    volumeRendering = value;
    emit viewVolumeRenderingChanged(value);
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
 * @brief Set view frame?
 * @param[in] value True/False
 */
void GWindow::setViewFrame(bool value)
{
    frame = value;
    renderLater();
}

/**
 * @brief Set fill space?
 * @param[in] value True/False
 */
void GWindow::setFillSpace(bool value)
{
    fillSpace = value;
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
 * @brief Sets slices count
 * @param[in] value Slice count
 */
void GWindow::setSlicesCount(int value)
{
    slicesCount = value;
    if (initialized) {
        program->bind();
        program->setUniformValue(uSteps, slicesCount);
        program->release();
    }
    renderLater();
}

/**
 * @brief Changes volume rendering mode
 * @param[in] mode Volume rendering mode
 */
void GWindow::setVolumeRenderingMode(int mode)
{
    vRMode = mode;
    if (initialized) {
        program->bind();
        program->setUniformValue(uMode, vRMode);
        program->release();
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
    intMode = glMode;
    if (initialized) {
        glBindTexture(GL_TEXTURE_3D, texture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_3D, 0);

        glBindTexture(GL_TEXTURE_3D, textureLC);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_3D, 0);

        glBindTexture(GL_TEXTURE_3D, textureCC);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_3D, 0);

        glBindTexture(GL_TEXTURE_2D, textureXY);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, textureXZ);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, textureYZ);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMode);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    renderLater();
}

/**
 * @brief Sets object
 * @param[in] object H5ObjectToVisualize
 */
void GWindow::setObject(H5ObjectToVisualize *object)
{
    if (initialized) {
        connect(object, SIGNAL(minValueChanged(float)), this, SLOT(setMinValue(float)));
        connect(object, SIGNAL(maxValueChanged(float)), this, SLOT(setMaxValue(float)));
        connect(object, SIGNAL(colormapChanged(ColorMap::Type)), this, SLOT(setColormap(ColorMap::Type)));
        connect(object, SIGNAL(opacityChanged(QVector<float>)), this, SLOT(setOpacity(QVector<float>)));
        connect(object, SIGNAL(minMaxValuesTrimChanged(bool)), this, SLOT(setTrim(bool)));

        connect(object, SIGNAL(data3DChanged(float *)), this, SLOT(set3DData(float *)));
        connect(object, SIGNAL(data3DCompressChanged(float *, float *, hsize_t)), this, SLOT(set3DCompressData(float *, float *, hsize_t)));
        connect(object, SIGNAL(dataXYChanged(float *, hsize_t)), this, SLOT(setXYSlice(float *, hsize_t)));
        connect(object, SIGNAL(dataXZChanged(float *, hsize_t)), this, SLOT(setXZSlice(float *, hsize_t)));
        connect(object, SIGNAL(dataYZChanged(float *, hsize_t)), this, SLOT(setYZSlice(float *, hsize_t)));

        connect(this, SIGNAL(viewVolumeRenderingChanged(bool)), object, SLOT(setData3DLoadingFlag(bool)));

        setCompressRendering(false);

        setMinValue(object->getMinValue());
        setMaxValue(object->getMaxValue());
        setColormap(object->getColormap());
        setOpacity(object->getOpacity());
        setTrim(object->getMinMaxValuesTrim());

        setFrameSize(object->getFrameSize());
        setDatasetSize(object->getSize());
        setDatasetPosition(object->getPos());

        setXYSlice(object->getDataXY(), object->getZIndex());
        setXZSlice(object->getDataXZ(), object->getYIndex());
        setYZSlice(object->getDataYZ(), object->getXIndex());

        object->setData3DLoadingFlag(volumeRendering);

        if (object->getCompressHelper()) {
            compressHelper = object->getCompressHelper();
            set3DData();

            glBindBuffer(GL_TEXTURE_BUFFER, textureBufferBE);
            glBufferData(GL_TEXTURE_BUFFER,
                         sizeof(GLfloat) * GLuint(compressHelper->getStride() * compressHelper->getBSize()),
                         reinterpret_cast<float *>(compressHelper->getBE()),
                         GL_STATIC_DRAW);
            glBindBuffer(GL_TEXTURE_BUFFER, 0);
            glBindTexture(GL_TEXTURE_BUFFER, textureBE);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, textureBufferBE);
            glBindTexture(GL_TEXTURE_BUFFER, 0);

            glBindBuffer(GL_TEXTURE_BUFFER, textureBufferBE_1);
            glBufferData(GL_TEXTURE_BUFFER,
                         sizeof(GLfloat) * GLuint(compressHelper->getStride() * compressHelper->getBSize()),
                         reinterpret_cast<float *>(compressHelper->getBE_1()),
                         GL_STATIC_DRAW);
            glBindBuffer(GL_TEXTURE_BUFFER, 0);
            glBindTexture(GL_TEXTURE_BUFFER, textureBE_1);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, textureBufferBE_1);
            glBindTexture(GL_TEXTURE_BUFFER, 0);

            program->bind();
            program->setUniformValue(uHarmonics, int(compressHelper->getHarmonics()));
            program->setUniformValue(uBSize2, int(compressHelper->getBSize() * 2));
            program->release();

            if (object->areCurrentData3DLoaded()) {
                set3DCompressData(object->getData3DLC(), object->getData3DCC(), object->getLocalStep() * 2);
            } else {
                set3DCompressData();
            }
            setCompressRendering(true);
        } else {
            set3DCompressData();
            if (object->areCurrentData3DLoaded()) {
                set3DData(object->getData3D());
            } else {
                set3DData();
            }
        }
        renderLater();
    }
}

/**
 * @brief Clears 3D data, slices data and reset sizes
 */
void GWindow::clear()
{
    if (initialized) {
        disconnect(this, SIGNAL(viewVolumeRenderingChanged(bool)), nullptr, nullptr);

        setCompressRendering(false);

        setMinValue();
        setMaxValue();
        setColormap();
        setOpacity();
        setTrim();

        setFrameSize();
        setDatasetSize();
        setDatasetPosition();

        unloadSlicesTextures();
        unload3DTexture();
        unload3DCompressTexture();

        renderLater();
    }
}

/**
 * @brief Saves image
 * @param[in] filename Filename
 */
void GWindow::saveImage()
{
    QString name = "no_name";
    /*if (object) {
        QString filename = QString::fromStdString(object->getFile()->getFilename());
        QString objectName = object->getOnlyName();
        name = filename + "_" + objectName + "_3Dscene";
    }*/
    QString imagefilename = QFileDialog::getSaveFileName(nullptr, "Save image", name + ".png", "Image (*.png)");

    if (!imagefilename.isEmpty()) {
        // Save 3D scene to png image
        QImage image = getImage();
        image.save(imagefilename, nullptr, 100);
    }
}

/**
 * @brief Initializes OpenGL window
 */
void GWindow::initialize()
{
    // Load, create and link shaders
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexShader");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentShader");
    program->link();

    // Init attribute variables
    aPosition = program->attributeLocation("aPosition");

    // Init uniform variables
    uMatrix = program->uniformLocation("uMatrix");
    uSliceMatrix = program->uniformLocation("uSliceMatrix");

    uVolumeTexture = program->uniformLocation("uVolume");
    uVolumeTextureLC = program->uniformLocation("uVolumeLC");
    uVolumeTextureCC = program->uniformLocation("uVolumeCC");
    uTextureBE = program->uniformLocation("uTextureBE");
    uTextureBE_1 = program->uniformLocation("uTextureBE_1");
    uColormapTexture = program->uniformLocation("uColormap");
    uOpacityTexture = program->uniformLocation("uOpacity");
    uSliceTexture = program->uniformLocation("uSlice");
    uBoxBackSampler = program->uniformLocation("uBoxBackSampler");
    uBoxFrontSampler = program->uniformLocation("uBoxFrontSampler");

    uStepLocal2 = program->uniformLocation("uStepLocal2");
    uHarmonics = program->uniformLocation("uHarmonics");
    uBSize2 = program->uniformLocation("uBSize2");

    uFrame = program->uniformLocation("uFrame");
    uSlices = program->uniformLocation("uSlices");
    uXYBorder = program->uniformLocation("uXYBorder");
    uXZBorder = program->uniformLocation("uXZBorder");
    uYZBorder = program->uniformLocation("uYZBorder");
    uVolumeRenderingBox = program->uniformLocation("uVolumeRenderingBox");
    uVolumeRendering = program->uniformLocation("uVolumeRendering");
    uVolumeCompressRendering = program->uniformLocation("uVolumeCompressRendering");
    uTrim = program->uniformLocation("uTrim");
    uSteps = program->uniformLocation("uSteps");
    uFrameColor = program->uniformLocation("uFrameColor");
    uWidth = program->uniformLocation("uWidth");
    uHeight = program->uniformLocation("uHeight");
    uMin = program->uniformLocation("uMin");
    uMax = program->uniformLocation("uMax");
    uMode = program->uniformLocation("uMode");

    // Create index buffers
    // for slices
    iboSliceElements = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    iboSliceElements.create();
    iboSliceElements.bind();
    iboSliceElements.allocate(sliceElements, sizeof(sliceElements));
    iboSliceElements.release();
    // and for 3D frame
    iboCubeElements = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    iboCubeElements.create();
    iboCubeElements.bind();
    iboCubeElements.allocate(cubeElements, sizeof(cubeElements));
    iboCubeElements.release();

    // Create vertex buffers
    vboSliceVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboSliceVertices.create();
    vboSliceVertices.bind();
    vboSliceVertices.allocate(sliceVertices, sizeof(GLfloat) * 4 * 3);
    vboSliceVertices.release();

    vboCubeVertices = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboCubeVertices.create();
    vboCubeVertices.bind();
    vboCubeVertices.allocate(cubeVertices, sizeof(GLfloat) * 8 * 3);
    vboCubeVertices.release();

    // VAO
    glGenVertexArrays(1, &vao);

    // 3D texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);

    // 3D compression coefficients texture
    glGenTextures(1, &textureLC);
    glBindTexture(GL_TEXTURE_3D, textureLC);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);

    // 3D compression coefficients texture
    glGenTextures(1, &textureCC);
    glBindTexture(GL_TEXTURE_3D, textureCC);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);

    // Texture for compression basis
    glGenBuffers(1, &textureBufferBE);
    glBindBuffer(GL_TEXTURE_BUFFER, textureBufferBE);
    glBufferData(GL_TEXTURE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // Texture buffer for compression basis
    glGenTextures(1, &textureBE);
    glBindTexture(GL_TEXTURE_BUFFER, textureBE);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, textureBufferBE);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // Texture for compression basis
    glGenBuffers(1, &textureBufferBE_1);
    glBindBuffer(GL_TEXTURE_BUFFER, textureBufferBE_1);
    glBufferData(GL_TEXTURE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // Texture buffer for compression basis
    glGenTextures(1, &textureBE_1);
    glBindTexture(GL_TEXTURE_BUFFER, textureBE_1);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, textureBufferBE_1);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // Raycasting textures
    glGenTextures(1, &textureFboBack);
    glBindTexture(GL_TEXTURE_2D, textureFboBack);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureFboFront);
    glBindTexture(GL_TEXTURE_2D, textureFboFront);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureXZ);
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &textureYZ);
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, intMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Framebuffer for raycasting
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, textureFboBack);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, textureFboFront);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboBack, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Settings
    //glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    QColor colorW = parent->palette().color(QPalette::Window);
    glClearColor(float(colorW.redF()), float(colorW.greenF()), float(colorW.blueF()), 0.0f/*float(colorW.alphaF())*/);
    //glClearColor(1,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_ADD);

    // Default scene rotation
    //rotateXMatrix.rotate(-20, -1, 0, 0);
    //rotateXMatrix.rotate(-45, 0, 1, 0);
    //rotateYMatrix.rotate(-45, 1, 0, 0);

    setColormap();
    setOpacity();

    program->bind();
    // Set some default uniform values
    program->setUniformValue(uVolumeTexture, 0);
    program->setUniformValue(uVolumeTextureLC, 6);
    program->setUniformValue(uVolumeTextureCC, 7);
    program->setUniformValue(uTextureBE, 8);
    program->setUniformValue(uTextureBE_1, 9);
    program->setUniformValue(uColormapTexture, 1);
    program->setUniformValue(uSliceTexture, 2);
    program->setUniformValue(uBoxBackSampler, 3);
    program->setUniformValue(uOpacityTexture, 4);
    program->setUniformValue(uBoxFrontSampler, 5);

    program->setUniformValue(uStepLocal2, 0);
    program->setUniformValue(uHarmonics, 1);
    program->setUniformValue(uBSize2, 2);

    program->setUniformValue(uFrame, false);
    program->setUniformValue(uSlices, false);
    program->setUniformValue(uXYBorder, false);
    program->setUniformValue(uXZBorder, false);
    program->setUniformValue(uYZBorder, false);
    program->setUniformValue(uVolumeRenderingBox, false);
    program->setUniformValue(uVolumeRendering, false);
    program->setUniformValue(uVolumeCompressRendering, false);

    program->setUniformValue(uTrim, trim);
    program->setUniformValue(uSteps, slicesCount);
    program->setUniformValue(uFrameColor, 1.0f - float(colorW.redF()), 1.0f - float(colorW.greenF()), 1.0f - float(colorW.blueF()), 1.0f);
    program->setUniformValue(uWidth, float(width()));
    program->setUniformValue(uHeight, float(height()));
    program->setUniformValue(uMin, 0.0f);
    program->setUniformValue(uMax, 0.0f);
    program->setUniformValue(uMode, vRMode);

    program->release();

    initialized = true;
    renderLater();
}

/**
 * @brief Main render function
 */
void GWindow::render()
{
    if (checkGlError() == GL_OUT_OF_MEMORY) {
        unload3DTexture();
        unload3DCompressTexture();
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
    QVector3DI position = fillSpace ? QVector3DI(0, 0, 0) : datasetPosition;

    // Create vectors for scale 3D frame to the longest size = 1.0f
    float maxSize = qMax(size.x(), qMax(size.y(), size.z()));

    QVector3D maxSizeScaled = size / maxSize; // the longest size is 1.0f
    QVector3D imageSizeScaled = datasetSize / maxSize;
    QVector3D imagePositionScaled = position / maxSize;

    // Translate to the middle of 3D frame
    matrix.translate(-maxSizeScaled.x() / 2.0f, -maxSizeScaled.y() / 2.0f, -maxSizeScaled.z() / 2.0f);

    float frameOffset = 0.01f;

    // Activate shader program
    program->bind();

    // Send matrix to shader
    program->setUniformValue(uMatrix, matrix);
    program->setUniformValue(uSliceMatrix, QMatrix4x4());

    // Clear viewport and framebuffers
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboFront, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboBack, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Frame
    if (frame) {
        program->setUniformValue(uFrame, true);

        QMatrix4x4 sMatrix;
        QMatrix4x4 rMatrix;

        rMatrix.scale(1 + 2 * frameOffset);
        rMatrix.translate(-frameOffset, -frameOffset, -frameOffset);

        // Smaller frame
        sMatrix.translate(imagePositionScaled);
        sMatrix.scale(imageSizeScaled);
        program->setUniformValue(uMatrix, matrix * sMatrix * rMatrix);
        renderBox(true);

        // Big frame
        if (datasetSize != size) {
            QMatrix4x4 sMatrix;
            sMatrix.scale(maxSizeScaled);
            program->setUniformValue(uMatrix, matrix * sMatrix * rMatrix);
            renderBox(true);
        }

        program->setUniformValue(uFrame, false);
    }

    // Slices
    if (sliceXY || sliceXZ || sliceYZ) {
        program->setUniformValue(uSlices, true);

        QMatrix4x4 imageMatrix;
        imageMatrix.translate(imagePositionScaled);
        imageMatrix.scale(imageSizeScaled);

        QMatrix4x4 offsetMatrix;
        offsetMatrix.scale(1 + 2 * frameOffset);
        offsetMatrix.translate(-frameOffset, -frameOffset, 0);

        // Rotate and translate by slice index
        QMatrix4x4 translateXYMatrix;
        translateXYMatrix.translate(0, 0, sliceIndex.z());
        QMatrix4x4 translateXZMatrix;
        translateXZMatrix.translate(0, sliceIndex.y(), 0);
        translateXZMatrix.rotate(90, 1, 0, 0);
        QMatrix4x4 translateYZMatrix;
        translateYZMatrix.translate(sliceIndex.x(), 0, 0);
        translateYZMatrix.rotate(90, 1, 0, 0);
        translateYZMatrix.rotate(90, 0, 1, 0);

        glBindVertexArray(vao);
        iboSliceElements.bind();
        vboSliceVertices.bind();

        glEnableVertexAttribArray(GLuint(aPosition));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        glActiveTexture(GL_TEXTURE2);

        if (sliceXY) {
            glVertexAttribPointer(GLuint(aPosition), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBindTexture(GL_TEXTURE_2D, textureXY);

            // Draw slice
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateXYMatrix);
            program->setUniformValue(uSliceMatrix, translateXYMatrix);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);

            program->setUniformValue(uVolumeRenderingBox, true);
            //program->setUniformValue(uMatrix, matrix * imageMatrix * translateXYMatrix * offsetMatrix);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            program->setUniformValue(uVolumeRenderingBox, false);

            // Draw 2D frame
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateXYMatrix * offsetMatrix);
            program->setUniformValue(uXYBorder, true);
            glDrawElements(GL_LINE_LOOP, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            program->setUniformValue(uXYBorder, false);
        }

        if (sliceXZ) {
            glVertexAttribPointer(GLuint(aPosition), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBindTexture(GL_TEXTURE_2D, textureXZ);

            // Draw slice
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateXZMatrix);
            program->setUniformValue(uSliceMatrix, translateXZMatrix);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);

            program->setUniformValue(uVolumeRenderingBox, true);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            program->setUniformValue(uVolumeRenderingBox, false);

            // Draw 2D frame
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateXZMatrix * offsetMatrix);
            program->setUniformValue(uXZBorder, true);
            glDrawElements(GL_LINE_LOOP, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            program->setUniformValue(uXZBorder, false);
        }

        if (sliceYZ) {
            glVertexAttribPointer(GLuint(aPosition), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBindTexture(GL_TEXTURE_2D, textureYZ);

            // Draw slice
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateYZMatrix);
            program->setUniformValue(uSliceMatrix, translateYZMatrix);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);

            program->setUniformValue(uVolumeRenderingBox, true);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawElements(GL_TRIANGLES, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            program->setUniformValue(uVolumeRenderingBox, false);

            // Draw 2D frame
            program->setUniformValue(uMatrix, matrix * imageMatrix * translateYZMatrix * offsetMatrix);
            program->setUniformValue(uYZBorder, true);
            glDrawElements(GL_LINE_LOOP, sizeof(sliceElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr);
            program->setUniformValue(uYZBorder, false);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_1D, 0);

        glDisableVertexAttribArray(GLuint(aPosition));

        vboSliceVertices.release();
        iboSliceElements.release();
        glBindVertexArray(0);

        program->setUniformValue(uSliceMatrix, QMatrix4x4());
        program->setUniformValue(uSlices, false);
    }

    // Volume rendering
    if (volumeRendering) {
        //glEnable(GL_BLEND);

        QMatrix4x4 sMatrix;
        sMatrix.translate(imagePositionScaled);
        sMatrix.scale(imageSizeScaled);
        program->setUniformValue(uMatrix, matrix * sMatrix);

        program->setUniformValue(uVolumeRenderingBox, true);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        renderBox();
        glCullFace(GL_FRONT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureFboFront, 0);
        renderBox();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        program->setUniformValue(uVolumeRenderingBox, false);

        program->setUniformValue(uVolumeRendering, true);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureFboBack);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D, opacityTexture);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textureFboFront);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_3D, textureLC);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_3D, textureCC);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_BUFFER, textureBE);

        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_BUFFER, textureBE_1);

        //glCullFace(GL_BACK);

        renderBox();

        glBindTexture(GL_TEXTURE_3D, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_1D, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        program->setUniformValue(uVolumeRendering, false);

        glDisable(GL_CULL_FACE);
        //glDisable(GL_BLEND);
    }

    program->release();
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
                setTrim(!trim);
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
            break;
        }
        case QEvent::MouseButtonPress: {
            if (dynamic_cast<QMouseEvent *>(event)->buttons() == Qt::MiddleButton) {
                position.setX(0);
                position.setY(0);
                renderLater();
            }
            break;
        }
        default:
            return OpenGLWindow::event(event);
    }
    return OpenGLWindow::event(event);
}

/**
 * @brief Resize event
 */
void GWindow::resizeEvent(QResizeEvent *)
{
    if (initialized) {
        // Resize framebuffer texture
        glBindTexture(GL_TEXTURE_2D, textureFboBack);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, textureFboFront);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        program->bind();
        program->setUniformValue(uWidth, float(width()));
        program->setUniformValue(uHeight, float(height()));
        program->release();
    }
    if (isExposed()) {
        renderNow();
    }
}

/**
 * @brief Sets compress rendering
 * @param[in] value True/False
 */
void GWindow::setCompressRendering(bool value)
{
    if (initialized) {
        program->bind();
        program->setUniformValue(uVolumeCompressRendering, value);
        program->release();
    }
}

/**
 * @brief Sets min value for colormapping
 * @param[in] value Min value
 */
void GWindow::setMinValue(float value)
{
    if (initialized) {
        program->bind();
        program->setUniformValue(uMin, value);
        program->release();
    }
    renderLater();
}

/**
 * @brief Sets max value for colormapping
 * @param[in] value Max value
 */
void GWindow::setMaxValue(float value)
{
    if (initialized) {
        program->bind();
        program->setUniformValue(uMax, value);
        program->release();
    }
    renderLater();
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
    if (value.size() > 0) {
        glBindTexture(GL_TEXTURE_1D, opacityTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, value.size(), 0, GL_RED, GL_FLOAT, value.data());
        glBindTexture(GL_TEXTURE_1D, 0);
        renderLater();
    }
}

/**
 * @brief Set trim?
 * @param[in] value True/False
 */
void GWindow::setTrim(bool value)
{
    trim = value;
    if (initialized) {
        program->bind();
        program->setUniformValue(uTrim, trim);
        program->release();
    }
    renderLater();
}

/**
 * @brief Sets size for main 3D frame - whole 3D domain
 * @param[in] size Size
 */
void GWindow::setFrameSize(H5Helper::Vector3D size)
{
    setFrameSize(QVector3DI(size.x(), size.y(), size.z()));
}

/**
 * @brief Sets size for main 3D frame - whole 3D domain
 * @param[in] size Size
 */
void GWindow::setFrameSize(QVector3DI size)
{
    frameSize = size;
}

/**
 * @brief Sets size for original (may be defined by sensor mask) frame
 * @param[in] size Size
 */
void GWindow::setDatasetSize(H5Helper::Vector3D size)
{
    setDatasetSize(QVector3DI(size.x(), size.y(), size.z()));
}

/**
 * @brief Sets size for original (may be defined by sensor mask) frame
 * @param[in] size Size
 */
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

/**
 * @brief Sets position of sensor mask defined 3D dataset
 * @param[in] position
 */
void GWindow::setDatasetPosition(QVector3DI position)
{
    datasetPosition = position;
}

/**
 * @brief Sets XY slice data to texture
 * @param[in] data Slice data
 * @param[in] index Slice index
 */
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
    this->sliceIndex.setZ(indexTmp);
    renderLater();
}

/**
 * @brief Sets XZ slice data to texture
 * @param[in] data Slice data
 * @param[in] index Slice index
 */
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
    this->sliceIndex.setY(indexTmp);
    renderLater();
}

/**
 * @brief Sets YZ slice data to texture
 * @param[in] data Slice data
 * @param[in] index Slice index
 */
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
    this->sliceIndex.setX(indexTmp);
    renderLater();
}

/**
 * @brief Sets 3D data to 3D texture
 * @param[in] data 3D data
 */
void GWindow::set3DData(float *data)
{
    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, datasetSize.x(), datasetSize.y(), datasetSize.z(), 0, GL_RED, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_3D, 0);

    // Check OUT_OF_MEMORY, dataset is too big
    if (checkGlError() != GL_NO_ERROR) {
        unload3DTexture();
        return;
    }

    renderLater();
}

/**
 * @brief Sets 3D compression coefficients data to 3D texture
 * @param[in] dataLC 3D compression last coefficients data
 * @param[in] dataCC 3D compression current coefficients data
 * @param[in] localStep Local step between coefficients
 */
void GWindow::set3DCompressData(float *dataLC, float *dataCC, hsize_t localStep)
{
    if (compressHelper) {
        glBindTexture(GL_TEXTURE_3D, textureLC);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, datasetSize.x() * GLint(compressHelper->getStride()), datasetSize.y(), datasetSize.z(), 0, GL_RED, GL_FLOAT, dataLC);
        glBindTexture(GL_TEXTURE_3D, 0);

        // Check OUT_OF_MEMORY, dataset is too big
        if (checkGlError() != GL_NO_ERROR) {
            unload3DCompressTexture();
            return;
        }

        glBindTexture(GL_TEXTURE_3D, textureCC);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, datasetSize.x() * GLint(compressHelper->getStride()), datasetSize.y(), datasetSize.z(), 0, GL_RED, GL_FLOAT, dataCC);
        glBindTexture(GL_TEXTURE_3D, 0);

        // Check OUT_OF_MEMORY, dataset is too big
        if (checkGlError() != GL_NO_ERROR) {
            unload3DCompressTexture();
            return;
        }

        program->bind();
        program->setUniformValue(uStepLocal2, int(localStep * 2));
        program->release();
    }
    renderLater();
}

/**
 * @brief Renders Box
 */
void GWindow::renderBox(bool frame)
{
    glBindVertexArray(vao);
    vboCubeVertices.bind();
    iboCubeElements.bind();
    glEnableVertexAttribArray(GLuint(aPosition));
    glVertexAttribPointer(GLuint(aPosition), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    if (frame) {
        glDrawElements(GL_LINE_LOOP, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr); // 3*12
    } else {
        glDrawElements(GL_TRIANGLES, sizeof(cubeElements) / sizeof(GLint), GL_UNSIGNED_INT, nullptr); // 3*12
    }
    glDisableVertexAttribArray(GLuint(aPosition));
    iboCubeElements.release();
    vboCubeVertices.release();
    glBindVertexArray(0);
}

/**
 * @brief Unloads 3D texture data
 */
void GWindow::unload3DTexture()
{
    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

/**
 * @brief Unloads 3D compression coefficients texture data
 */
void GWindow::unload3DCompressTexture()
{
    glBindTexture(GL_TEXTURE_3D, textureLC);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);

    glBindTexture(GL_TEXTURE_3D, textureCC);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

/**
 * @brief Frees or resets 2D textures for slices
 */
void GWindow::unloadSlicesTextures()
{
    sliceIndex.setX(0.5);
    sliceIndex.setY(0.5);
    sliceIndex.setZ(0.5);
    glBindTexture(GL_TEXTURE_2D, textureXY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, textureXZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, textureYZ);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);
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
