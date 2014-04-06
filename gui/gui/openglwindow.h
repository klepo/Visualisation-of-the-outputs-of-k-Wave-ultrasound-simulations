#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QTimer>
#include <QThread>

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE

class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow *parent = 0);
    ~OpenGLWindow();
    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();
    void setAnimating(bool animating);
    bool event(QEvent *event);

public slots:
    void renderLater();
    void renderNow();
    void clearDiff();

signals:
    void setStatusMessage(QString, int timeout = 0);

protected:
    //bool event(QEvent *event);
    void exposeEvent(QExposeEvent *event);
    void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    QPointF lastPos;
    QPointF currentPos;
    QPointF diffPos;
    bool mouseDown;
    int wheelDelta;
    QTimer *timer;
    QTimer *moveTimer;

private:
    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
    bool m_update_pending;
    float r;
    //QThread *thread;

};

#endif // OPENGLWINDOW_H
