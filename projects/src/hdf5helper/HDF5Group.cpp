/**
 * @file        HDF5Group.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The implementation file containing HDF5Group class definition.
 *              This class is for better work with HDF5 group.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5Group.h"
#include <HDF5File.h>

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
    if (deleteLog)
        std::cout << "Closing group \"" << name << "\"";
    err = H5Gclose(group);
    if (err < 0){
        throw std::runtime_error("H5Gclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (deleteLog)
        std::cout << " ... OK" << std::endl;

}

HDF5Dataset *HDF5Group::openDataset(const std::string datasetName, bool log)
{
    if (name == "/")
        return hDF5File->openDataset(datasetName, log);
    else
        return hDF5File->openDataset(name + "/" + datasetName, log);
}

HDF5Dataset *HDF5Group::openDataset(hsize_t idx, bool log)
{
    return openDataset(getObjNameByIdx(idx), log);
}

void HDF5Group::closeDataset(const std::string datasetName, bool log)
{
    if (name == "/")
        hDF5File->closeDataset(datasetName, log);
    else
        hDF5File->closeDataset(name + "/" + datasetName, log);
}

void HDF5Group::closeDataset(hsize_t idx, bool log)
{
    closeDataset(getObjNameByIdx(idx), log);
}

void HDF5Group::closeDataset(HDF5Helper::HDF5Dataset *dataset, bool log)
{
    closeDataset(dataset->getName(), log);
}

void HDF5Group::createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite)
{
    hDF5File->createDatasetI(name + "/" + datasetName, size, chunk_size, rewrite);
}

void HDF5Group::createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite)
{
    hDF5File->createDatasetF(name + "/" + datasetName, size, chunk_size, rewrite);
}

HDF5Group *HDF5Group::openGroup(const std::string groupName, bool log)
{
    if (name == "/")
        return hDF5File->openGroup(groupName, log);
    else
        return hDF5File->openGroup(name + "/" + groupName, log);
}

HDF5Group *HDF5Group::openGroup(hsize_t idx, bool log)
{
    return openGroup(getObjNameByIdx(idx), log);
}

void HDF5Group::closeGroup(const std::string groupName, bool log)
{
    if (name == "/")
        hDF5File->closeGroup(groupName, log);
    else
        hDF5File->closeGroup(name + "/" + groupName, log);
}

void HDF5Group::closeGroup(hsize_t idx, bool log)
{
    closeGroup(getObjNameByIdx(idx), log);
}

void HDF5Group::closeGroup(HDF5Group *group, bool log)
{
    closeGroup(group->getName(), log);
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
