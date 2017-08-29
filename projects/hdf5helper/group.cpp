/**
 * @file        group.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Group class definition.
 *              This class is for better work with HDF5 group.
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

#include <group.h>
#include <file.h>

namespace HDF5Helper {

/**
 * @brief Creates Group object with given file, name and group
 * @param[in] group Group id
 * @param[in] name Name of group
 * @param[in] file HDF5 File
 */
Group::Group(const hid_t group, const std::string name, File *file) : Object(group, name) {
    this->file = file;
    this->group = group;
    object = this->group;
}

/**
 * @brief Destructor of Group object
 *
 * Closes HDF5 group.
 */
Group::~Group()
{
    if (deleteLog)
        std::cout << "Closing group \"" << name << "\"";
    err = H5Gclose(group);
    if (err < 0) {
        //throw std::runtime_error("H5Gclose error");
    }
    if (deleteLog)
        std::cout << " ... OK" << std::endl;

}

/**
 * @brief Opens dataset by name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 * @return Opened dataset
 */
Dataset *Group::openDataset(const std::string name, bool log)
{
    if (this->name == "/")
        return file->openDataset(name, log);
    else
        return file->openDataset(this->name + "/" + name, log);
}

/**
 * @brief Opens dataset by index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @param[in] log Logging flag (optional)
 * @return Opened dataset
 */
Dataset *Group::openDataset(hsize_t idx, bool log)
{
    return openDataset(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes dataset with given name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 */
void Group::closeDataset(const std::string name, bool log)
{
    if (this->name == "/")
        file->closeDataset(name, log);
    else
        file->closeDataset(this->name + "/" + name, log);
}

/**
 * @brief Closes dataset with given index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @param[in] log Logging flag (optional)
 */
void Group::closeDataset(hsize_t idx, bool log)
{
    closeDataset(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes dataset with same name as given dataset has
 * @param[in] dataset Dataset
 * @param[in] log Logging flag (optional)
 */
void Group::closeDataset(HDF5Helper::Dataset *dataset, bool log)
{
    closeDataset(dataset->getName(), log);
}

/**
 * @brief Creates new integer dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 */
void Group::createDatasetI(const std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    file->createDatasetI(this->name + "/" + name, size, chunkSize, rewrite, log);
}

/**
 * @brief Creates new float dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 */
void Group::createDatasetF(const std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    file->createDatasetF(this->name + "/" + name, size, chunkSize, rewrite, log);
}

/**
 * @brief Opens group with given name in HDF5 file
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 * @return Opened group
 */
Group *Group::openGroup(const std::string name, bool log)
{
    if (this->name == "/")
        return file->openGroup(name, log);
    else
        return file->openGroup(this->name + "/" + name, log);
}

/**
 * @brief Opens group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 * @return Opened group
 */
Group *Group::openGroup(hsize_t idx, bool log)
{
    return openGroup(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes group with given name in HDF5 file
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 */
void Group::closeGroup(const std::string name, bool log)
{
    if (this->name == "/")
        file->closeGroup(name, log);
    else
        file->closeGroup(this->name + "/" + name, log);
}

/**
 * @brief Closes group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 */
void Group::closeGroup(hsize_t idx, bool log)
{
    closeGroup(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes group with same name as given group
 * @param[in] group Group
 * @param[in] log Logging flag (optional)
 */
void Group::closeGroup(Group *group, bool log)
{
    closeGroup(group->getName(), log);
}

/**
 * @brief Creates new group with given name
 * @param[in] name Name of group
 * @param[in] rewrite Flag for rewriting existing group (optional)
 * @param[in] log Logging flag (optional)
 */
void Group::createGroup(const std::string name, bool rewrite, bool log)
{
    file->createGroup(this->name + "/" + name, rewrite, log);
}

/**
 * @brief Returns id of group
 * @return Id of group
 */
hid_t Group::getId()
{
    return group;
}

/**
 * @brief Returns number of objects in group
 * @return Number of objects in group
 */
hsize_t Group::getNumObjs()
{
    return file->getNumObjs(group);
}

/**
 * @brief Returns object name by index
 * @param[in] idx Index of object in file
 * @param[in] groupId Group id (optional)
 * @return Object name
 */
std::string Group::getObjNameByIdx(hsize_t idx)
{
    return file->getObjNameByIdx(idx, group);
}

/**
 * @brief Returns object type by index
 * @param[in] idx Index of object in file
 * @param[in] groupId Group id (optional)
 * @return Object type
 */
H5G_obj_t Group::getObjTypeByIdx(hsize_t idx)
{
    return file->getObjTypeByIdx(idx, group);
}
}
