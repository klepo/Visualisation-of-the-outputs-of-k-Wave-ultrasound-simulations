/**
 * @file        HDF5Object.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with HDF5Group class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5OBJECT_H
#define HDF5OBJECT_H

#include <string>
#include <iostream>

#include <hdf5.h>  // HDF5

#include <HDF5Attribute.h>

namespace HDF5Helper
{
class File;
class HDF5Object
{
public:
    HDF5Object(const hid_t object, std::string name);
    ~HDF5Object();

    void setAttribute(const std::string name, const int value, bool log = true);
    void setAttribute(const std::string name, const hsize_t value, bool log = true);
    void setAttribute(const std::string name, const float value, bool log = true);
    void setAttribute(const std::string name, const double value, bool log = true);
    void setAttribute(const std::string name, const std::string value, bool log = true);

    float readAttributeF(const std::string name, bool log = true) const;
    hsize_t readAttributeI(const std::string name, bool log = true) const;
    std::string readAttributeS(const std::string name, bool log = true) const;

    HDF5Attribute *getAttribute(const std::string name) const;
    HDF5Attribute *getAttribute(const hsize_t idx) const;

    void setAttribute(HDF5Attribute *attribute, bool log = true);

    void removeAttribute(const unsigned int idx, bool log = true);
    void removeAttribute(const std::string name, bool log = true);

    bool hasAttribute(const std::string name) const;

    hsize_t getNumAttrs() const;

    std::string getName();

    File *getFile();

    void setDeleteLog(bool value);

    friend std::ostream &operator<<(std::ostream &os, const HDF5Object &hDF5Object) {
        os << std::string(hDF5Object);
        return os;
    }

    operator std::string() const;

protected:
    hid_t object;
    std::string name;
    File *hDF5File;
    herr_t err;
    bool deleteLog = true;

private:
    void setAttribute(const std::string name, const hid_t type, const void *value, bool log = true);
    void createAttribute(const std::string name, const hid_t type, const hid_t space, const void *value, bool log = true);
    void creatingAttributeMessage(const std::string name, const hid_t type, const void *value);
    std::string getStringValueByType(const hid_t type, const void *value) const;
    std::string getStringTypeByType(const hid_t type) const;

};
}

#endif // HDF5OBJECT_H
