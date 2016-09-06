/*
 * @file        HDF5Vector4D.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Vector4D class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5VECTOR4D_H
#define HDF5VECTOR4D_H

#include "HDF5Vector.h"
#include "HDF5Vector3D.h"

namespace HDF5Helper
{
class HDF5Vector4D : public HDF5Vector
{
public:
    HDF5Vector4D();
    HDF5Vector4D(const HDF5Vector &hDF5Vector);
    HDF5Vector4D(hsize_t value, const HDF5Vector3D &hDF5Vector);
    HDF5Vector4D(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const int w, const int z, const int y, const int x);
    void x(const hsize_t x);
    void y(const hsize_t y);
    void z(const hsize_t z);
    void w(const hsize_t w);
    void x(const int x);
    void y(const int y);
    void z(const int z);
    void w(const int w);
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
    hsize_t w() const;
};
}

#endif // HDF5VECTOR4D_H
