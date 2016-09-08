/**
 * @file        helper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        8  September 2016 (created)
 *
 * @brief       The header file with Helper namespace declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HELPER_H
#define HELPER_H

#include <hdf5helper.h>

namespace Helper
{

void printDebugTitle(std::string msg);
void printDebugMsg(std::string msg);
void printErrorMsg(std::string msg);
hsize_t round(double number);

}

#endif // HELPER_H
