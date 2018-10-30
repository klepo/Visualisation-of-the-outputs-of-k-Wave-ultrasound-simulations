/**
 * @file        compresshelper.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        26 September 2016 (created) <br>
 *              30 October   2018 (updated)
 *
 * @brief       The header file with H5Helper::CompressHelper class declaration.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef COMPRESSHELPER_H
#define COMPRESSHELPER_H

#include <iostream>
#include <string>
#include <algorithm> // std::sort

//#define _USE_MATH_DEFINES // for C++
#ifndef M_PI
/// M_PI definition
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <vector>
#include <complex>
#include <omp.h>

namespace H5Helper
{
/// Float complex datatype
typedef std::complex<float> floatC;
/// Unsigned long long datatype
typedef unsigned long long hsize_t;
/// Long long datatype
typedef long long hssize_t;

/**
 * @brief The CompressHelper class represents wrapper for the ultrasound signals compression
 */
class CompressHelper
{
public:
    CompressHelper(hsize_t period, hsize_t mos, hsize_t harmonics, bool normalize = false);
    ~CompressHelper();

    static hsize_t findPeriod(const float *dataSrc, hsize_t length);
    float computeTimeStep(const float *cC, const float *lC, hsize_t stepLocal) const;

    const floatC *getBE() const;
    const floatC *getBE_1() const;
    hsize_t getOSize() const;
    hsize_t getBSize() const;
    hsize_t getPeriod() const;
    hsize_t getMos() const;
    hsize_t getHarmonics() const;
    hsize_t getStride() const;

private:
    /// Disable copy contructor
    CompressHelper(const CompressHelper &);
    /// Disable assignment operator
    /// \return CompressHelper
    CompressHelper &operator=(const CompressHelper &);

    static void xcorr(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1, hsize_t lengthSrc2);
    static void conv(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1, hsize_t lengthSrc2);
    static void findPeaks(const float *dataSrc, hsize_t *dataDst, hsize_t length, hsize_t &lengthDst);
    static void diff(const float *dataSrc, float *dataDst, hsize_t length);
    static void diff(const hsize_t *dataSrc, hsize_t *dataDst, hsize_t length);
    static float mean(const float *dataSrc, hsize_t length);
    static hsize_t mean(const hsize_t *dataSrc, hsize_t length);
    static hsize_t median(const hsize_t *dataSrc, hsize_t length);

    void generateFunctions(hsize_t bSize, hsize_t oSize, hsize_t period, hsize_t harmonics, float *b, floatC *e, floatC *bE, floatC *bE_1, bool normalize = false) const;
    void triangular(hsize_t oSize, float *w) const;
    void hann(hsize_t oSize, float *w) const;
    void generateE(hsize_t period, hsize_t ih, hsize_t h, hsize_t bSize, floatC *e) const;
    void generateBE(hsize_t ih, hsize_t bSize, hsize_t oSize, const float *b, const floatC *e, floatC *bE, floatC *bE_1, bool normalize = false) const;

    /// Overlap size
    hsize_t oSize = 0;
    /// Base size
    hsize_t bSize = 0;
    /// Period
    hsize_t period = 0;
    /// Multiple of overlap size
    hsize_t mos = 1;
    /// Number of harmonics
    hsize_t harmonics = 1;
    /// Coeficients stride
    hsize_t stride = 2;

    // Memory for helper functions data, 2D arrays for harmonics
    /// Window basis
    float *b = nullptr;
    /// Complex exponencial basis
    floatC *e = nullptr;
    /// Complex exponencial window basis
    floatC *bE = nullptr;
    /// Inverted complex exponencial window basis
    floatC *bE_1 = nullptr;
};
}

#endif // COMPRESSHELPER_H
