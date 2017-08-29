/**
 * @file        object.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with Group class declaration.
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <iostream>

#include <hdf5.h>  // HDF5

#include <attribute.h>

namespace HDF5Helper
{
class File;
/**
 * @brief The Object class represents wrapper for the HDF5 objects
 */
class Object
{
public:
    Object(const hid_t object, std::string name);
    ~Object();

    void setAttribute(Attribute *attribute, bool log = true);
    void setAttribute(const std::string name, const int value, bool log = true);
    void setAttribute(const std::string name, const hsize_t value, bool log = true);
    void setAttribute(const std::string name, const double value, bool log = true);
    void setAttribute(const std::string name, const float value, bool log = true);
    void setAttribute(const std::string name, const std::string value, bool log = true);

    float readAttributeF(const std::string name, bool log = true) const;
    hsize_t readAttributeI(const std::string name, bool log = true) const;
    std::string readAttributeS(const std::string name, bool log = true) const;

    Attribute *getAttribute(const std::string name) const;
    Attribute *getAttribute(const hsize_t idx) const;

    void removeAttribute(const std::string name, bool log = true);
    void removeAttribute(const unsigned int idx, bool log = true);

    bool hasAttribute(const std::string name) const;

    hsize_t getNumAttrs() const;
    std::string getName() const;
    std::string getOnlyName() const;
    File *getFile();

    void setDeleteLog(bool value);

    friend std::ostream &operator<<(std::ostream &os, const Object &object) {
        os << std::string(object);
        return os;
    }

    operator std::string() const;

protected:
    hid_t object;
    std::string name;
    File *file;
    herr_t err;
    bool deleteLog = true;

private:
    void createAttribute(const std::string name, const hid_t datatype, const hid_t dataspace, const void *value, bool log = true);
    void setAttribute(const std::string name, const hid_t datatype, const void *value, bool log = true);
    void creatingAttributeMessage(const std::string name, const hid_t type, const void *value);
};
}

#endif // OBJECT_H
