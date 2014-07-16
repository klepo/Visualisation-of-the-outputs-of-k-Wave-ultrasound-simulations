/*
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
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

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QTime>

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
{
    //setOpacity(0.5);
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    // Smoother lines
    surfaceFormat.setSamples(4);
    //surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);

    // Timer for getting FPS
    //timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(renderNow()));
    // Timer for animation of rotate
    /*moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(clearDiff()));*/
}

/**
 * @brief OpenGLWindow::~OpenGLWindow
 */
OpenGLWindow::~OpenGLWindow()
{
    //timer->stop();
    //delete timer;
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
    if (event->type() == QEvent::UpdateRequest)
        renderNow();

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
    //QTime timer;
    //timer.start();
    if (!isExposed())
        return;

    m_update_pending = false;

    bool needsInitialize = false;

    if (!m_context) {
        // OpenGL context creation
        m_context = new QOpenGLContext();
        m_context->setFormat(requestedFormat());
        m_context->create();
        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);
    //qDebug() << "render time: " << timer.elapsed();
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
    renderNow();
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
        renderNow();
        // Change last position
        lastPos = event->pos();
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
    renderNow();
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
    renderNow();
    wheelDelta = 0;
}
