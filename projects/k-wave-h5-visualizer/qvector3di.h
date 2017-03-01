/**
 * @file        qvector3di.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) \n
 *
 * @brief       The header file with QVector3DI class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef QVECTOR3DI_H
#define QVECTOR3DI_H

#include <QVector3D>

class QVector3DI : public QVector3D
{
public:
    QVector3DI();
    QVector3DI(int xpos, int ypos, int zpos);
    void setX(int x);
    void setY(int y);
    void setZ(int z);
    int x() const;
    int y() const;
    int z() const;

};

#endif // QVECTOR3DI_H
