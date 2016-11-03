/**
 * @file        HDF5Attribute.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with HDF5Attribute class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
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
    HDF5Attribute(hid_t object, hsize_t idx);
    ~HDF5Attribute();

    hid_t getDatatype() const;
    hsize_t getSize() const;
    std::string getName() const;
    hid_t getDataspace() const;
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
