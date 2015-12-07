/*
 * @file        openglwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The header file with OpenGLWindow class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
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
    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();
    bool event(QEvent *event);

public slots:
    void renderLater();
    void renderNow();
    void clearDiff();

signals:
    void setStatusMessage(QString, int timeout = 3000);

protected:
    //bool event(QEvent *event);
    void exposeEvent(QExposeEvent *event);
    void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    GLenum checkGlError();

    bool mouseDown;
    bool leftButton;
    bool rightButton;
    int wheelDelta;
    QElapsedTimer timer;
    QTimer *moveTimer;
    QPointF lastPos;
    QPointF currentPos;
    QPointF diffPos;

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
