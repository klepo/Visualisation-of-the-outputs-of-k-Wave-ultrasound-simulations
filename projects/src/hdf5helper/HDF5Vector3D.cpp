/*
 * @file        HDF5Vector3D.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Vector3D class definition.
 *              This class is for 3D point representation.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Vector3D.h"

namespace HDF5Helper {

HDF5Vector3D::HDF5Vector3D()
    : HDF5Vector(3)
{
    set(0, 0, 0);
}

HDF5Vector3D::HDF5Vector3D(HDF5Vector &hDF5Vector)
    : HDF5Vector(3)
{
    if (3 != hDF5Vector.getLength()){
        throw std::runtime_error("Convert error - Length of HDF5Vector is not 3");
    }
    set(hDF5Vector[0], hDF5Vector[1], hDF5Vector[2]);
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
