/**
 * @file        helper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The header file with Helper namespace declaration.
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

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

/**
 * @namespace Helper
 * @brief Namespace for helper functions
 */
namespace Helper
{
void printDebugTitle(std::string str);
void printDebugTime(std::string ofWhat, double t0, double t1);
void printDebugMsg(std::string str);
void printMsg(std::string str);
void printDebugMsgStart(std::string str);
void printDebugMsg2S(std::string str);
void printDebugTwoColumnsTab(std::string first, std::string second, unsigned int width = 20);
void printDebugTwoColumnsTab(std::string first, int second, unsigned int width = 20);
void printDebugTwoColumnsTab(std::string first, unsigned long long second, unsigned int width = 20);
void printDebugTwoColumnsTab(std::string first, float second, unsigned int width = 20);
void printDebugTwoColumnsTab(std::string first, double second, unsigned int width = 20);
void printDebugTwoColumns2S(std::string first, std::string second, unsigned int width = 20);
void printDebugTwoColumns2S(std::string first, int second, unsigned int width = 20);
void printDebugTwoColumns2S(std::string first, unsigned long long second, unsigned int width = 20);
void printDebugTwoColumns2S(std::string first, float second, unsigned int width = 20);
void printDebugTwoColumns2S(std::string first, double second, unsigned int width = 20);
void printDebugString(std::string str);
void printErrorMsg(std::string str);

unsigned long long round(float number);

/// Enable debug messages flag
extern bool enableDebugMsgs;
}

#endif // HELPER_H
