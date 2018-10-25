/**
 * @file        attribute.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The header file with H5Helper::Attribute class declaration.
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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>

#ifdef __unix
#include <stdexcept>
#endif

#include <hdf5.h> // HDF5

namespace H5Helper
{
/**
 * @brief The Attribute class represents wrapper for the HDF5 attributes
 */
class Attribute
{
public:
    Attribute(hid_t objectId, std::string name);
    Attribute(hid_t objectId, hsize_t idx);
    ~Attribute();

    hid_t getDatatype() const;
    hsize_t getSize() const;
    std::string getName() const;
    hid_t getDataspace() const;
    void *getData() const;
    std::string getStringValue() const;
    std::string getStringDatatype() const;
    static std::string getStringValue(hid_t datatypeId, const void *value, hsize_t size = 0);
    static std::string getStringDatatype(hid_t datatypeId);

private:
    /// Disable copy contructor
    Attribute(const Attribute &);
    /// Disable assignment operator
    /// \return Attribute
    Attribute &operator=(const Attribute &);

    void loadAttribute(hid_t attributeId);

    /// Object id
    hid_t objectId;
    /// Attribute id
    hid_t attributeId;
    /// Attribute name
    std::string name;
    /// Attribute datatype id
    hid_t datatypeId;
    /// Attribute size
    hsize_t size;
    /// Attribute dataspace id
    hid_t dataspaceId;
    /// Attribute value
    void *value = nullptr;
    /// Error handle
    herr_t err;
};
}

#endif // ATTRIBUTE_H
