/**
 * @file        group.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing H5Helper::Group class definition.
 *
 * This class is used for better work with k-Wave HDF5 groups.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "group.h"

namespace H5Helper
{

/**
 * @brief Creates Group object with given file, name and group
 * @param[in] groupId Group id
 * @param[in] name Name of group
 * @param[in] file HDF5 File
 */
Group::Group(hid_t groupId, std::string name, File *file)
    : Object(groupId, name, file)
    , groupId(groupId)
{
}

/**
 * @brief Destructor of Group object
 *
 * Closes HDF5 group.
 */
Group::~Group()
{
    Helper::printDebugMsgStart("Closing group \"" + getName() + "\"");
    err = H5Gclose(groupId);
    if (err < 0) {
        // throw std::runtime_error("H5Gclose error");
    }
    Helper::printDebugMsgEnd("OK");
}

/**
 * @brief Opens dataset by name in HDF5 file
 * @param[in] name Name of dataset
 * @return Opened dataset
 */
Dataset *Group::openDataset(std::string name) const
{
    return getFile()->openDataset(concatenatePath(getName(), name));
}

/**
 * @brief Opens dataset by index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @return Opened dataset
 */
Dataset *Group::openDataset(hsize_t idx) const
{
    return openDataset(getObjNameByIdx(idx));
}

/**
 * @brief Closes dataset with given name in HDF5 file
 * @param[in] name Name of dataset
 */
void Group::closeDataset(std::string name) const
{
    getFile()->closeDataset(concatenatePath(getName(), name));
}

/**
 * @brief Closes dataset with given index in HDF5 file
 * @param[in] idx Index of dataset in file
 */
void Group::closeDataset(hsize_t idx) const
{
    closeDataset(getObjNameByIdx(idx));
}

/**
 * @brief Closes dataset with same name as given dataset has
 * @param[in] dataset Dataset
 */
void Group::closeDataset(const Dataset *dataset) const
{
    closeDataset(dataset->getName());
}

/**
 * @brief Creates new 64-bit unsigned integer dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 */
void Group::createDatasetI(std::string name, Vector size, Vector chunkSize, bool rewrite) const
{
    getFile()->createDatasetI(concatenatePath(getName(), name), size, chunkSize, rewrite);
}

/**
 * @brief Creates new float dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 */
void Group::createDatasetF(std::string name, Vector size, Vector chunkSize, bool rewrite) const
{
    getFile()->createDatasetF(concatenatePath(getName(), name), size, chunkSize, rewrite);
}

/**
 * @brief Opens group with given name in HDF5 file
 * @param[in] name Name of group
 * @return Opened group
 */
Group *Group::openGroup(std::string name) const
{
    return getFile()->openGroup(concatenatePath(getName(), name));
}

/**
 * @brief Opens group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @return Opened group
 */
Group *Group::openGroup(hsize_t idx) const
{
    return openGroup(getObjNameByIdx(idx));
}

/**
 * @brief Closes group with given name in HDF5 file
 * @param[in] name Name of group
 */
void Group::closeGroup(std::string name) const
{
    getFile()->closeGroup(concatenatePath(getName(), name));
}

/**
 * @brief Closes group with given index in HDF5 file
 * @param[in] idx Index of group in file
 */
void Group::closeGroup(hsize_t idx) const
{
    closeGroup(getObjNameByIdx(idx));
}

/**
 * @brief Closes group with same name as given group
 * @param[in] group Group
 */
void Group::closeGroup(const Group *group) const
{
    closeGroup(group->getName());
}

/**
 * @brief Creates new group with given name
 * @param[in] name Name of group
 * @param[in] rewrite Flag for rewriting existing group (optional)
 */
void Group::createGroup(std::string name, bool rewrite) const
{
    getFile()->createGroup(concatenatePath(getName(), name), rewrite);
}

/**
 * @brief Returns id of group
 * @return Id of group
 */
hid_t Group::getId() const
{
    return groupId;
}

/**
 * @brief Returns number of objects in group
 * @return Number of objects in group
 */
hsize_t Group::getNumObjs() const
{
    return getFile()->getNumObjs(groupId);
}

/**
 * @brief Returns object name by index
 * @param[in] idx Index of object in file
 * @return Object name
 */
std::string Group::getObjNameByIdx(hsize_t idx) const
{
    return getFile()->getObjNameByIdx(idx, groupId);
}

/**
 * @brief Returns object type by index
 * @param[in] idx Index of object in file
 * @return Object type
 */
H5G_obj_t Group::getObjTypeByIdx(hsize_t idx) const
{
    return getFile()->getObjTypeByIdx(idx, groupId);
}
} // namespace H5Helper
