/**
 * @file        qvector3di.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) \n
 *
 * @brief       The implementation file containing QVector3DI class definition.
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

#include "qvector3di.h"

/**
 * @brief Creates QVector3DI object
 */
QVector3DI::QVector3DI() : QVector3D()
{

}

/**
 * @brief Creates QVector3DI object
 * @param[in] x X position
 * @param[in] y Y position
 * @param[in] z Z position
 */
QVector3DI::QVector3DI(int x, int y, int z) : QVector3D(float(x), float(y), float(z))
{

}

/**
 * @brief Creates QVector3DI object
 * @param[in] x X position
 * @param[in] y Y position
 * @param[in] z Z position
 */
QVector3DI::QVector3DI(unsigned long long x, unsigned long long y, unsigned long long z) : QVector3D(float(x), float(y), float(z))
{

}

/**
 * @brief Sets X
 * @param[in] x X position
 */
void QVector3DI::setX(int x)
{
    QVector3D::setX(x);
}

/**
 * @brief Sets Y
 * @param[in] y Y position
 */
void QVector3DI::setY(int y)
{
    QVector3D::setY(y);
}

/**
 * @brief Sets Z
 * @param[in] z Z position
 */
void QVector3DI::setZ(int z)
{
    QVector3D::setZ(z);
}

/**
 * @brief Returns x position
 * @return X position
 */
int QVector3DI::x() const
{
    return int(QVector3D::x());
}

/**
 * @brief Returns y position
 * @return Y position
 */
int QVector3DI::y() const
{
    return int(QVector3D::y());
}

/**
 * @brief Returns z position
 * @return Z position
 */
int QVector3DI::z() const
{
    return int(QVector3D::z());
}


