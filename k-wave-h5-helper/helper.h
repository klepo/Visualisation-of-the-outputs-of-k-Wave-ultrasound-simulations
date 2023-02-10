/**
 * @file        helper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The header file with Helper namespace declaration.
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

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <stack>

/**
 * @namespace Helper
 * @brief Namespace for helper functions
 */
namespace Helper
{
void printDebugTitle(std::string str);
void printDebugTime(std::string ofWhat, double t0, double t1);
void printDebugMsg(std::string str);
void printDebugMsgStart(std::string str);
void printDebugMsgEnd(std::string str);
void printMsgEnd(std::string str);
void printDebugTwoColumnsS(std::string first, std::string second, size_t indentation = 0, size_t firstWidth = 20,
                           size_t secondWidth = 20, bool rightAlign = false);
void printDebugTwoColumnsS(std::string first, int second, size_t indentation = 0, size_t firstWidth = 20,
                           size_t secondWidth = 20, bool rightAlign = false);
void printDebugTwoColumnsS(std::string first, unsigned long long second, size_t indentation = 0, size_t firstWidth = 20,
                           size_t secondWidth = 20, bool rightAlign = false);
void printDebugTwoColumnsS(std::string first, float second, size_t indentation = 0, size_t firstWidth = 20,
                           size_t secondWidth = 20, bool rightAlign = false);
void printDebugTwoColumnsS(std::string first, double second, size_t indentation = 0, size_t firstWidth = 20,
                           size_t secondWidth = 20, bool rightAlign = false);
void printMsg(std::string str);
void printUnformattedMsg(std::string str);
void printErrorMsg(std::string str);
void printDebugLine();
void printLine();
void printLineErr();
std::string wrapMsg(std::string str, size_t indentation = 2, size_t firstIndentation = 0, bool rightAlign = false);
std::string floatToString(float number, size_t precision = 2);

void setDebugFlagAndStoreLast(bool flag);
void recoverLastDebugFlag();

float roundf(float number, int n = 0);
unsigned long long round(float number);
} // namespace Helper

#endif // HELPER_H
