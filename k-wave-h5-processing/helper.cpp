/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The implementation file containing Helper class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper {

bool enableDebugMsgs = true;

/**
 * @brief Prints debug title
 * @param[in] str Debugging string
 */
void printDebugTitle(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << std::endl << "---- " << str << " ----" << std::endl << std::endl;
}

/**
 * @brief Prints debug time
 * @param[in] ofWhat Time of what?
 * @param[in] t0 Start time
 * @param[in] t1 End time
 */
void printDebugTime(std::string ofWhat, double t0, double t1)
{
    printDebugTwoColumns2S("Time of " + ofWhat + ":", std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints debug message
 * @param[in] str Debugging message
 */
void printDebugMsg(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << str << std::endl;
}

/**
 * @brief Prints message
 * @param[in] str String
 */
void printMsg(std::string str)
{
    std::cout << str << std::endl;
}

/**
 * @brief Prints start debug message
 * @param[in] str Debugging string
 */
void printDebugMsgStart(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << str << " ... ";
}

/**
 * @brief Prints debug message with 2 spaces
 * @param[in] str Debugging string
 */
void printDebugMsg2S(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << "  " << str << std::endl;
}

/**
 * @brief Prints debug into two columns with tabulator
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsTab(std::string first, std::string second, unsigned int width)
{
    if (!enableDebugMsgs)
        return;
    unsigned int widthTmp = width;
    if (widthTmp <= first.length())
        widthTmp = (static_cast<unsigned int>(first.length()) / 10) * 10 + 10;
    std::cout << "\t" << std::left << std::setw(widthTmp) << first << std::left << std::setw(static_cast<unsigned int>(second.length())) << second << std::endl;
}

/**
 * @brief Prints debug into two columns with tabulator
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsTab(std::string first, unsigned long long second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

/**
 * @brief Prints debug into two columns with tabulator
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsTab(std::string first, int second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

/**
 * @brief Prints debug into two columns with tabulator
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsTab(std::string first, float second, unsigned int width)
{
    std::ostringstream out;
    out.precision(4);
    out << second;
    printDebugTwoColumnsTab(first, out.str(), width);
}

/**
 * @brief Prints debug into two columns with tabulator
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsTab(std::string first, double second, unsigned int width)
{
    printDebugTwoColumnsTab(first, std::to_string(second), width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumns2S(std::string first, std::string second, unsigned int width)
{
    if (!enableDebugMsgs)
        return;
    unsigned int widthTmp = width;
    if (widthTmp <= first.length())
        widthTmp = (static_cast<unsigned int>(first.length()) / 10) * 10 + 10;
    std::cout << "  " << std::left << std::setw(widthTmp) << first << std::left << std::setw(static_cast<unsigned int>(second.length())) << second << std::endl;
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumns2S(std::string first, int second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumns2S(std::string first, unsigned long long second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumns2S(std::string first, float second, unsigned int width)
{
    std::ostringstream out;
    out.precision(4);
    out << second;
    printDebugTwoColumns2S(first, out.str(), width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumns2S(std::string first, double second, unsigned int width)
{
    printDebugTwoColumns2S(first, std::to_string(second), width);
}

/**
 * @brief Prints debug string
 * @param[in] str Debugging string
 */
void printDebugString(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << str;
}

/**
 * @brief Prints error message
 * @param[in] str Debugging string
 */
void printErrorMsg(std::string str)
{
    std::cerr << str << std::endl;
}

/**
 * @brief Round
 * @param[in] number Float number
 * @param[in] n Number of decimal places
 * @return Rounded number
 */
float roundf(float number, int n)
{
    if (n != 0) {
        float c = powf(10.0, float(n));
        float c1 = roundf(number * c);
        return c1 / c;
    } else {
        return (number >= 0.0f) ? floorf(number + 0.5f) : ceilf(number - 0.5f);
    }
}

/**
 * @brief Round
 * @param[in] number Float number
 * @return Rounded number
 */
unsigned long long round(float number)
{
    return static_cast<unsigned long long>(roundf(number));
}

}
