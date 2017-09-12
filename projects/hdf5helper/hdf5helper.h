﻿/**
 * @file        hdf5helper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              30 August    2017 (updated)
 *
 * @brief       The header file including all important HDF5Helper classes.
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

#ifndef HDF5HELPER_H
#define HDF5HELPER_H

/**
 * @namespace HDF5Helper
 * @brief Namespace for the %HDF5Helper library
 */
namespace HDF5Helper {}

#include <file.h>
#include <dataset.h>
#include <group.h>
#include <attribute.h>
#include <vector3d.h>
#include <vector4d.h>

/**
 * TODO:
 *  Q_DISABLE_COPY()
 *  Unify adding or removing slash in dataset and group names
 *  Test getOnlyName
 *  make vector4D .t as .w
 *
 *
 *
 *
 */

#endif // HDF5HELPER_H
