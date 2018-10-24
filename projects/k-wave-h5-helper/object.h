/**
 * @file        object.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              23 October   2018 (updated)
 *
 * @brief       The header file with H5Helper::Object class declaration.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <iostream>
#include <map>

#include <hdf5.h>  // HDF5

#include "attribute.h"

namespace H5Helper
{
class File;
/**
 * @brief The Object class represents wrapper for the HDF5 objects
 */
class Object
{
public:
    Object(hid_t object, std::string name, File *file);
    virtual ~Object();

    void setAttribute(Attribute *attribute, bool log = true);
    void setAttribute(std::string name, int value, bool log = true);
    void setAttribute(std::string name, unsigned int value, bool log = true);
    void setAttribute(std::string name, hssize_t value, bool log = true);
    void setAttribute(std::string name, hsize_t value, bool log = true);
    void setAttribute(std::string name, float value, bool log = true);
    void setAttribute(std::string name, double value, bool log = true);
    void setAttribute(std::string name, long double value, bool log = true);
    void setAttribute(std::string name, std::string value, bool log = true);

    float readAttributeF(std::string name, bool log = true) const;
    hsize_t readAttributeI(std::string name, bool log = true) const;
    std::string readAttributeS(std::string name, bool log = true) const;

    Attribute *getAttribute(std::string name) const;
    Attribute *getAttribute(hsize_t idx) const;

    void removeAttribute(std::string name, bool log = true) const;
    void removeAttribute(unsigned int idx, bool log = true) const;

    bool hasAttribute(std::string name) const;

    void renameAttribute(std::string srcName, std::string dstName) const;

    hsize_t getNumAttrs() const;
    std::string getName() const;
    std::string getOnlyName() const;
    File *getFile() const;

    void setDeleteLog(bool value);
    bool getDeleteLog() const;

    /**
     * @brief Operator <<
     * @param[in] os std::ostream
     * @param[in] object Abject
     * @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &os, const Object &object) {
        os << std::string(object);
        return os;
    }

    operator std::string() const;

protected:
    /// Error handle
    herr_t err = 0;
    /// Delete log flag
    bool deleteLog = true;

private:
    Object(const Object &);
    Object &operator=(const Object &);
    void createAttribute(std::string name, hid_t datatype, hid_t dataspace, const void *value, bool log = true) const;
    void setAttribute(std::string name, hid_t datatype, const void *value, bool log = true) const;
    void creatingAttributeMessage(std::string name, hid_t datatype, const void *value) const;

    File *file;
    hid_t object;
    std::string name;
};

/// Map of objects datatype
typedef std::map<std::string, Object *> MapOfObjects;

/// Pair of objects datatype
typedef std::pair<std::string, Object *> PairOfObjects;
}

#endif // OBJECT_H
