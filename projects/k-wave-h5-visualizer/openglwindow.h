/**
 * @file        openglwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with OpenGLWindow class declaration.
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

#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QtCore>
#include <QtGui>
//#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_4_3_Core>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <Windows.h> // for Sleep
#endif

// Set OpenGL version and profile
/// OpenGL major version
//static const unsigned int OPENGL_MAJOR = 3;
static const unsigned int OPENGL_MAJOR = 4;

/// OpenGL minor version
//static const unsigned int OPENGL_MINOR = 3;
static const unsigned int OPENGL_MINOR = 3;

/// Qt OpenGL functions selection
//typedef QOpenGLFunctions_3_3_Core OPENGL_FUNCTIONS;
typedef QOpenGLFunctions_4_3_Core OPENGL_FUNCTIONS;

/// Selection of OpenGL profile (QSurfaceFormat::CoreProfile)
static const QSurfaceFormat::OpenGLContextProfile OPENGL_PROFILE = QSurfaceFormat::CoreProfile;

namespace QTest
{
    void qSleep(int ms);
}

/**
 * @brief The OpenGLWindow class represents wrapper for OpenGL window
 */
class OpenGLWindow : public QWindow, public OPENGL_FUNCTIONS
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow *parent = nullptr);
    ~OpenGLWindow();
    /// Pure virtual render function
    virtual void render() = 0;
    /// Pure virtual initialization function
    virtual void initialize() = 0;
    bool event(QEvent *event);

    double getElapsedMs() const;
    bool getLeftButtonPressed() const;
    bool getRightButtonPressed() const;
    int getWheelDelta() const;
    QPointF getLastPositionPressed() const;
    QPointF getCurrentPositionPressed() const;

public slots:
    void renderLater();
    void renderNow();

signals:
    /**
     * @brief Sets status message signal
     * @param[in] message Message
     * @param[in] timeout Timeout in ms
     */
    void setStatusMessage(QString message, int timeout = 3000);
    /// Rendered signal
    void rendered();

protected:
    virtual void exposeEvent(QExposeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *event);
    GLenum checkGlError();
    void checkInitAndMakeCurrentContext();

private:
    bool hasDebugExtension();
    bool isOpenGLVersionSupported();
    static void messageLogged(const QOpenGLDebugMessage &message);
    QOpenGLContext *context;
    QOpenGLPaintDevice *device;
    QOpenGLDebugLogger *logger;
    bool m_update_pending;
    double elapsedMs = 0;

    bool mouseDown;
    bool leftButtonPressed;
    bool rightButtonPressed;
    int wheelDelta = 0;
    QPointF lastPositionPressed;
    QPointF currentPositionPressed;
    QPointF currentPosition;
};

#endif // OPENGLWINDOW_H
