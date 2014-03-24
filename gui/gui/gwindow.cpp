#include "gwindow.h"
#include "hdf5readingthread.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <QtCore/qmath.h>
#include <QEvent>
#include <QKeyEvent>
#include <QMutex>
#include <QLabel>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>

#include <opencv2/opencv.hpp>


GLfloat plane_vertices[] = {
    -0.5, -0.5, 0.0,
    +1.5, -0.5, 0.0,
    +1.5, +1.5, 0.0,
    -0.5, +1.5, 0.0,
};

GLint plane_elements[] = {
    0, 1, 2,
    2, 3, 0,
};

GWindow::GWindow()
    : m_program(0)
    , m_frame(0)
    , texture3DInitialized(false)
{

    count = 80;
    frame = true;
    alpha = 0.01f;
    red = 0.2f;
    green = 0.2f;
    blue = 0.2f;
    zoom = -25.0f;

    imageWidth = 100;
    imageHeight = 100;
    imageDepth = 100;

    imageHeightDepthRatio = (float) imageHeight / (float) imageDepth;
    imageWidthDepthRatio = (float) imageWidth / (float) imageDepth;

    thread = NULL;

    colormap = cv::COLORMAP_JET;
}

GWindow::~GWindow()
{
    m_program->release();
    delete m_program;
}

void GWindow::initialize()
{
    emit setStatusMessage(QString("Slices: %1").arg(count));

    m_program = new QOpenGLShaderProgram(this);
    //m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexShader.vert");
    //m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentShader.frag");
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../gui/vertexShader.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../gui/fragmentShader.frag");
    qDebug() << m_program->link();

    m_aPosition = m_program->attributeLocation("aPosition");

    m_uFrame = m_program->uniformLocation("uFrame");

    m_uMatrix = m_program->uniformLocation("uMatrix");
    m_uScaleMatrix = m_program->uniformLocation("uScaleMatrix");
    m_uScalelMatrix = m_program->uniformLocation("uScalelMatrix");

    m_uHeightDepthRatio = m_program->uniformLocation("uHeightDepthRatio");
    m_uWidthDepthRatio = m_program->uniformLocation("uWidthDepthRatio");

    m_uAlpha = m_program->uniformLocation("uAlpha");
    m_uRed = m_program->uniformLocation("uRed");
    m_uGreen = m_program->uniformLocation("uGreen");
    m_uBlue = m_program->uniformLocation("uBlue");

    m_uSampler = m_program->uniformLocation("uSampler");

    m_uColormapSampler = m_program->uniformLocation("uColormapSampler");

    m_uMin = m_program->uniformLocation("uMin");
    m_uMax = m_program->uniformLocation("uMax");

    glGenBuffers(1, &ibo_plane_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_plane_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_elements), plane_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    rotateXMatrix.rotate(-30, -1, 0, 0);
    rotateYMatrix.rotate(-15, 0, -1, 0);

    glEnable(GL_TEXTURE_1D);
    glGenTextures(1, &colormapTexture);

    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_1D);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    cv::Mat colormapImage = cv::Mat::zeros(1, 256, CV_8UC1);
    for (unsigned int i = 0; i < 256; i++)
        colormapImage.data[i] = i;
    cv::applyColorMap(colormapImage, colormapImage, colormap);
    cvtColor(colormapImage, colormapImage, CV_BGR2RGB);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, colormapImage.data);

    qDebug() << m_program->bind();

    m_program->setUniformValue(m_uMin, 0.0f);
    m_program->setUniformValue(m_uMax, 0.0f);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    m_program->setUniformValue(m_uColormapSampler, 1);

    m_program->release();

    renderLater();
}

void GWindow::load3DTexture(HDF5File::HDF5Dataset *dataset, int _colormap)
{
    colormap = _colormap;

    //textureMutex.lock();
    texture3DInitialized = false;

    PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
    glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");

    imageWidth = dataset->getDims()[2];
    imageHeight = dataset->getDims()[1];
    imageDepth = dataset->getDims()[0];
    minG = dataset->getGlobalMinValueF();
    maxG = dataset->getGlobalMaxValueF();

    m_program->bind();
    m_program->setUniformValue(m_uMin, minG);
    m_program->setUniformValue(m_uMax, maxG);
    m_program->release();

    //delete thread;
    //thread =  NULL;
    thread = new HDF5ReadingThread();
    connect(thread, SIGNAL(sliceLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)), this, SLOT(setLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float)));

    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, imageWidth, imageHeight, imageDepth, 0, GL_RED, GL_FLOAT, NULL);

    for (unsigned int i = 0; i < imageDepth; i++) {
        thread->setParams(dataset, i, 0, 0, 1, imageHeight, imageWidth, imageDepth);
    }

    imageHeightDepthRatio = (float) imageHeight / (float) imageDepth;
    imageWidthDepthRatio = (float) imageWidth / (float) imageDepth;

    renderLater();

    thread->start();

}

void GWindow::setLoaded(hsize_t i, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *data, float, float)
{
    //textureMutex.lock();
    PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) wglGetProcAddress("glTexSubImage3D");

    cv::Mat image = cv::Mat(imageHeight, imageWidth, CV_32FC1, data);

    glBindTexture(GL_TEXTURE_3D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, imageWidth, imageHeight, 1, GL_RED, GL_FLOAT, image.data);

    image.release();
    delete [] data;
    data = NULL;

    //textureMutex.unlock();

    if (i == imageDepth - 1) {
        //thread->deleteLater();
        texture3DInitialized = true;
        changeColormap();
        renderLater();
    }
}

void GWindow::changeColormap(int colormap)
{
    if (texture3DInitialized) {

        glBindTexture(GL_TEXTURE_1D, colormapTexture);

        cv::Mat colormapImage = cv::Mat::zeros(1, 256, CV_8UC1);
        for (unsigned int i = 0; i < 256; i++)
            colormapImage.data[i] = i;
        cv::applyColorMap(colormapImage, colormapImage, colormap);
        cvtColor(colormapImage, colormapImage, CV_BGR2RGB);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, colormapImage.data);

        qDebug() << m_program->bind();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colormapTexture);
        m_program->setUniformValue(m_uColormapSampler, 1);

        m_program->release();

        renderLater();
    }
}

void GWindow::changeMinValue(float value)
{
    if (texture3DInitialized) {
        minG = value;
        qDebug() << value;
        m_program->bind();
        m_program->setUniformValue(m_uMin, minG);
        m_program->release();
        renderLater();
    }
}

void GWindow::changeMaxValue(float value)
{
    if (texture3DInitialized) {
        maxG = value;
        qDebug() << value;
        m_program->bind();
        m_program->setUniformValue(m_uMax, maxG);
        m_program->release();
        renderLater();
    }
}

void GWindow::render()
{
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    rotateXMatrix.rotate((float) (lastPos.y() - currentPos.y()) / 2.0f, -1, 0, 0);
    rotateYMatrix.rotate((float) (lastPos.x() - currentPos.x()) / 2.0f, 0, -1, 0);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glViewport(0, 0, width(), height());

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(45, (float) width() / (float) height(), 0.1f, 100.0f);

    QMatrix4x4 zoomMatrix;
    zoom += (float) wheelDelta / 100.0f;
    if (zoom > 0.0f) zoom = 0.0f;
    zoomMatrix.translate(0, 0, -qExp(qFabs(zoom)/20));
    matrix = matrix * zoomMatrix * rotateXMatrix * rotateYMatrix;
    matrix.translate(-0.5, -0.5, -0.5);

    m_program->setUniformValue(m_uMatrix, matrix);

    glEnableVertexAttribArray(m_aPosition);
    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, plane_vertices);

    //textureMutex.lock();
    //if (texture3DInitialized) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture);
    m_program->setUniformValue(m_uSampler, 0);
    //}
    //textureMutex.unlock();

    m_program->setUniformValue(m_uHeightDepthRatio, imageHeightDepthRatio);
    m_program->setUniformValue(m_uWidthDepthRatio, imageWidthDepthRatio);

    m_program->setUniformValue(m_uFrame, 0.0f);
    if (frame) {
        m_program->setUniformValue(m_uFrame, 1.0f);
        QMatrix4x4 sMmatrix;
        sMmatrix.translate((1.0f - (float) imageWidthDepthRatio) / 2.0f, (1.0f - (float) imageHeightDepthRatio) / 2.0f, 0.0f);
        sMmatrix.scale((float) imageWidthDepthRatio, (float) imageHeightDepthRatio, 1.0f);
        m_program->setUniformValue(m_uScaleMatrix, sMmatrix);

        glBegin(GL_LINE_LOOP);
            glVertex3f(+1.0, +0.0, +1.0);
            glVertex3f(+1.0, +1.0, +1.0);
            glVertex3f(+0.0, +1.0, +1.0);
            glVertex3f(+0.0, +0.0, +1.0);
        glEnd();

        glBegin(GL_LINES);
            glVertex3f(+1.0, +0.0, +1.0);
            glVertex3f(+1.0, +0.0, +0.0);
            glVertex3f(+1.0, +1.0, +1.0);
            glVertex3f(+1.0, +1.0, +0.0);
            glVertex3f(+0.0, +1.0, +1.0);
            glVertex3f(+0.0, +1.0, +0.0);
            glVertex3f(+0.0, +0.0, +1.0);
            glVertex3f(+0.0, +0.0, +0.0);
        glEnd();

        glBegin(GL_LINE_LOOP);
            glVertex3f(+1.0, +0.0, +0.0);
            glVertex3f(+1.0, +1.0, +0.0);
            glVertex3f(+0.0, +1.0, +0.0);
            glVertex3f(+0.0, +0.0, +0.0);
        glEnd();
    }
    m_program->setUniformValue(m_uFrame, 0.0f);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_plane_elements);

    mutex.lock();
    m_program->setUniformValue(m_uAlpha, alpha / ((float) count / 100));
    m_program->setUniformValue(m_uRed, red / ((float) count / 100));
    m_program->setUniformValue(m_uGreen, green / ((float) count / 100));
    m_program->setUniformValue(m_uBlue, blue / ((float) count / 100));
    float zHalfSize = std::max(imageWidthDepthRatio, imageHeightDepthRatio);
    zHalfSize += zHalfSize / 2;
    float step = 2.0f * zHalfSize / (count - 1);
    for (float i = -zHalfSize; i <= zHalfSize; i += step) {
        QMatrix4x4 sMmatrix;
        sMmatrix.translate(-(imageWidthDepthRatio - 1.0f) / 2.0f, -(imageHeightDepthRatio - 1.0f) / 2.0f, 0);
        sMmatrix.scale((float) imageWidthDepthRatio, (float) imageHeightDepthRatio, 1);
        sMmatrix.translate(0, 0, i);
        QMatrix4x4 slMmatrix;
        slMmatrix.translate(-(imageWidthDepthRatio - 1.0f) / 2.0f, -(imageHeightDepthRatio - 1.0f) / 2.0f, 0);
        slMmatrix.scale((float) imageWidthDepthRatio, (float) imageHeightDepthRatio, 1);
        slMmatrix.translate(0, 0, i - step);
        QMatrix4x4 mMmatrix;
        mMmatrix.translate(-0.5, -0.5, -0.5);
        QMatrix4x4 m2Mmatrix;
        m2Mmatrix.translate(0.5, 0.5, 0.5);

        m_program->setUniformValue(m_uScaleMatrix, m2Mmatrix * rotateYMatrix.inverted() * rotateXMatrix.inverted() * mMmatrix * sMmatrix);
        m_program->setUniformValue(m_uScalelMatrix, m2Mmatrix * rotateYMatrix.inverted() * rotateXMatrix.inverted() * mMmatrix * slMmatrix);
        glDrawElements(GL_TRIANGLES,  sizeof(plane_elements) / sizeof(GLint), GL_UNSIGNED_INT, 0);
    }
    mutex.unlock();


    glDisableVertexAttribArray(m_aPosition);

    m_program->release();
}

void GWindow::setAlpha(int value)
{
    alpha = (double) value / 100;
    renderLater();
}

void GWindow::setRed(int value)
{
    red = (double) value / 100;
    renderLater();
}

void GWindow::setGreen(int value)
{
    green = (double) value / 100;
    renderLater();
}

void GWindow::setBlue(int value)
{
    blue = (double) value / 100;
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
             imageHeightDepthRatio += 0.1f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Down) {
             mutex.lock();
             imageHeightDepthRatio -= 0.1f;
             if (imageHeightDepthRatio <= 0.0f) imageHeightDepthRatio = 0.1f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Right) {
             mutex.lock();
             imageWidthDepthRatio += 0.1f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_Left) {
             mutex.lock();
             imageWidthDepthRatio -= 0.1f;
             if (imageWidthDepthRatio <= 0.0f) imageWidthDepthRatio = 0.1f;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_S) {
             mutex.lock();
             imageHeightDepthRatio = (float) imageHeight / imageDepth;
             imageWidthDepthRatio = (float) imageWidth / imageDepth;
             mutex.unlock();
         }
         if (ke->key() == Qt::Key_F) {
             mutex.lock();
             if (frame)
                 frame = false;
             else
                 frame = true;
             mutex.unlock();
         }
         renderLater();
    }

    if (event->type() == QEvent::UpdateRequest)
        renderNow();

    return QWindow::event(event);
}
