/**
 * @file        openglwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing OpenGLWindow class definition.
 *
 * This class is mainly for OpenGL context creation, render, and mouse move
 * tracking with buttons.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "openglwindow.h"

/**
 * @brief Helper sleep function
 * @param[in] ms Milliseconds
 */
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
 * @brief Creates OpenGLWindow object
 * @param[in] parent Parent (optional)
 */
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , context(nullptr)
    , logger(nullptr)
    , updatePending(false)
    , mouseDown(false)
    , leftButtonPressed(false)
    , rightButtonPressed(false)
{
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setVersion(OPENGL_MAJOR, OPENGL_MINOR);
    surfaceFormat.setProfile(OPENGL_PROFILE);
    surfaceFormat.setOption(QSurfaceFormat::DebugContext);
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setDepthBufferSize(24);
    surfaceFormat.setRedBufferSize(8);
    surfaceFormat.setGreenBufferSize(8);
    surfaceFormat.setBlueBufferSize(8);
    surfaceFormat.setAlphaBufferSize(8);
    // Smoother lines
    surfaceFormat.setSamples(4);

    // surfaceFormat.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    // surfaceFormat.setSwapInterval(0);

    setFormat(surfaceFormat);

    // qDebug() << this->requestedFormat().swapBehavior() << this->requestedFormat().swapInterval();
}

/**
 * @brief Destructor of OpenGLWindow object
 */
OpenGLWindow::~OpenGLWindow()
{
    // moveTimer->stop();
    // delete moveTimer;
    // delete context; // Some BUG - deletion causes wrong freeing memory
    // thread->deleteLater();
}

/**
 * @brief Event
 * @param[in] event Event
 * @return QWindow::event(event)
 */
bool OpenGLWindow::event(QEvent *event)
{
    // qDebug() << event->type();
    switch (event->type()) {
        case QEvent::UpdateRequest:
            updatePending = false;
            renderNow();
            return true;
        default:
            break;
    }
    return QWindow::event(event);
}

/**
 * @brief Is left button pressed?
 * @return True/False
 */
bool OpenGLWindow::getLeftButtonPressed() const
{
    return leftButtonPressed;
}

/**
 * @brief Is right button pressed?
 * @return True/False
 */
bool OpenGLWindow::getRightButtonPressed() const
{
    return rightButtonPressed;
}

/**
 * @brief Returns wheel delta
 * @return Wheel delta
 */
int OpenGLWindow::getWheelDelta() const
{
    return wheelDelta;
}

/**
 * @brief Returns last position pressed
 * @return Last position pressed
 */
QPointF OpenGLWindow::getLastPositionPressed() const
{
    return lastPositionPressed;
}

/**
 * @brief Returns current position pressed
 * @return Current position pressed
 */
QPointF OpenGLWindow::getCurrentPositionPressed() const
{
    return currentPositionPressed;
}

/**
 * @brief Creates update request (event) for render
 */
void OpenGLWindow::renderLater()
{
    if (!updatePending) {
        updatePending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

/**
 * @brief Renders immediately
 */
void OpenGLWindow::renderNow()
{
    // timer.start();

    if (!isExposed())
        return;

    checkInitAndMakeCurrentContext();

    // glFinish();

    // timer
    QElapsedTimer timer;
    timer.start();

    render();

    glFinish();
    // glFlush();
    context->swapBuffers(this);
    // context->makeCurrent(this);
    // context->makeCurrent(this);
    // context->swapBuffers(this);

    // QTest::qSleep(17); // max ca 60 fps

    elapsedNs = timer.nsecsElapsed();
    // elapsedMs = double(elapsed / 1000000.0);
    emit rendered(elapsedNs);

    // QString framesPerSecond;
    // framesPerSecond.setNum(1000.0 / elapsedNs, 'f', 2);

    // qDebug() << "render time:" << elapsedNs << "ms";
    // qDebug() << framesPerSecond.toDouble() << "fps";

    // Change last position
    lastPositionPressed = currentPositionPressed;
    wheelDelta          = 0;
}

/**
 * @brief Expose event
 * @param[in] event Event
 */
void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

/**
 * @brief Mouse press event
 * @param[in] event Event
 */
void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::RightButton)
        rightButtonPressed = true;
    if (event->buttons() == Qt::LeftButton)
        leftButtonPressed = true;
    mouseDown = true;
    // Save mouse position
    currentPositionPressed = event->pos();
    lastPositionPressed    = currentPositionPressed;
    renderLater();
}

/**
 * @brief Mouse move event
 * @param[in] event Event
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
 * @brief Mouse release event
 */
void OpenGLWindow::mouseReleaseEvent(QMouseEvent *)
{
    rightButtonPressed = false;
    leftButtonPressed  = false;
    mouseDown          = false;
    renderLater();
}

/**
 * @brief Mouse wheel event
 * @param[in] event Event
 */
void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
    // Save delta
    wheelDelta = event->angleDelta().y();
    renderLater();
}

/**
 * @brief Checks OpenGL errors
 * @return Error code
 */
GLenum OpenGLWindow::checkGlError()
{
    GLenum err;
    GLenum ret = GL_NO_ERROR;
    while ((err = glGetError()) != GL_NO_ERROR) {
        ret = err;
        QMessageBox messageBox;
        qWarning() << "OpenGL error:" << getGLErrorString(err) << err;
        messageBox.critical(nullptr, "OpenGL error",
                            (getGLErrorString(err) + " " + QString::number(err)).toStdString().c_str());
    }
    return ret;
}

/**
 * @brief Checks OpenGL framebuffer status
 * @return Error code
 */
GLenum OpenGLWindow::checkFramebufferStatus()
{
    GLenum err = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        QMessageBox messageBox;
        qWarning() << "OpenGL framebuffer error:" << getGLFramebufferStatusString(err) << err;
        messageBox.critical(nullptr, "OpenGL error",
                            (getGLFramebufferStatusString(err) + " " + QString::number(err)).toStdString().c_str());
    }
    return err;
}

/**
 * @brief Converts OpenGL error codes to string
 * @param[in] errorCode OpenGL error code
 * @return Error string
 */
QString OpenGLWindow::getGLErrorString(GLenum errorCode) const
{
    QString ret = "UNKNOWN ERROR";
    switch (errorCode) {
        case GL_INVALID_ENUM:
            ret = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            ret = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            ret = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            ret = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            ret = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            ret = "GL_OUT_OF_MEMORY";
            break;
        case GL_TABLE_TOO_LARGE:
            ret = "GL_TABLE_TOO_LARGE";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            ret = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            break;
    }
    return ret;
}

/**
 * @brief Converts OpenGL framebuffer status error codes to string
 * @param[in] errorCode OpenGL framebuffer status error code
 * @return Framebuffer error string
 */
QString OpenGLWindow::getGLFramebufferStatusString(GLenum errorCode) const
{
    QString ret = "UNKNOWN ERROR";
    switch (errorCode) {
        case GL_FRAMEBUFFER_COMPLETE:
            ret = "GL_FRAMEBUFFER_COMPLETE";
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            ret = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            ret = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            ret = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            break;
    }
    return ret;
}

/**
 * @brief Checks initialization and make current context
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

        if (!isOpenGLVersionSupported()) {
            qCritical("Could not obtain required OpenGL context version");
            Q_ASSERT_X(false, "OpenGL error", "Could not obtain required OpenGL context version");
            exit(1);
        }

        // Activate debug extension
        if (hasDebugExtension()) {
            logger = new QOpenGLDebugLogger(context);
            logger->initialize();
            QObject::connect(logger, &QOpenGLDebugLogger::messageLogged, OpenGLWindow::messageLogged);
            logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        }

        needsInitialize = true;
    }

    if (needsInitialize) {
        // Initialize
        initializeOpenGLFunctions();
        initialize();
    }

    context->makeCurrent(this);
}

/**
 * @brief Has debug extension?
 * @return True/False
 */
bool OpenGLWindow::hasDebugExtension() const
{
    return context->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
}

/**
 * @brief Is OpenGL version supported?
 * @return True/False
 */
bool OpenGLWindow::isOpenGLVersionSupported() const
{
    OPENGL_FUNCTIONS *funcs = nullptr;
    funcs                   = context->versionFunctions<OPENGL_FUNCTIONS>();
    return funcs;
}

/**
 * @brief Message logged
 * @param[in] message Message
 */
void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &message)
{
    if (message.type() == QOpenGLDebugMessage::ErrorType)
        qCritical() << message;
    /*else
        qDebug() << message;*/
}
