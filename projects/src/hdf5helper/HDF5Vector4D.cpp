/*
 * @file        HDF5Vector4D.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Vector4D class definition.
 *              This class is for 4D point representation.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Vector4D.h"

namespace HDF5Helper {

HDF5Vector4D::HDF5Vector4D()
    : HDF5Vector(4)
{
    set(0, 0, 0, 0);
}

HDF5Vector4D::HDF5Vector4D(HDF5Vector &hDF5Vector)
    : HDF5Vector(4)
{
    if (4 != hDF5Vector.getLength()){
        throw std::runtime_error("Convert error - Length of HDF5Vector is not 4");
    }
    set(hDF5Vector[0], hDF5Vector[1], hDF5Vector[2], hDF5Vector[3]);
}

HDF5Vector4D::HDF5Vector4D(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
    : HDF5Vector(4)
{
    set(w, z, y, x);
}

void HDF5Vector4D::set(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = w;
    vector[1] = z;
    vector[2] = y;
    vector[3] = x;
}

void HDF5Vector4D::x(const hsize_t x)
{
    vector[3] = x;
}

void HDF5Vector4D::y(const hsize_t y)
{
    vector[2] = y;
}

void HDF5Vector4D::z(const hsize_t z)
{
    vector[1] = z;
}

void HDF5Vector4D::w(const hsize_t w)
{
    vector[0] = w;
}

hsize_t HDF5Vector4D::x() const
{
    return vector[3];
}

hsize_t HDF5Vector4D::y() const
{
    return vector[2];
}

hsize_t HDF5Vector4D::z() const
{
    return vector[1];
}

hsize_t HDF5Vector4D::w() const
{
    return vector[0];
}
}
