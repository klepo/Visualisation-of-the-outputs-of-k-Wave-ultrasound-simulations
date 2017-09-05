/**
 * @file        openglwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with OpenGLWindow class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QOpenGLFunctions_3_3_Core>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

namespace QTest
{
    void qSleep(int ms);
}

/**
 * @brief The OpenGLWindow class represents wrapper for OpenGL window
 */
class OpenGLWindow : public QWindow, public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow *parent = 0);
    ~OpenGLWindow();
    virtual void render() = 0;
    virtual void initialize() = 0;
    bool event(QEvent *event);
    double getElapsedMs() const;

public slots:
    void renderLater();
    void renderNow();

signals:
    void setStatusMessage(QString, int timeout = 3000);
    /// Rendered signal
    void rendered();

protected:
    void exposeEvent(QExposeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    GLenum checkGlError();
    void checkInitAndMakeCurrentContext();

    bool mouseDown;
    bool leftButton;
    bool rightButton;
    int wheelDelta = 0;
    QElapsedTimer timer;
    QTimer *moveTimer;
    QPointF lastPositionPressed;
    QPointF currentPositionPressed;
    QPointF diffPos;
    QPointF currentPosition;

private:
    bool hasDebugExtension();
    bool isOpenGLVersionSupported();
    static void messageLogged(const QOpenGLDebugMessage &message);
    QOpenGLContext *context;
    QOpenGLPaintDevice *device;
    QOpenGLDebugLogger *logger;
    bool m_update_pending;
    float r;
    double elapsedMs = 0;

};

#endif // OPENGLWINDOW_H
