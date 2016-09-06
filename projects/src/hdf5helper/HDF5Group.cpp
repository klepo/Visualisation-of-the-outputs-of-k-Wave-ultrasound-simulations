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

HDF5Dataset *HDF5Group::openDataset(const std::string datasetName)
{
    return hDF5File->openDataset(name + "/" + datasetName);
}

HDF5Dataset *HDF5Group::openDataset(hsize_t idx)
{
    return hDF5File->openDataset(name + "/" + getObjNameByIdx(idx));
}

void HDF5Group::closeDataset(const std::string datasetName)
{
    hDF5File->closeDataset(name + "/" + datasetName);
}

void HDF5Group::closeDataset(hsize_t idx)
{
    hDF5File->closeDataset(name + "/" + getObjNameByIdx(idx));
}

void HDF5Group::createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite)
{
    hDF5File->createDatasetI(name + "/" + datasetName, size, chunk_size, rewrite);
}

void HDF5Group::createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite)
{
    hDF5File->createDatasetF(name + "/" + datasetName, size, chunk_size, rewrite);
}

HDF5Group *HDF5Group::openGroup(const std::string groupName)
{
    return hDF5File->openGroup(name + "/" + groupName);
}

HDF5Group *HDF5Group::openGroup(hsize_t idx)
{
    return hDF5File->openGroup(name + "/" + getObjNameByIdx(idx));
}

void HDF5Group::closeGroup(const std::string groupName)
{
    hDF5File->closeGroup(name + "/" + groupName);
}

void HDF5Group::createGroup(const std::string groupName, bool rewrite)
{
    hDF5File->createGroup(name + "/" + groupName, rewrite);
}

/**
 * @brief HDF5Group::getId Get id of group
 * @return id of group
 */
hid_t HDF5Group::getId()
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

std::string HDF5Group::getObjNameByIdx(hsize_t idx)
{
    return hDF5File->getObjNameByIdx(idx, group);
}

H5G_obj_t HDF5Group::getObjTypeByIdx(hsize_t idx)
{
    return hDF5File->getObjTypeByIdx(idx, group);
}
}
