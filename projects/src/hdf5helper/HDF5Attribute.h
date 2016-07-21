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

namespace HDF5Helper
{
class HDF5Attribute
{
public:
    HDF5Attribute(hid_t object, std::string name);
    HDF5Attribute(hid_t object, hid_t idx);
    ~HDF5Attribute();

    hid_t getDatatype();
    hsize_t getSize();
    std::string getName();
    hid_t getDataspace();
    void *getData();

protected:
    hid_t attribute;
    std::string name;
    hid_t datatype;
    hsize_t size;
    hid_t dataspace;
    void *buffer;

private:
    void loadAttribute(hid_t attribute);
    herr_t err;

};
}

#endif // HDF5ATTRIBUTE_H
