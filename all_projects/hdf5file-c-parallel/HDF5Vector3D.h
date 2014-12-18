/*
 * @file        HDF5Vector3D.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Vector3D class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5VECTOR3D_H
#define HDF5VECTOR3D_H

#include "HDF5File.h"

class HDF5File::HDF5Vector3D
{
public:
    HDF5Vector3D();
    HDF5Vector3D(const hsize_t z, const hsize_t y, const hsize_t x);
    ~HDF5Vector3D();
    void set(HDF5Vector3D vector);
    void set(const hsize_t z, const hsize_t y, const hsize_t x);
    hsize_t size();
    hsize_t &x();
    hsize_t &y();
    hsize_t &z();
    hsize_t *getVectorPtr();
    hsize_t &operator [](uint i);
private:
    hsize_t vector[3];
};

#endif // HDF5VECTOR3D_H
