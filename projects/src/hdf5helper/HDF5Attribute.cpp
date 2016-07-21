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

namespace HDF5Helper {

void HDF5Attribute::loadAttribute(hid_t attribute)
{
    datatype = H5Aget_type(attribute);
    if (datatype < 0){
        throw std::runtime_error("H5Aget_type error");
        //MPI::COMM_WORLD.Abort(1);
    }
    size = H5Aget_storage_size(attribute);
    ssize_t nameSize = H5Aget_name(attribute, 0, NULL);
    if (nameSize < 0){
        throw std::runtime_error("H5Aget_name error");
        //MPI::COMM_WORLD.Abort(1);
    }
    char *nameC = new char[nameSize + 1];
    H5Aget_name(attribute, nameSize + 1, nameC);
    name = std::string(nameC);
    delete [] nameC;
    dataspace = H5Aget_space(attribute);
    if (dataspace < 0){
        throw std::runtime_error("H5Aget_space error");
        //MPI::COMM_WORLD.Abort(1);
    }
    buffer = malloc(size);
    err = H5Aread(attribute, datatype, buffer);
    if (err < 0){
        throw std::runtime_error("H5Aread error");
        //MPI::COMM_WORLD.Abort(1);
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
    if (attribute < 0){
        throw std::runtime_error("H5Aopen_name error");
        //MPI::COMM_WORLD.Abort(1);
    }
    loadAttribute(attribute);
    err = H5Aclose(attribute);
    if (err < 0){
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
}

HDF5Attribute::HDF5Attribute(hid_t object, hid_t idx)
{
    attribute = H5Aopen_idx(object, idx);
    if (attribute < 0){
        throw std::runtime_error("H5Aopen_name error");
        //MPI::COMM_WORLD.Abort(1);
    }
    loadAttribute(attribute);
    err = H5Aclose(attribute);
    if (err < 0){
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
}

/**
 * @brief HDF5Attribute::~HDF5Attribute
 */
HDF5Attribute::~HDF5Attribute()
{
    free(buffer);
    H5Tclose(datatype);
    H5Sclose(dataspace);
}

/**
 * @brief HDF5Attribute::getType
 * @return data type of attribute
 */
hid_t HDF5Attribute::getDatatype()
{
    return datatype;
}

/**
 * @brief HDF5Attribute::getSize
 * @return size of attribute
 */
hsize_t HDF5Attribute::getSize()
{
    return size;
}

/**
 * @brief HDF5Attribute::getName
 * @return name of attribute
 */
std::string HDF5Attribute::getName()
{
    return name;
}

/**
 * @brief HDF5Attribute::getSpace
 * @return data space of attribute
 */
hid_t HDF5Attribute::getDataspace()
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
}
