#include "openglwindow.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <qDebug>
#include <QThread>
#include <QTime>

OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_context(0)
    , m_device(0)
    , m_update_pending(false)
    , mouseDown(false)
    , leftButton(false)
    , rightButton(false)
    , wheelDelta(0)
{
    //setOpacity(0.5);
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat surfaceFormat = requestedFormat();

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSamples(4);
    //surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(renderNow()));
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(clearDiff()));
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
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void OpenGLWindow::renderNow()
{
    //QTime timer;
    //timer.start();
    if (!isExposed())
        return;

    m_update_pending = false;

    bool needsInitialize = false;

    if (!m_context) {
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


void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::RightButton)
        rightButton = true;
    if (event->buttons() == Qt::LeftButton)
        leftButton = true;

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

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *)
{
    if (currentPos + diffPos != lastPos) {
        //setAnimating(true);
    }

    rightButton = false;
    leftButton = false;
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
