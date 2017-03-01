/**
 * @file        helper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with Helper namespace declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string>
#include <algorithm>    // std::sort
#include <vector>       // std::vector

//#define _USE_MATH_DEFINES // for C++
#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif
#include <cmath>
#include <complex>

namespace Helper
{
void printDebugTitle(std::string msg);
void printDebugMsg(std::string msg);
void printErrorMsg(std::string msg);

unsigned long long round(float number);
void xcorr(float *dataSrc1, float *dataSrc2, float *dataDst, const unsigned long long lengthSrc1, const unsigned long long lengthSrc2);
void conv(float *dataSrc1, float *dataSrc2, float *dataDst, const unsigned long long lengthSrc1, const unsigned long long lengthSrc2);
void findPeaks(float *dataSrc, unsigned long long *dataDst, const unsigned long long length, unsigned long long &lengthDst);
void diff(float *dataSrc, float *dataDst, const unsigned long long length);
void diff(unsigned long long *dataSrc, unsigned long long *dataDst, const unsigned long long length);
float mean(float *dataSrc, const unsigned long long length);
unsigned long long mean(unsigned long long *dataSrc, const unsigned long long length);
unsigned long long median(unsigned long long *dataSrc, const unsigned long long length);
unsigned long long getPeriod(float *dataSrc, const unsigned long long length);

void triangular(unsigned long long oSize, float *b);   // Triangular window
void hann(unsigned long long oSize, float *b);         // Hann window

typedef std::complex<float> floatC;
}

#endif // HELPER_H
