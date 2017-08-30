/**
 * @file        attribute.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with HDF5Helper::Attribute class declaration.
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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>

#ifdef __unix
    #include <stdexcept>
#endif

#include <hdf5.h>  // HDF5

namespace HDF5Helper
{
/**
 * @brief The Attribute class represents wrapper for the HDF5 attributes
 */
class Attribute
{
public:
    Attribute(hid_t object, std::string name);
    Attribute(hid_t object, hsize_t idx);
    ~Attribute();

    hid_t getDatatype() const;
    hsize_t getSize() const;
    std::string getName() const;
    hid_t getDataspace() const;
    void *getData();
    std::string getStringValue() const;
    std::string getStringDatatype() const;
    static std::string getStringValue(const hid_t datatype, const void *value, hsize_t size = 0);
    static std::string getStringDatatype(const hid_t datatype);

private:
    void loadAttribute(hid_t attribute);

    hid_t attribute;
    std::string name;
    hid_t datatype;
    hsize_t size;
    hid_t dataspace;
    void *value;
    herr_t err;
};
}

#endif // ATTRIBUTE_H
