/**
 * @file        HDF5Object.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with HDF5Group class declaration.
 *
 * @license     This file is partof the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5OBJECT_H
#define HDF5OBJECT_H

#include "HDF5File.h"
#include "HDF5Vector.h"

namespace HDF5Helper
{
class HDF5Object
{
public:
    HDF5Object(const hid_t object);
    ~HDF5Object();

    void setAttribute(const std::string name, const int value);
    void setAttribute(const std::string name, const hsize_t value);
    void setAttribute(const std::string name, const float value);
    void setAttribute(const std::string name, const double value);
    void setAttribute(const std::string name, const std::string value);

    float readAttributeF(const std::string name);
    hsize_t readAttributeI(const std::string name);
    std::string readAttributeS(const std::string name);

    HDF5Attribute *getAttribute(const std::string name);
    HDF5Attribute *getAttribute(const unsigned int idx);

    void setAttribute(HDF5Attribute *attribute, bool flag = true);

    void removeAttribute(const unsigned int idx, bool flag = true);
    void removeAttribute(const std::string name, bool flag = true);

    bool hasAttribute(const std::string name);

    unsigned int getNumAttrs();

    File *getFile();

protected:
    hid_t object;
    File *hDF5File;
    herr_t err;

private:
    void setAttribute(const std::string name, const hid_t type, const void *value, bool flag = true);
    void createAttribute(const std::string name, const hid_t type, const hid_t space, const void *value, bool flag = true);
    void creatingAttributeMessage(const std::string name, const hid_t type, const void *value);
    std::string getStringValueByType(const hid_t type, const void *value);
    std::string getStringTypeByType(const hid_t type);


};
}

#endif // HDF5OBJECT_H
