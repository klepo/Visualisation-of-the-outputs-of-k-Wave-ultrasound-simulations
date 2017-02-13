/**
 * @file        openglwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing the OpenGLWindow class.
 *              This class is mainly for OpenGL context creation, render, and mouse move tracking with buttons.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
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
    nanosleep(&ts, 0);
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
    , context(0)
    , device(0)
    , logger(0)
    , m_update_pending(false)
{
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setVersion(3, 3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setOption(QSurfaceFormat::DebugContext);
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setDepthBufferSize(24);
    surfaceFormat.setRedBufferSize(8);
    surfaceFormat.setGreenBufferSize(8);
    surfaceFormat.setBlueBufferSize(8);
    surfaceFormat.setAlphaBufferSize(8);
    // Smoother lines
    surfaceFormat.setSamples(4);
    setFormat(surfaceFormat);
}

/**
 * @brief OpenGLWindow::~OpenGLWindow
 */
OpenGLWindow::~OpenGLWindow()
{
    //moveTimer->stop();
    //delete moveTimer;
    //delete m_context; // Some BUG - deletion causes wrong freeing memory
    delete device;
    //thread->deleteLater();
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
            break;
    }
    return QWindow::event(event);
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

    checkInitAndMakeCurrentContext();

    glFinish();

    // timer
    QElapsedTimer timer;
    timer.start();

    render();

    glFinish();

    context->swapBuffers(this);

    //QTest::qSleep(17); // max cca 60 fps

    qint64 elapsed = timer.nsecsElapsed();
    elapsedMs = double(elapsed / 1000000.0);
    emit rendered();

    //QString framesPerSecond;
    //framesPerSecond.setNum(1000.0 / elapsedMs, 'f', 2);

    //qDebug() << "render time:" << elapsedMs << "ms";
    //qDebug() << framesPerSecond.toDouble() << "fps";

    // Change last position
    lastPositionPressed = currentPositionPressed;
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
    currentPositionPressed = event->pos();
    lastPositionPressed = currentPositionPressed;
    renderLater();
}

/**
 * @brief OpenGLWindow::mouseMoveEvent
 * @param event
 */
void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
    currentPosition = event->pos();
    if (mouseDown) {
        // Change current position
        currentPositionPressed = currentPosition;
        // Render
        renderLater();
    }
}

/**
 * @brief OpenGLWindow::mouseReleaseEvent Mouse release event
 */
void OpenGLWindow::mouseReleaseEvent(QMouseEvent *)
{
    rightButton = false;
    leftButton = false;
    mouseDown = false;
    renderLater();
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
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_ENUM") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_VALUE) {
            qWarning() << "OpenGL error: GL_INVALID_VALUE " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_VALUE") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_OPERATION) {
            qWarning() << "OpenGL error: GL_INVALID_OPERATION " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_OPERATION") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_STACK_OVERFLOW) {
            qWarning() << "OpenGL error: GL_STACK_OVERFLOW " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_STACK_OVERFLOW") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_STACK_UNDERFLOW) {
            qWarning() << "OpenGL error: GL_STACK_UNDERFLOW " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_STACK_UNDERFLOW") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_OUT_OF_MEMORY) {
            qWarning() << "OpenGL error: GL_OUT_OF_MEMORY " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_OUT_OF_MEMORY") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_TABLE_TOO_LARGE) {
            qWarning() << "OpenGL error: GL_TABLE_TOO_LARGE " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_TABLE_TOO_LARGE") + " " + QString::number(err)).toStdString().c_str());
        } else if (err == GL_INVALID_FRAMEBUFFER_OPERATION) {
            qWarning() << "OpenGL error: GL_INVALID_FRAMEBUFFER_OPERATION " << err;
            messageBox.critical(0, "OpenGL error", (QString("GL_INVALID_FRAMEBUFFER_OPERATION") + " " + QString::number(err)).toStdString().c_str());
        } else {
            qWarning() << "OpenGL error: " << err;
            messageBox.critical(0, "OpenGL error", QString::number(err).toStdString().c_str());
        }
    }
    return ret;
}

/**
 * @brief OpenGLWindow::checkInitAndMakeCurrentContext
 */
void OpenGLWindow::checkInitAndMakeCurrentContext()
{
    bool needsInitialize = false;

    if (!context) {
        // OpenGL context creation
        context = new QOpenGLContext();
        context->setFormat(requestedFormat());
        context->create();
        context->makeCurrent(this);

        // Activate debug extension
        if (hasDebugExtension()) {
            logger = new QOpenGLDebugLogger();
            logger->initialize();
            QObject::connect(logger, &QOpenGLDebugLogger::messageLogged, OpenGLWindow::messageLogged);
            logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        }

        needsInitialize = true;
    }

    if (needsInitialize) {
        if (!isOpenGLVersionSupported()) {
            qCritical("Could not obtain required OpenGL context version");
            Q_ASSERT_X(false, "OpenGL error", "Could not obtain required OpenGL context version");
            exit(1);
        } else {
            // Initialize
            initializeOpenGLFunctions();
            initialize();
        }
    }

    context->makeCurrent(this);
}

/**
 * @brief OpenGLWindow::hasDebugExtension
 * @return true/false
 */
bool OpenGLWindow::hasDebugExtension()
{
    return context->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
}

/**
 * @brief OpenGLWindow::isOpenGLVersionSupported
 * @return true/false
 */
bool OpenGLWindow::isOpenGLVersionSupported()
{
    QOpenGLFunctions_3_3_Core* funcs = 0;
    funcs = context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    return funcs;
}

/**
 * @brief OpenGLWindow::messageLogged
 * @param message
 */
void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &message)
{
    if (message.type() == QOpenGLDebugMessage::ErrorType)
        qCritical() << message;
    else
        qDebug() << message;
}

double OpenGLWindow::getElapsedMs() const
{
    return elapsedMs;
}
