/*
 * @file        HDF5Group.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Group class definition.
 *              This class is for better work with HDF5 group.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Group.h"

namespace HDF5Helper {

/**
 * @brief HDF5Group::HDF5Group
 * @param group group (H5::Group type)
 * @param name name of group
 * @param hDF5File file
 * @throw std::runtime_error
 */
HDF5Group::HDF5Group(const hid_t group, const std::string name, File *hDF5File) : HDF5Object(group) {
    this->hDF5File = hDF5File;
    this->group = group;
    object = this->group;
    this->name = name;
}

/**
 * @brief HDF5Group::~HDF5Group
 */
HDF5Group::~HDF5Group()
{
    std::cout << "Closing group \"" << name << "\"";
    H5Gclose(group);
    std::cout << " ... OK" << std::endl;

}

/**
 * @brief HDF5Group::getId Get id of group
 * @return id of group
 */
hsize_t HDF5Group::getId()
{
    return group;
}

/**
 * @brief HDF5Group::getName Get name of group
 * @return name
 */
std::string HDF5Group::getName()
{
    return name;
}

/**
 * @brief HDF5Group::getNumObjs Get number of object in group
 * @return number of objects
 */
hsize_t HDF5Group::getNumObjs()
{
    H5G_info_t group_info;
    err = H5Gget_info(group, &group_info);
    if (err < 0){
        throw std::runtime_error("H5Gget_info error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return group_info.nlinks;
}
}
