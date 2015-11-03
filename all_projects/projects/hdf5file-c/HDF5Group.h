/*
 * @file        HDF5Group.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Group class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5GROUP_H
#define HDF5GROUP_H

#include "HDF5Object.h"

class HDF5File::HDF5Group : public HDF5File::HDF5Object
{
public:
    HDF5Group(const hid_t group, const std::string name, HDF5File *hDF5File);
    ~HDF5Group();

    hsize_t getId();
    std::string getName();
    hsize_t getNumObjs();

private:
    hid_t group;
    std::string name;

};

#endif // HDF5GROUP_H
