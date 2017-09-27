/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Helper class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper {

/**
 * @brief Prints debug title
 * @param[in] msg Debugging message
 */
void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

/**
 * @brief Prints debug message
 * @param[in] msg Debugging message
 */
void printDebugMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

/**
 * @brief Prints error message
 * @param[in] msg Debugging message
 */
void printErrorMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

/**
 * @brief Round
 * @param[in] number Float number
 * @return Rounded number
 */
unsigned long long round(float number)
{
    return static_cast<unsigned long long>(floor(double(number) + 0.5));
}
}
