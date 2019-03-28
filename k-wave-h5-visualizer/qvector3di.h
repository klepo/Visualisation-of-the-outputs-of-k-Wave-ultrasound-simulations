/**
 * @file        qvector3di.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with QVector3DI class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef QVECTOR3DI_H
#define QVECTOR3DI_H

#include <QVector3D>

/**
 * @brief The QVector3DI class represents wrapper for integer QVector3D
 */
class QVector3DI : public QVector3D
{
public:
    QVector3DI();
    QVector3DI(int x, int y, int z);
    QVector3DI(unsigned long long x, unsigned long long y, unsigned long long z);
    void setX(int x);
    void setY(int y);
    void setZ(int z);
    int x() const;
    int y() const;
    int z() const;
};

#endif // QVECTOR3DI_H
