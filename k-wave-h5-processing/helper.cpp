/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Helper class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
    printLine();
    std::cout << wrapMsg("::: " + str + " :::", 2, 0);
    printLine();
}

/**
 * @brief Prints debug time
 * @param[in] ofWhat Time of what?
 * @param[in] t0 Start time
 * @param[in] t1 End time
 */
void printDebugTime(std::string ofWhat, double t0, double t1)
{
    printDebugTwoColumnsS("Time of " + ofWhat + ":", std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints debug message
 * @param[in] str Debugging message
 */
void printDebugMsg(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << wrapMsg(str);
}

/**
 * @brief Prints start debug message
 * @param[in] str Debugging string
 */
void printDebugMsgStart(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << "| " << str << std::string(41 - str.length(), ' ');
}

/**
 * @brief Prints end debug message
 * @param[in] str Debugging string
 */
void printDebugMsgEnd(std::string str)
{
    if (!enableDebugMsgs)
        return;
    std::cout << std::string(20 - str.length(), ' ') << str << " |" << std::endl;
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, std::string second, size_t indentation, unsigned int width)
{
    if (!enableDebugMsgs)
        return;
    unsigned int widthTmp = width;
    if (widthTmp <= first.length())
        widthTmp = (static_cast<unsigned int>(first.length()) / 10) * 10 + 10;
    std::stringstream ss;
    ss << std::string(indentation, ' ') << std::left << std::setw(widthTmp) << first << std::left << std::setw(static_cast<unsigned int>(second.length())) << second;
    std::cout << wrapMsg(ss.str());
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, int second, size_t indentation, unsigned int width)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, unsigned long long second, size_t indentation, unsigned int width)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, float second, size_t indentation, unsigned int width)
{
    std::ostringstream out;
    out.precision(4);
    out << second;
    printDebugTwoColumnsS(first, out.str(), indentation, width);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, double second, size_t indentation, unsigned int width)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, width);
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
 * @brief Prints error message
 * @param[in] str Debugging string
 */
void printErrorMsg(std::string str)
{
    printLineErr();
    std::cerr << wrapMsg("Error: " + str);
    printLineErr();
}

void printDebugLine()
{
    if (!enableDebugMsgs)
        return;
    std::cout << "+" << std::string(63, '-') << "+" << std::endl;
}


void printLine()
{
    std::cout << "+" << std::string(63, '-') << "+" << std::endl;
}

void printLineErr()
{
    std::cerr << "+" << std::string(63, '-') << "+" << std::endl;
}

std::string wrapMsg(std::string str, size_t indentation, size_t firstIndentation)
{
    size_t s = 61;
    std::string strIn = str;
    std::string strOut = "| " + std::string(firstIndentation, ' ');
    size_t currentIndentation = firstIndentation;
    while (strIn.length() + currentIndentation > s) {
        size_t pos = strIn.substr(0, s - currentIndentation).find_last_of(" ");
        size_t c = 0;
        if (pos == std::string::npos) {
            c = 1;
            pos = strIn.substr(0, s - currentIndentation - c).find_last_of(",.-?:_\"!'/()=%;~^\\|");
            if (pos == std::string::npos) {
                pos = s - currentIndentation - c;
            }
        }
        strOut += strIn.substr(0, pos + c) + std::string(s - pos - c - currentIndentation, ' ') + " |\n";
        strOut += "| " + std::string(indentation, ' ');
        strIn = strIn.substr(pos + 1);
        currentIndentation = indentation;
    }
    strOut += strIn + std::string(s - strIn.length() - currentIndentation, ' ') + " |\n";
    return strOut;
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
