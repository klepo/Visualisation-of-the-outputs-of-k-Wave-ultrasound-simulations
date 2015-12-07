/*
 * @file        openglwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The implementation file containing the OpenGLWindow class.
 *              This class is mainly for OpenGL context creation, render, and mouse move tracking with buttons.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "openglwindow.h"

void QTest::qSleep(int ms)
{
    Q_ASSERT(ms > 0);

#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

/**
 * @brief OpenGLWindow::OpenGLWindow
 * @param parent
 */
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , mouseDown(false)
    , leftButton(false)
    , rightButton(false)
    , wheelDelta(0)
    , m_context(0)
    , m_device(0)
    , m_update_pending(false)
    , logger(0)
{
    //setOpacity(0.5);
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setVersion(3, 3);
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setOption(QSurfaceFormat::DebugContext);
    surfaceFormat.setDepthBufferSize(24);
    surfaceFormat.setRedBufferSize(8);
    surfaceFormat.setGreenBufferSize(8);
    surfaceFormat.setBlueBufferSize(8);
    surfaceFormat.setAlphaBufferSize(8);

    // Smoother lines
    surfaceFormat.setSamples(4);
    setFormat(surfaceFormat);

    // Timer for animation of rotate
    /*moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(clearDiff()));*/
}

/**
 * @brief OpenGLWindow::~OpenGLWindow
 */
OpenGLWindow::~OpenGLWindow()
{
    //moveTimer->stop();
    //delete moveTimer;
    //delete m_context; // Some BUG - deletion causes wrong freeing memory
    delete m_device;
    //thread->deleteLater();
}

/**
 * @brief OpenGLWindow::render
 * @param painter
 */
void OpenGLWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

/**
 * @brief OpenGLWindow::initialize
 */
void OpenGLWindow::initialize()
{

}

/**
 * @brief OpenGLWindow::render
 */
void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}

/**
 * @brief OpenGLWindow::event
 * @param event
 * @return QWindow::event(event)
 */
bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

/**
 * @brief OpenGLWindow::exposeEvent
 * @param event
 */
void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

/**
 * @brief OpenGLWindow::resizeEvent Redraw scene on window resize
 * @param event
 */
void OpenGLWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

/**
 * @brief OpenGLWindow::renderLater Create update request (event) for render
 */
void OpenGLWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

/**
 * @brief OpenGLWindow::renderNow Render immediately
 */
void OpenGLWindow::renderNow()
{
    //timer.start();

    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        // OpenGL context creation
        m_context = new QOpenGLContext();
        m_context->setFormat(requestedFormat());
        m_context->create();
        m_context->makeCurrent(this);

        if (hasDebugExtension()) {
            logger = new QOpenGLDebugLogger();
            logger->initialize();
            QObject::connect(logger, &QOpenGLDebugLogger::messageLogged, OpenGLWindow::messageLogged);
            logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        }

        needsInitialize = true;
    }

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    m_context->makeCurrent(this);

    render();

    m_context->swapBuffers(this);

    QTest::qSleep(17); // max cca 60 fps

    //QString framesPerSecond;
    //framesPerSecond.setNum( 1000.0 / (timer.nsecsElapsed() / 1000000.0), 'f', 2);

    //qDebug() << "render time:" << (timer.nsecsElapsed() / 1000000.0) << "ms";
    //qDebug() << framesPerSecond.toDouble() << "fps";

    // Change last position
    lastPos = currentPos;
    wheelDelta = 0;
}

/**
 * @brief OpenGLWindow::mousePressEvent
 * @param event
 */
void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::RightButton)
        rightButton = true;
    if (event->buttons() == Qt::LeftButton)
        leftButton = true;

    mouseDown = true;
    // Save mouse position
    lastPos = event->pos();
    currentPos = event->pos();
    //diffPos = lastPos - currentPos;
    renderLater();
    //moveTimer->start(200);
}

/**
 * @brief OpenGLWindow::mouseMoveEvent
 * @param event
 */
void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseDown) {
        // Change current position
        currentPos = event->pos();
        //diffPos = lastPos - currentPos;
        // Render
        renderLater();
    }
}

/**
 * @brief OpenGLWindow::clearDiff (Unused)
 */
void OpenGLWindow::clearDiff()
{
    //diffPos = QPointF(0,0);
}

/**
 * @brief OpenGLWindow::mouseReleaseEvent Mouse release event
 */
void OpenGLWindow::mouseReleaseEvent(QMouseEvent *)
{
    //if (currentPos + diffPos != lastPos) {
        //setAnimating(true);
    //}

    rightButton = false;
    leftButton = false;
    mouseDown = false;
    renderLater();
    //moveTimer->stop();
}

/**
 * @brief OpenGLWindow::wheelEvent Mouse wheel event
 * @param event
 */
void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
    // Save delta
    wheelDelta = event->delta();
    renderLater();
}

/**
 * @brief OpenGLWindow::checkGlError
 * @return error code
 */
GLenum OpenGLWindow::checkGlError()
{
    GLenum err;
    GLenum ret = GL_NO_ERROR;
    while ((err = glGetError()) != GL_NO_ERROR) {
        ret = err;
        QMessageBox messageBox;
        if (err == GL_INVALID_ENUM) {
            qWarning() << "OpenGL error: GL_INVALID_ENUM " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_ENUM") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_VALUE) {
            qWarning() << "OpenGL error: GL_INVALID_VALUE " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_VALUE") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_OPERATION) {
            qWarning() << "OpenGL error: GL_INVALID_OPERATION " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_OPERATION") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_STACK_OVERFLOW) {
            qWarning() << "OpenGL error: GL_STACK_OVERFLOW " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_STACK_OVERFLOW") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_STACK_UNDERFLOW) {
            qWarning() << "OpenGL error: GL_STACK_UNDERFLOW " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_STACK_UNDERFLOW") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_OUT_OF_MEMORY) {
            qWarning() << "OpenGL error: GL_OUT_OF_MEMORY " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_OUT_OF_MEMORY") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_TABLE_TOO_LARGE) {
            qWarning() << "OpenGL error: GL_TABLE_TOO_LARGE " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_TABLE_TOO_LARGE") + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_FRAMEBUFFER_OPERATION) {
            qWarning() << "OpenGL error: GL_INVALID_FRAMEBUFFER_OPERATION " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_FRAMEBUFFER_OPERATION") + QString::number(err)).toStdString().c_str());
        } else {
            qWarning() << "OpenGL error: " << err;
            messageBox.critical(0, "OpenGL error", QString::number(err).toStdString().c_str());
        }
    }
    return ret;
}

/**
 * @brief OpenGLWindow::hasDebugExtension
 * @return
 */
bool OpenGLWindow::hasDebugExtension()
{
    return m_context->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
}

/**
 * @brief OpenGLWindow::messageLogged
 * @param message
 */
void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &message)
{
    if (message.type() == QOpenGLDebugMessage::ErrorType)
        qCritical() << message;
    else if (message.type() == QOpenGLDebugMessage::PerformanceType)
        qDebug() << message;
}
