/*
 * @file        HDF5Object.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Group class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5OBJECT_H
#define HDF5OBJECT_H

#include "HDF5File.h"

class HDF5File::HDF5Object
{
public:
    HDF5Object(H5::H5Object *object);
    ~HDF5Object();

    class HDF5Attribute;

    void setAttribute(const H5std_string name, float value);
    void setAttribute(const H5std_string name, double value);
    void setAttribute(const H5std_string name, int value);
    void setAttribute(const H5std_string name, uint64_t value);
    void setAttribute(const H5std_string name, const H5std_string value);

    float readAttributeF(const H5std_string name);
    uint64_t readAttributeI(const H5std_string name);
    H5std_string readAttributeS(const H5std_string name);

    HDF5Attribute *getAttribute(H5std_string name);
    HDF5Attribute *getAttribute(const unsigned int idx);
    void setAttribute(HDF5Attribute *attribute);

    void removeAttribute(const unsigned int idx);
    void removeAttribute(H5std_string name);

    bool hasAttribute(H5std_string name);

    int getNumAttrs();

protected:
    H5::H5Object *object;

    HDF5File *hDF5File;
};

#endif // HDF5OBJECT_H
