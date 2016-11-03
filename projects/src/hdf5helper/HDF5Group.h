/**
 * @file        HDF5Group.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with HDF5Group class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5GROUP_H
#define HDF5GROUP_H

#include "HDF5Object.h"

namespace HDF5Helper
{
class HDF5Group : public HDF5Object
{
public:
    HDF5Group(const hid_t group, const std::string name, File *hDF5File);
    ~HDF5Group();

    HDF5Dataset *openDataset(const std::string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const std::string datasetName);
    void closeDataset(hsize_t idx);

    void createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite = false);
    void createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunk_size, bool rewrite = false);

    HDF5Group *openGroup(const std::string groupName);
    HDF5Group *openGroup(hsize_t idx);

    void closeGroup(const std::string groupName);

    void createGroup(const std::string groupName, bool rewrite = false);

    hid_t getId();
    std::string getName();
    hsize_t getNumObjs();
    std::string getObjNameByIdx(hsize_t idx);
    H5G_obj_t getObjTypeByIdx(hsize_t idx);

private:
    hid_t group;
    std::string name;

};
}

#endif // HDF5GROUP_H
