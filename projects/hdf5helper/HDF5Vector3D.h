/**
 * @file        HDF5Vector3D.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with HDF5Vector3D class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5VECTOR3D_H
#define HDF5VECTOR3D_H

#include <HDF5Vector.h>

namespace HDF5Helper
{
class HDF5Vector4D;
class HDF5Vector3D : public HDF5Vector
{
public:
    HDF5Vector3D();
    HDF5Vector3D(const HDF5Vector &hDF5Vector);
    HDF5Vector3D(const HDF5Vector4D &hDF5Vector);
    HDF5Vector3D(const hsize_t value);
    HDF5Vector3D(const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const int z, const int y, const int x);
    void x(const hsize_t x);
    void y(const hsize_t y);
    void z(const hsize_t z);
    void x(const int x);
    void y(const int y);
    void z(const int z);
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
};
}

#endif // HDF5VECTOR3D_H
