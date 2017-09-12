/**
 * @file        attribute.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              30 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::Attribute class definition.
 *
 * This class is used for better work with k-Wave HDF5 attributes (e.g. copying).
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <attribute.h>

namespace HDF5Helper {

/**
 * @brief Creates Attribute object with given name and object
 * @param[in] object HDF5 group or dataset id
 * @param[in] name The name of attribute
 * @throw std::runtime_error
 */
Attribute::Attribute(hid_t object, std::string name)
{
    attribute = H5Aopen_name(object, name.c_str());
    if (attribute < 0) {
        throw std::runtime_error("H5Aopen_name error");
    }
    loadAttribute(attribute);
    err = H5Aclose(attribute);
    if (err < 0) {
        throw std::runtime_error("H5Aclose error");
    }
}

/**
 * @brief Creates Attribute object with given index and object
 * @param[in] object The id of HDF5 group or dataset
 * @param[in] idx The index of attribute
 * @throw std::runtime_error
 */
Attribute::Attribute(hid_t object, hsize_t idx)
{
    attribute = H5Aopen_by_idx(object, ".", H5_INDEX_NAME, H5_ITER_INC, unsigned(idx), 0, 0);
    if (attribute < 0) {
        throw std::runtime_error("H5Aopen_name error");
    }
    loadAttribute(attribute);
    err = H5Aclose(attribute);
    if (err < 0) {
        throw std::runtime_error("H5Aclose error");
    }
}

/**
 * @brief Destructor of Attribute object
 *
 * Deletes some buffer memory, closes datatype and dataspace.
 */
Attribute::~Attribute()
{
    free(value);
    err = H5Tclose(datatype);
    if (err < 0) {
        //throw std::runtime_error("H5Tclose error");
    }
    err = H5Sclose(dataspace);
    if (err < 0) {
        //throw std::runtime_error("H5Sclose error");
    }
}

/**
 * @brief Returns datatype of attribute
 * @return Datatype of attribute
 */
hid_t Attribute::getDatatype() const
{
    return datatype;
}

/**
 * @brief Returns size of attribute
 * @return Size of attribute
 */
hsize_t Attribute::getSize() const
{
    return size;
}

/**
 * @brief Returns name of attribute
 * @return Name of attribute
 */
std::string Attribute::getName() const
{
    return name;
}

/**
 * @brief Returns dataspace of attribute
 * @return Dataspace of attribute
 */
hid_t Attribute::getDataspace() const
{
    return dataspace;
}

/**
 * @brief Returns data of attribute
 * @return Data of attribute
 */
void *Attribute::getData()
{
    return value;
}

/**
 * @brief Returns attribute value as string
 * @return Attribute value as string
 */
std::string Attribute::getStringValue() const
{
    return getStringValue(datatype, value, size);
}

/**
 * @brief Returns attribute datatype as string
 * @return Attribute datatype as string
 */
std::string Attribute::getStringDatatype() const
{
    return getStringDatatype(datatype);
}

/**
 * @brief Returns attribute value as string
 * @param[in] datatype Attribute datatype
 * @param[in] value Attribute value
 * @param[in] size Attribute size (optional)
 * @return Attribute value as string
 */
std::string Attribute::getStringValue(const hid_t datatype, const void *value, hsize_t size)
{
    if (H5Tget_class(datatype) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(datatype)) {
            void *voidValue = const_cast<void *>(value);
            return std::string(*static_cast<char **>(voidValue));
        } else {
            if (size) {
                return std::string(static_cast<const char *>(value), size);
            } else {
                return static_cast<const char *>(value);
            }
        }
    } else if (H5Tequal(datatype, H5T_NATIVE_INT)) {
        return std::to_string(*static_cast<const int *>(value));
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        return std::to_string(*static_cast<const hsize_t *>(value));
    } else if (H5Tequal(datatype, H5T_NATIVE_INT64)) {
        return std::to_string(*static_cast<const hssize_t *>(value));
    } else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)) {
        return std::to_string(*static_cast<const double *>(value));
    } else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        return std::to_string(*static_cast<const float *>(value));
    } else {
        return static_cast<const char *>(value);
    }
}

/**
 * @brief Returns attribute datatype as string
 * @param[in] datatype Attribute datatype
 * @return Attribute datatype as string
 */
std::string Attribute::getStringDatatype(const hid_t datatype)
{
    if (H5Tget_class(datatype) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(datatype)) {
            return "H5T_C_S1, H5T_VARIABLE";
        } else {
            return "H5T_C_S1";
        }
    } else if (H5Tequal(datatype, H5T_NATIVE_INT)) {
        return "H5T_NATIVE_INT";
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        return "H5T_NATIVE_UINT64";
    } else if (H5Tequal(datatype, H5T_NATIVE_INT64)) {
        return "H5T_NATIVE_INT64";
    } else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)) {
        return "H5T_NATIVE_DOUBLE";
    } else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        return "H5T_NATIVE_FLOAT";
    } else {
        return std::to_string(datatype);
    }
}

/**
 * @brief Loads attribute data
 * @param[in] attribute Attribute id
 * @throw std::runtime_error
 */
void Attribute::loadAttribute(hid_t attribute)
{
    datatype = H5Aget_type(attribute);
    if (datatype < 0) {
        throw std::runtime_error("H5Aget_type error");
    }
    size = H5Aget_storage_size(attribute);
    ssize_t nameSize = H5Aget_name(attribute, 0, 0);
    if (nameSize < 0) {
        throw std::runtime_error("H5Aget_name error");
    }
    char *nameC = new char[size_t(nameSize) + 1];
    H5Aget_name(attribute, size_t(nameSize + 1), nameC);
    name = std::string(nameC);
    delete [] nameC;
    dataspace = H5Aget_space(attribute);
    if (dataspace < 0) {
        throw std::runtime_error("H5Aget_space error");
    }
    value = malloc(size_t(size));
    err = H5Aread(attribute, datatype, value);
    if (err < 0) {
        throw std::runtime_error("H5Aread error");
    }
}
}
