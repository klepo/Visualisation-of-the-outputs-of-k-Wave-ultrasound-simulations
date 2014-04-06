#include "openglwindow.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <qDebug>
#include <QThread>

OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_context(0)
    , m_device(0)
    , m_update_pending(false)
    , mouseDown(false)
    , wheelDelta(0)
    , r(1)
{
    //setOpacity(0.5);
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSamples(4);
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(renderNow()));
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(clearDiff()));


    //thread = new QThread();
    //moveToThread(thread);
}

OpenGLWindow::~OpenGLWindow()
{
    timer->stop();
    delete timer;
    moveTimer->stop();
    delete moveTimer;
    delete m_context;
    delete m_device;
    //thread->deleteLater();
}

void OpenGLWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void OpenGLWindow::initialize()
{

}

void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}

bool OpenGLWindow::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest)
        renderNow();

    return QWindow::event(event);
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void OpenGLWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void OpenGLWindow::renderLater()
{
    //if (!m_update_pending) {
        //m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    //}
}

void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    //m_update_pending = false;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    if (r > 0) {
        r *= 0.995f;
        currentPos -= diffPos * r;
        if (r < 0.01) {
            setAnimating(false);
        }
    }

    render();

    if (r > 0)
        lastPos = currentPos;

    m_context->swapBuffers(this);
}


void OpenGLWindow::setAnimating(bool animating)
{
    if (animating == true) {
        r = 1;
        timer->start(20);
    } else {
        r = 0;
        timer->stop();
    }
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    setAnimating(false);
    mouseDown = true;
    lastPos = event->pos();
    currentPos = event->pos();
    diffPos = lastPos - currentPos;
    renderNow();
    moveTimer->start(200);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseDown) {
        currentPos = event->pos();
        diffPos = lastPos - currentPos;
        renderNow();
        lastPos = event->pos();
    }
}

void OpenGLWindow::clearDiff()
{
    diffPos = QPointF(0,0);
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent */* event */)
{
    if (currentPos + diffPos != lastPos) {
        //setAnimating(true);
    }

    mouseDown = false;
    renderNow();
    moveTimer->stop();
}

void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
    wheelDelta = event->delta();
    renderNow();
    wheelDelta = 0;
}
