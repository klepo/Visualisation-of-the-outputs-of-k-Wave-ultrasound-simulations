/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        8  September 2016 (created)
 *
 * @brief       The implementation file containing helper stuff.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper
{

void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

void printDebugMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

void printErrorMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

hsize_t round(double number)
{
    return static_cast<hsize_t>(floor(number + 0.5));
}

}
