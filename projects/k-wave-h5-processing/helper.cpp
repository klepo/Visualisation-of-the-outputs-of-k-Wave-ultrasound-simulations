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

bool enableDebugMsgs = true;

/**
 * @brief Prints debug title
 * @param[in] msg Debugging message
 */
void printDebugTitle(std::string msg)
{
    if (!enableDebugMsgs)
        return;
    std::cout << std::endl << "---- " << msg << " ----" << std::endl << std::endl;
}

void printDebugTime(std::string ofWhat, double t0, double t1)
{
    printDebugTwoColumns2S("Time of " + ofWhat, std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints debug message
 * @param[in] msg Debugging message
 */
void printDebugMsg(std::string msg)
{
    if (!enableDebugMsgs)
        return;
    std::cout << msg << std::endl;
}

void printDebugMsgStart(std::string msg)
{
    if (!enableDebugMsgs)
        return;
    std::cout << msg << " ... ";
}

void printDebugMsg2S(std::string msg)
{
    if (!enableDebugMsgs)
        return;
    std::cout << "  " << msg << std::endl;
}

void printDebugTwoColumnsTab(std::string first, std::string second, unsigned int width)
{
    if (!enableDebugMsgs)
        return;
    unsigned int widthTmp = width;
    if (widthTmp <= first.length())
        widthTmp = (static_cast<unsigned int>(first.length()) / 10) * 10 + 10;
    std::cout << "\t" << std::left << std::setw(widthTmp) << first << std::left << std::setw(static_cast<unsigned int>(second.length())) << second << std::endl;
}

void printDebugTwoColumnsTab(std::string first, unsigned long long second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

void printDebugTwoColumnsTab(std::string first, int second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

void printDebugTwoColumnsTab(std::string first, float second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

void printDebugTwoColumnsTab(std::string first, double second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}


void printDebugTwoColumns2S(std::string first, std::string second, unsigned int width)
{
    if (!enableDebugMsgs)
        return;
    unsigned int widthTmp = width;
    if (widthTmp <= first.length())
        widthTmp = (static_cast<unsigned int>(first.length()) / 10) * 10 + 10;
    std::cout << "  " << std::left << std::setw(widthTmp) << first << std::left << std::setw(static_cast<unsigned int>(second.length())) << second << std::endl;
}

void printDebugTwoColumns2S(std::string first, int second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

void printDebugTwoColumns2S(std::string first, unsigned long long second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

void printDebugTwoColumns2S(std::string first, float second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

void printDebugTwoColumns2S(std::string first, double second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

void printDebugString(std::string msg)
{
    if (!enableDebugMsgs)
        return;
    std::cout << msg;
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
