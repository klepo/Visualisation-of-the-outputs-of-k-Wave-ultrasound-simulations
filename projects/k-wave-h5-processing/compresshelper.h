/**
 * @file        compresshelper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        26  September 2016 (created) \n
 *              26 September 2017 (updated)
 *
 * @brief       The header file with CompressHelper class declaration.
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

#ifndef COMPRESSHELPER_H
#define COMPRESSHELPER_H

#include <iostream>
#include <string>
#include <algorithm>    // std::sort

//#define _USE_MATH_DEFINES // for C++
#ifndef M_PI
    /// M_PI definition
    #define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <complex>
#include <omp.h>

#include <hdf5helper.h>


class CompressHelper
{
public:
    static void xcorr(float *dataSrc1, float *dataSrc2, float *dataDst, const hsize_t lengthSrc1, const hsize_t lengthSrc2);
    static void conv(float *dataSrc1, float *dataSrc2, float *dataDst, const hsize_t lengthSrc1, const hsize_t lengthSrc2);
    static void findPeaks(float *dataSrc, hsize_t *dataDst, const hsize_t length, hsize_t &lengthDst);
    static void diff(float *dataSrc, float *dataDst, const hsize_t length);
    static void diff(hsize_t *dataSrc, hsize_t *dataDst, const hsize_t length);
    static float mean(float *dataSrc, const hsize_t length);
    static hsize_t mean(hsize_t *dataSrc, const hsize_t length);
    static hsize_t median(hsize_t *dataSrc, const hsize_t length);
    static hsize_t getPeriod(float *dataSrc, const hsize_t length);

protected:
    /// Float complex datatype
    typedef std::complex<float> floatC;

    void triangular(hsize_t oSize, float *w);   // Triangular window
    void hann(hsize_t oSize, float *w);         // Hann window
    void generateE(hsize_t period, hsize_t ih, hsize_t h, hsize_t bSize, floatC *e);
    void generateBE(hsize_t h, hsize_t bSize, hsize_t oSize, float *b, floatC *e, floatC *bE, floatC *bE_1, bool normalize = false);
    void generateFunctions(hsize_t bSize, hsize_t oSize, hsize_t period, hsize_t harmonics, float *b, floatC *e, floatC *bE, floatC *bE_1, HDF5Helper::VectorOfDatasets srcDatasets = HDF5Helper::VectorOfDatasets());
};

#endif // COMPRESSHELPER_H
