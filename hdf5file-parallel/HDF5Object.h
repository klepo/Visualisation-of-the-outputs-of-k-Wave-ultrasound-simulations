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
    HDF5Object(hid_t object);
    ~HDF5Object();

    class HDF5Attribute;

    void setAttribute(std::string name, int value);
    void setAttribute(std::string name, uint64_t value);
    void setAttribute(std::string name, float value);
    void setAttribute(std::string name, double value);
    void setAttribute(std::string name, std::string value);

    float readAttributeF(std::string name);
    uint64_t readAttributeI(std::string name);
    std::string readAttributeS(std::string name);

    HDF5Attribute *getAttribute(std::string name);
    HDF5Attribute *getAttribute(unsigned int idx);

    void setAttribute(HDF5Attribute *attribute);

    void removeAttribute(unsigned int idx);
    void removeAttribute(std::string name);

    bool hasAttribute(std::string name);

    int getNumAttrs();

protected:
    hid_t object;

    HDF5File *hDF5File;

private:
    void setAttribute(std::string name, hid_t type, void *value);
    void readAttribute(std::string name, void *value);

};

#endif // HDF5OBJECT_H
