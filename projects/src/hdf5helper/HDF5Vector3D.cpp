/**
 * @file        HDF5Vector3D.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The implementation file containing HDF5Vector3D class definition.
 *              This class is for 3D point representation.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5Vector3D.h"

namespace HDF5Helper {

HDF5Vector3D::HDF5Vector3D()
    : HDF5Vector(3)
{
    set(0, 0, 0);
}

HDF5Vector3D::HDF5Vector3D(const HDF5Vector &hDF5Vector)
    : HDF5Vector(3)
{
    HDF5Vector tmp = hDF5Vector;
    hsize_t length = hDF5Vector.getLength();

    if (3 > length){
        throw std::runtime_error("Convert error - Length of HDF5Vector is not 3");
    }

    set(tmp[length - 3], tmp[length - 2], tmp[length - 1]);
}

HDF5Vector3D::HDF5Vector3D(const hsize_t value) : HDF5Vector(3 , value)
{

}

HDF5Vector3D::HDF5Vector3D(const hsize_t z, const hsize_t y, const hsize_t x)
    : HDF5Vector(3)
{
    set(z, y, x);
}

void HDF5Vector3D::set(const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = z;
    vector[1] = y;
    vector[2] = x;
}

void HDF5Vector3D::set(const int z, const int y, const int x)
{
    set(static_cast<hsize_t>(z), static_cast<hsize_t>(y), static_cast<hsize_t>(x));
}

void HDF5Vector3D::x(const hsize_t x)
{
    vector[2] = x;
}

void HDF5Vector3D::y(const hsize_t y)
{
    vector[1] = y;
}

void HDF5Vector3D::z(const hsize_t z)
{
    vector[0] = z;
}

void HDF5Vector3D::x(const int x)
{
    this->x(static_cast<hsize_t>(x));
}

void HDF5Vector3D::y(const int y)
{
    this->y(static_cast<hsize_t>(y));
}

void HDF5Vector3D::z(const int z)
{
    this->z(static_cast<hsize_t>(z));
}

hsize_t HDF5Vector3D::x() const
{
    return vector[2];
}

hsize_t HDF5Vector3D::y() const
{
    return vector[1];
}

hsize_t HDF5Vector3D::z() const
{
    return vector[0];
}
}
