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
    class HDF5Attribute;

    HDF5Object(const hid_t object);
    ~HDF5Object();

    void setAttribute(const std::string name, const int value);
    void setAttribute(const std::string name, const uint64_t value);
    void setAttribute(const std::string name, const float value);
    void setAttribute(const std::string name, const double value);
    void setAttribute(const std::string name, const std::string value);

    float readAttributeF(const std::string name);
    uint64_t readAttributeI(const std::string name);
    std::string readAttributeS(const std::string name);

    HDF5Attribute *getAttribute(const std::string name);
    HDF5Attribute *getAttribute(const unsigned int idx);

    void setAttribute(HDF5Attribute *attribute);

    void removeAttribute(const unsigned int idx);
    void removeAttribute(const std::string name);

    bool hasAttribute(const std::string name);

    int getNumAttrs();

protected:
    hid_t object;
    HDF5File *hDF5File;
    herr_t err;

private:
    void setAttribute(const std::string name, const hid_t type, const void *value);
    void createAttribute(const std::string name, const hid_t type, const hid_t space, const void *value);
    void creatingAttributeMessage(const std::string name, const hid_t type, const void *value);
    std::string getStringValueByType(const hid_t type, const void *value);
    std::string getStringTypeByType(const hid_t type);


};

#endif // HDF5OBJECT_H