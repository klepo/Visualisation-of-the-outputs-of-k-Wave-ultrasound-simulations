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

HDF5File::HDF5Vector3D::HDF5Vector3D()
{
    vector[0] = 0;
    vector[1] = 0;
    vector[2] = 0;
}

HDF5File::HDF5Vector3D::HDF5Vector3D(const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = z;
    vector[1] = y;
    vector[2] = x;
}

HDF5File::HDF5Vector3D::~HDF5Vector3D()
{
}

void HDF5File::HDF5Vector3D::set(HDF5Vector3D vector)
{
    this->vector[0] = vector[0];
    this->vector[1] = vector[1];
    this->vector[2] = vector[2];
}

void HDF5File::HDF5Vector3D::set(const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = z;
    vector[1] = y;
    vector[2] = x;
}

hsize_t HDF5File::HDF5Vector3D::size()
{
    return vector[0] * vector[1] * vector[2];
}

hsize_t &HDF5File::HDF5Vector3D::x()
{
    return vector[2];
}

hsize_t &HDF5File::HDF5Vector3D::y()
{
    return vector[1];
}

hsize_t &HDF5File::HDF5Vector3D::z()
{
    return vector[0];
}

hsize_t *HDF5File::HDF5Vector3D::getVectorPtr()
{
    return vector;
}

hsize_t &HDF5File::HDF5Vector3D::operator [](uint i)
{
    if (i > 2){
        throw std::runtime_error("Index to HDF5Vector3D is too big");
    }
    return vector[i];
}

HDF5File::HDF5Vector3D &HDF5File::HDF5Vector3D::operator =(HDF5Vector3D vector)
{
    this->set(vector);
    return *this;
}


