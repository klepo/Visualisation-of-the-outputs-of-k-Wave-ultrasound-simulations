/**
 * @file        qdoublespinboxts.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) \n
 *
 * @brief       The implementation file containing the QVector3DI class.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

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


