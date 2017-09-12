/**
 * @file        group.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              30 August    2017 (updated)
 *
 * @brief       The header file with HDF5Helper::Group class declaration.
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

#ifndef GROUP_H
#define GROUP_H

#include <object.h>
#include <vector3d.h>
#include <vector4d.h>
#include <dataset.h>

namespace HDF5Helper
{
/**
 * @brief The Group class represents wrapper for the HDF5 groups
 */
class Group : public Object
{
public:
    Group(const hid_t group, const std::string name, File *file);
    ~Group();

    Dataset *openDataset(const std::string name, bool log = true);
    Dataset *openDataset(hsize_t idx, bool log = true);

    void closeDataset(const std::string name, bool log = true);
    void closeDataset(hsize_t idx, bool log = true);
    void closeDataset(Dataset *dataset, bool log = true);

    void createDatasetI(const std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDatasetF(const std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);

    Group *openGroup(const std::string name, bool log = true);
    Group *openGroup(hsize_t idx, bool log = true);

    void closeGroup(const std::string name, bool log = true);
    void closeGroup(hsize_t idx, bool log = true);
    void closeGroup(Group *group, bool log = true);

    void createGroup(const std::string name, bool rewrite = false, bool log = true);

    hid_t getId();
    hsize_t getNumObjs();
    std::string getObjNameByIdx(hsize_t idx);
    H5G_obj_t getObjTypeByIdx(hsize_t idx);

private:
    hid_t group;
};

/// Map of groups datatype
typedef std::map<const std::string, Group *> MapOfGroups;

/// Pair of groups datatype
typedef std::pair<const std::string, Group *> PairOfGroups;
}

#endif // GROUP_H
