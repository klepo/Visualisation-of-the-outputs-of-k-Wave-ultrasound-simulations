/*
 * @file        HDF5Attribute.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Attribute class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5ATTRIBUTE_H
#define HDF5ATTRIBUTE_H

#include "HDF5Object.h"

class HDF5File::HDF5Object::HDF5Attribute
{
public:
    HDF5Attribute(H5::Attribute attribute);
    ~HDF5Attribute();

    H5::DataType getDataType();
    hsize_t getSize();
    std::string getName();
    H5::DataSpace getSpace();
    void *getData();

protected:
    H5::DataType type;
    hsize_t size;
    std::string name;
    H5::DataSpace space;
    void *buffer;
};

#endif // HDF5ATTRIBUTE_H
