/**
 * @file        HDF5Attribute.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing HDF5Attribute class definition.
 *              This class is for better work with HDF5 attributes (copying).
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <HDF5Attribute.h>

namespace HDF5Helper {

/**
 * @brief HDF5Attribute::loadAttribute
 * @param attribute
 * @throw std::runtime_error
 */
void HDF5Attribute::loadAttribute(hid_t attribute)
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
    buffer = malloc(size_t(size));
    err = H5Aread(attribute, datatype, buffer);
    if (err < 0) {
        throw std::runtime_error("H5Aread error");
    }
}

/**
 * @brief HDF5Attribute::HDF5Attribute
 * @param attribute
 * @throw std::runtime_error
 */
HDF5Attribute::HDF5Attribute(hid_t object, std::string name)
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
 * @brief HDF5Attribute::HDF5Attribute
 * @param object
 * @param idx
 * @throw std::runtime_error
 */
HDF5Attribute::HDF5Attribute(hid_t object, hsize_t idx)
{
    attribute = H5Aopen_idx(object, unsigned(idx));
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
 * @brief HDF5Attribute::~HDF5Attribute
 */
HDF5Attribute::~HDF5Attribute()
{
    free(buffer);
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
 * @brief HDF5Attribute::getType
 * @return data type of attribute
 */
hid_t HDF5Attribute::getDatatype() const
{
    return datatype;
}

/**
 * @brief HDF5Attribute::getSize
 * @return size of attribute
 */
hsize_t HDF5Attribute::getSize() const
{
    return size;
}

/**
 * @brief HDF5Attribute::getName
 * @return name of attribute
 */
std::string HDF5Attribute::getName() const
{
    return name;
}

/**
 * @brief HDF5Attribute::getSpace
 * @return data space of attribute
 */
hid_t HDF5Attribute::getDataspace() const
{
    return dataspace;
}

/**
 * @brief HDF5Attribute::getData
 * @return data of attribute
 */
void *HDF5Attribute::getData()
{
    return buffer;
}

std::string HDF5Attribute::getStringValue() const
{
    if (H5Tget_class(datatype) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(datatype)) {
            void *voidValue = const_cast<void *>(buffer);
            //return *(char **)(value);
            return std::string(*static_cast<char **>(voidValue));
        } else {
             return static_cast<const char *>(buffer);
        }
    } else if (H5Tequal(datatype, H5T_NATIVE_INT))
        return std::to_string(*static_cast<const int *>(buffer));
    else if (H5Tequal(datatype, H5T_NATIVE_UINT64))
        return std::to_string(*static_cast<const hsize_t *>(buffer));
    else if (H5Tequal(datatype, H5T_NATIVE_INT64))
        return std::to_string(*static_cast<const hssize_t *>(buffer));
    else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE))
        return std::to_string(*static_cast<const double *>(buffer));
    else if (H5Tequal(datatype, H5T_NATIVE_FLOAT))
        return std::to_string(*static_cast<const float *>(buffer));
    else
        return static_cast<const char *>(buffer);
}
}
