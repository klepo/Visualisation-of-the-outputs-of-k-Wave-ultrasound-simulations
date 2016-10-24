/**
 * @file        openglwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with OpenGLWindow class declaration.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
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

class OpenGLWindow : public QWindow, public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow *parent = 0);
    ~OpenGLWindow();
    virtual void render() = 0;
    virtual void initialize() = 0;
    bool event(QEvent *event);

public slots:
    void renderLater();
    void renderNow();

signals:
    void setStatusMessage(QString, int timeout = 3000);

protected:
    void exposeEvent(QExposeEvent *event);
    void resizeEvent(QResizeEvent *event);
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
    QPointF lastPos;
    QPointF currentPos;
    QPointF diffPos;
    QPointF pos;

private:
    bool hasDebugExtension();
    static void messageLogged(const QOpenGLDebugMessage &message);
    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
    QOpenGLDebugLogger *logger;
    bool m_update_pending;
    float r;
};

#endif // OPENGLWINDOW_H
