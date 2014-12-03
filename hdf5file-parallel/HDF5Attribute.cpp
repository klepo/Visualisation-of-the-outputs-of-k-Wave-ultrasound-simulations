/*
 * @file        HDF5Attribute.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Attribute class definition.
 *              This class is for better work with HDF5 attributes (copying).
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Attribute.h"

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::HDF5Attribute
 * @param attribute
 * @throw std::runtime_error
 */
HDF5File::HDF5Object::HDF5Attribute::HDF5Attribute(hid_t attribute)
{
    type = H5Aget_type(attribute);
    size = H5Aget_storage_size(attribute);
    ssize_t nameSize = H5Aget_name(attribute, 0, NULL);
    char *nameC = new char[nameSize];
    H5Aget_name(attribute, nameSize, nameC);
    name = std::string(nameC);
    delete [] nameC;
    space = H5Aget_space(attribute);
    buffer = malloc(size);
    H5Aread(attribute, type, buffer);
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::~HDF5Attribute
 */
HDF5File::HDF5Object::HDF5Attribute::~HDF5Attribute()
{
    free(buffer);
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::getDataType
 * @return data type of attribute
 */
hid_t HDF5File::HDF5Object::HDF5Attribute::getDataType()
{
    return type;
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::getSize
 * @return size of attribute
 */
hsize_t HDF5File::HDF5Object::HDF5Attribute::getSize()
{
    return size;
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::getName
 * @return name of attribute
 */
std::string HDF5File::HDF5Object::HDF5Attribute::getName()
{
    return name;
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::getSpace
 * @return data space of attribute
 */
hid_t HDF5File::HDF5Object::HDF5Attribute::getSpace()
{
    return space;
}

/**
 * @brief HDF5File::HDF5Object::HDF5Attribute::getData
 * @return data of attribute
 */
void *HDF5File::HDF5Object::HDF5Attribute::getData()
{
    return buffer;
}

