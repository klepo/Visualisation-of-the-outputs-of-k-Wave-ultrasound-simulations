/**
 * @file        group.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The header file with H5Helper::Group class declaration.
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

#ifndef GROUP_H
#define GROUP_H

#include "vector.h"
#include "object.h"
#include "dataset.h"
#include "file.h"

namespace H5Helper
{
/**
 * @brief The Group class represents wrapper for the HDF5 groups
 */
class Group : public Object
{
public:
    Group(hid_t groupId, std::string name, File *file);
    ~Group();

    Dataset *openDataset(std::string name, bool log = true) const;
    Dataset *openDataset(hsize_t idx, bool log = true) const;

    void closeDataset(std::string name, bool log = true) const;
    void closeDataset(hsize_t idx, bool log = true) const;
    void closeDataset(const Dataset *dataset, bool log = true) const;

    void createDatasetI(std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true) const;
    void createDatasetF(std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true) const;

    Group *openGroup(std::string name, bool log = true) const;
    Group *openGroup(hsize_t idx, bool log = true) const;

    void closeGroup(std::string name, bool log = true) const;
    void closeGroup(hsize_t idx, bool log = true) const;
    void closeGroup(const Group *groupId, bool log = true) const;

    void createGroup(std::string name, bool rewrite = false, bool log = true) const;

    hid_t getId() const;
    hsize_t getNumObjs() const;
    std::string getObjNameByIdx(hsize_t idx) const;
    H5G_obj_t getObjTypeByIdx(hsize_t idx) const;

private:
    /// Disable copy contructor
    Group(const Group &);
    /// \brief Disable assignment operator
    /// \return Group
    Group &operator=(const Group &);

    /// Group id
    hid_t groupId;
};

}

#endif // GROUP_H
