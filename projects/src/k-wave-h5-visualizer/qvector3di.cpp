#include "qvector3di.h"


QVector3DI::QVector3DI() : QVector3D()
{

}

QVector3DI::QVector3DI(int xpos, int ypos, int zpos) : QVector3D(float(xpos), float(ypos), float(zpos))
{

}

void QVector3DI::setX(int x)
{
    QVector3D::setX(x);
}

void QVector3DI::setY(int y)
{
    QVector3D::setY(y);
}

void QVector3DI::setZ(int z)
{
    QVector3D::setZ(z);
}

int QVector3DI::x() const
{
    return int(QVector3D::x());
}

int QVector3DI::y() const
{
    return int(QVector3D::y());
}

int QVector3DI::z() const
{
    return int(QVector3D::z());
}


