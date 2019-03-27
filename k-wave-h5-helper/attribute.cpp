/**
 * @file        attribute.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              29 November  2018 (updated)
 *
 * @brief       The implementation file containing H5Helper::Attribute class definition.
 *
 * This class is used for better work with k-Wave HDF5 attributes (e.g. copying).
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

#include "attribute.h"

namespace H5Helper {

/**
 * @brief Creates Attribute object with given name and object
 * @param[in] objectId HDF5 group or dataset id
 * @param[in] name The name of attribute
 * @throw std::runtime_error
 */
Attribute::Attribute(hid_t objectId, std::string name)
    : objectId(objectId)
{
    attributeId = H5Aopen_name(objectId, name.c_str());
    if (attributeId < 0) {
        throw std::runtime_error("H5Aopen_name error");
    }
    loadAttribute(attributeId);
    err = H5Aclose(attributeId);
    if (err < 0) {
        throw std::runtime_error("H5Aclose error");
    }
}

/**
 * @brief Creates Attribute object with given index and object
 * @param[in] objectId HDF5 group or dataset id
 * @param[in] idx The index of attribute
 * @throw std::runtime_error
 */
Attribute::Attribute(hid_t objectId, hsize_t idx)
    : objectId(objectId)
{
    attributeId = H5Aopen_by_idx(objectId, ".", H5_INDEX_NAME, H5_ITER_INC, unsigned(idx), 0, 0);
    if (attributeId < 0) {
        throw std::runtime_error("H5Aopen_name error");
    }
    loadAttribute(attributeId);
    err = H5Aclose(attributeId);
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
    value = nullptr;
    err = H5Tclose(datatypeId);
    if (err < 0) {
        //throw std::runtime_error("H5Tclose error");
    }
    err = H5Sclose(dataspaceId);
    if (err < 0) {
        //throw std::runtime_error("H5Sclose error");
    }
}

/**
 * @brief Returns datatype id of attribute
 * @return Datatype id of attribute
 */
hid_t Attribute::getDatatype() const
{
    return datatypeId;
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
 * @brief Returns dataspace id of attribute
 * @return Dataspace id of attribute
 */
hid_t Attribute::getDataspace() const
{
    return dataspaceId;
}

/**
 * @brief Returns data of attribute
 * @return Data of attribute
 */
void *Attribute::getData() const
{
    return value;
}

/**
 * @brief Returns attribute value as string
 * @return Attribute value as string
 */
std::string Attribute::getStringValue() const
{
    return getStringValue(datatypeId, value, size);
}

/**
 * @brief Returns attribute datatype as string
 * @return Attribute datatype as string
 */
std::string Attribute::getStringDatatype() const
{
    return getStringDatatype(datatypeId);
}

/**
 * @brief Returns attribute value as string
 * @param[in] datatypeId Attribute datatype id
 * @param[in] value Attribute value
 * @param[in] size Attribute size (optional)
 * @return Attribute value as string
 */
std::string Attribute::getStringValue(hid_t datatypeId, const void *value, hsize_t size)
{
    if (H5Tget_class(datatypeId) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(datatypeId)) {
            void *voidValue = const_cast<void *>(value);
            return std::string(*static_cast<char **>(voidValue));
        } else {
            if (size) {
                const char *str = static_cast<const char *>(value);
                if (str[size - 1] != '\0') {
                    return std::string(static_cast<const char *>(value), size);
                } else {
                    return std::string(static_cast<const char *>(value), size - 1);
                }
            } else {
                return static_cast<const char *>(value);
            }
        }
    } else if (H5Tequal(datatypeId, H5T_NATIVE_INT)) {
        return std::to_string(*static_cast<const int *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_UINT)) {
        return std::to_string(*static_cast<const unsigned int *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_INT64)) {
        return std::to_string(*static_cast<const hssize_t *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_UINT64)) {
        return std::to_string(*static_cast<const hsize_t *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_FLOAT)) {
        return std::to_string(*static_cast<const float *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_DOUBLE)) {
        return std::to_string(*static_cast<const double *>(value));
    } else if (H5Tequal(datatypeId, H5T_NATIVE_LDOUBLE)) {
        return std::to_string(*static_cast<const long double *>(value));
    } else {
        return static_cast<const char *>(value);
    }
}

/**
 * @brief Returns attribute datatype as string
 * @param[in] datatypeId Attribute datatype id
 * @return Attribute datatype as string
 */
std::string Attribute::getStringDatatype(hid_t datatypeId)
{
    if (H5Tget_class(datatypeId) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(datatypeId)) {
            return "H5T_C_S1, H5T_VARIABLE";
        } else {
            return "H5T_C_S1";
        }
    } else if (H5Tequal(datatypeId, H5T_NATIVE_INT)) {
        return "H5T_NATIVE_INT";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_UINT)) {
        return "H5T_NATIVE_UINT";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_INT64)) {
        return "H5T_NATIVE_INT64";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_UINT64)) {
        return "H5T_NATIVE_UINT64";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_FLOAT)) {
        return "H5T_NATIVE_FLOAT";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_DOUBLE)) {
        return "H5T_NATIVE_DOUBLE";
    } else if (H5Tequal(datatypeId, H5T_NATIVE_LDOUBLE)) {
        return "H5T_NATIVE_LDOUBLE";
    } else {
        return std::to_string(datatypeId);
    }
}

/**
 * @brief Loads attribute data
 * @param[in] attributeId Attribute id
 * @throw std::runtime_error
 */
void Attribute::loadAttribute(hid_t attributeId)
{
    datatypeId = H5Aget_type(attributeId);
    if (datatypeId < 0) {
        throw std::runtime_error("H5Aget_type error");
    }
    size = H5Aget_storage_size(attributeId);
    ssize_t nameSize = H5Aget_name(attributeId, 0, nullptr);
    if (nameSize < 0) {
        throw std::runtime_error("H5Aget_name error");
    }
    char *nameC = new char[size_t(nameSize) + 1]();
    H5Aget_name(attributeId, size_t(nameSize + 1), nameC);
    name = std::string(nameC);
    delete[] nameC;
    nameC = nullptr;
    dataspaceId = H5Aget_space(attributeId);
    if (dataspaceId < 0) {
        throw std::runtime_error("H5Aget_space error");
    }
    value = malloc(size_t(size));
    err = H5Aread(attributeId, datatypeId, value);
    if (err < 0) {
        throw std::runtime_error("H5Aread error");
    }
}
}
