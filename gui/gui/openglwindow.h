/*
 * @file        openglwindow.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
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

#include <QtGui/QWindow>
//#include <QtGui/QOpenGLFunctions>
#include <QTimer>
#include <QThread>

#include <QOpenGLFunctions_3_1>


QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE

class OpenGLWindow : public QWindow, public QOpenGLFunctions_3_1
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

    bool mouseDown;
    bool leftButton;
    bool rightButton;
    int wheelDelta;
    QTimer *timer;
    QTimer *moveTimer;
    QPointF lastPos;
    QPointF currentPos;
    QPointF diffPos;

private:
    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
    bool m_update_pending;
    float r;
};

#endif // OPENGLWINDOW_H
