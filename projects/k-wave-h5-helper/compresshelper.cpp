/**
 * @file        compresshelper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The implementation file containing H5Helper::CompressHelper class definition.
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

#include "compresshelper.h"

namespace H5Helper {

/**
 * @brief Creates CompressHelper object with period, mos, harmonics and normalize flag
 * @param[in] period Period
 * @param[in] mos Multiple of overlap size
 * @param[in] harmonics Number of harmonics
 * @param[in] normalize Normalizes basis functions for compression (optional)
 */
CompressHelper::CompressHelper(hsize_t period, hsize_t mos, hsize_t harmonics, bool normalize)
{
    oSize = period * mos;
    bSize = oSize * 2 + 1;
    this->period = period;
    this->mos = mos;
    this->harmonics = harmonics;
    stride = harmonics * 2;
    b = new float[bSize]();
    e = new floatC[harmonics * bSize]();
    bE = new floatC[harmonics * bSize]();
    bE_1 = new floatC[harmonics * bSize]();

    generateFunctions(bSize, oSize, period, harmonics, b, e, bE, bE_1, normalize);
}

/**
 * @brief Destructor of CompressHelper object
 *
 * Deletes some memory for basis functions.
 */
CompressHelper::~CompressHelper()
{
    if (b) {
        delete[] b;
        b = nullptr;
    }
    if (e) {
        delete[] e;
        e = nullptr;
    }
    if (bE) {
        delete[] bE;
        bE = nullptr;
    }
    if (bE_1) {
        delete[] bE_1;
        bE_1 = nullptr;
    }
}

/**
 * @brief Computes period form data
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Period
 */
hsize_t CompressHelper::findPeriod(const float *dataSrc, hsize_t length)
{
    float *dataTmp = new float[length];
    hsize_t *peaksTmp = new hsize_t[length];
    hsize_t peaksCount;
    hsize_t period;

    //xcorr(dataSrc, dataSrc, dataTmp, length, length);
    findPeaks(dataSrc, peaksTmp, length, peaksCount);
    hsize_t *peaks = new hsize_t[peaksCount - 1];
    diff(peaksTmp, peaks, peaksCount);
    period = median(peaks, peaksCount - 1);

    if (dataTmp) {
        delete[] dataTmp;
        dataTmp = nullptr;
    }
    if (peaksTmp) {
        delete[] peaksTmp;
        peaksTmp = nullptr;
    }
    if (peaks) {
        delete[] peaks;
        peaks = nullptr;
    }
    return period;
}

/**
 * @brief Computes sample value for given step using coefficients (decompression)
 * @param[in] cC Current coefficients
 * @param[in] lC Last coefficients
 * @param[in] stepLocal Local step between coefficients
 * @return Step value
 */
float CompressHelper::computeTimeStep(const float *cC, const float *lC, hsize_t stepLocal) const
{
    float stepValue = 0;
    hsize_t sH = stepLocal;
    for (hsize_t h = 0; h < harmonics; h++) {
        stepValue += real(conj(reinterpret_cast<const floatC *>(cC)[h]) * getBE()[sH]) +
                real(conj(reinterpret_cast<const floatC *>(lC)[h]) * getBE_1()[sH]);
        sH += bSize;
    }
    return stepValue;
}

/**
 * @brief Returns complex exponencial window basis
 * @return Complex exponencial window basis
 */
floatC *CompressHelper::getBE() const
{
    return bE;
}

/**
 * @brief Returns inverted complex exponencial window basis
 * @return Inverted complex exponencial window basis
 */
floatC *CompressHelper::getBE_1() const
{
    return bE_1;
}

/**
 * @brief Returns overlap size
 * @return Overlap size
 */
hsize_t CompressHelper::getOSize() const
{
    return oSize;
}

/**
 * @brief Return base size
 * @return Base size
 */
hsize_t CompressHelper::getBSize() const
{
    return bSize;
}

/**
 * @brief Returns period
 * @return Period
 */
hsize_t CompressHelper::getPeriod() const
{
    return period;
}

/**
 * @brief Returns multiple of overlap size
 * @return Multiple of overlap size
 */
hsize_t CompressHelper::getMos() const
{
    return mos;
}

/**
 * @brief Return number of harmonics
 * @return Number of harmonics
 */
hsize_t CompressHelper::getHarmonics() const
{
    return harmonics;
}

/**
 * @brief Returns coefficients stride for one step (harmonics * 2;)
 * @return Coefficients stride for one step
 */
hsize_t CompressHelper::getStride() const
{
    return stride;
}

/**
 * @brief Cross correlation
 * @param[in] dataSrc1 Source data 1
 * @param[in] dataSrc2 Source data 2
 * @param[out] dataDst Destination
 * @param[in] lengthSrc1 Source length 1
 * @param[in] lengthSrc2 Source length 2
 */
void CompressHelper::xcorr(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1, hsize_t lengthSrc2)
{
    hsize_t i, j;
    hssize_t i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1 = hssize_t(i);
        tmp = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < hssize_t(lengthSrc1))
                tmp = tmp + (dataSrc1[i1] * dataSrc2[lengthSrc2 - 1 - j]);

            i1 = i1 - 1;
            dataDst[i] = tmp;
        }
    }
}

/**
 * @brief Convolution
 * @param[in] dataSrc1 Source data 1
 * @param[in] dataSrc2 Source data 2
 * @param[out] dataDst Destination
 * @param[in] lengthSrc1 Source length 1
 * @param[in] lengthSrc2 Source length 2
 */
void CompressHelper::conv(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1, hsize_t lengthSrc2)
{
    hsize_t i, j;
    hssize_t i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1 = hssize_t(i);
        tmp = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < hssize_t(lengthSrc1))
                tmp = tmp + (dataSrc1[i1] * dataSrc2[j]);

            i1 = i1 - 1;
            dataDst[i] = tmp;
        }
    }
}

/**
 * @brief Finds peaks in source signal
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination
 * @param[in] lengthSrc Source length
 * @param[out] lengthDst Destination length
 */
void CompressHelper::findPeaks(const float *dataSrc, hsize_t *dataDst, hsize_t lengthSrc, hsize_t &lengthDst)
{
    for (hsize_t i = 0; i < lengthSrc; i++) {
        dataDst[i] = 0;
    }

    lengthDst = 0;

    for (hsize_t i = 0; i < lengthSrc; i++) {
        // If the peak is only one
        /*if (length == 1) {
            dataDst[0] = 0;
            break;
        }*/

        // If first element is peak
        /*if (i == 0 && length > 1) {
            if (dataSrc[i + 1] < dataSrc[i]) {
                dataDst[j] = i;
                j++;
            }
            continue;
        }*/

        // If last element is peak
        /*if (i == length - 1 && length > 1) {
            if (dataSrc[i - 1] < dataSrc[i]) {
                dataDst[j] = i;
                j++;
            }
            break;
        }*/

        // Peaks between
        if (i > 0 && i < lengthSrc - 1 && lengthSrc > 2 && dataSrc[i] > dataSrc[i - 1] && dataSrc[i] >= dataSrc[i + 1]) {
            dataDst[lengthDst] = i;
            lengthDst++;
        }
    }
}

/**
 * @brief Differences
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination
 * @param[in] length Source length
 */
void CompressHelper::diff(const float *dataSrc, float *dataDst, hsize_t length)
{
    for (hsize_t i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

/**
 * @brief Differences
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination
 * @param[in] length Source length
 */
void CompressHelper::diff(const hsize_t *dataSrc, hsize_t *dataDst, hsize_t length)
{
    for (hsize_t i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

/**
 * @brief Mean
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Mean
 */
float CompressHelper::mean(const float *dataSrc, hsize_t length)
{
    float sum = 0;
    for (hsize_t i = 0; i < length; i++) {
        sum += dataSrc[i];
    }
    return sum / length;
}

/**
 * @brief Mean
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Mean
 */
hsize_t CompressHelper::mean(const hsize_t *dataSrc, hsize_t length)
{
    float sum = 0;
    for (hsize_t i = 0; i < length; i++) {
        sum += dataSrc[i];
    }
    return hsize_t(round(sum / length));
}

/**
 * @brief Median
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Median
 */
hsize_t CompressHelper::median(const hsize_t *dataSrc, hsize_t length)
{
    std::vector<hsize_t> dataSrcVector(dataSrc, dataSrc + length);
    std::sort(dataSrcVector.begin(), dataSrcVector.end());
    return dataSrcVector[size_t(length / 2)];
}

/**
 * @brief Generates basis function values
 * @param[in] bSize Base size
 * @param[in] oSize Overlap size
 * @param[in] period Period
 * @param[in] harmonics Harmonics
 * @param[out] b Window basis
 * @param[out] e Exponencial basis
 * @param[out] bE Complex exponencial window basis
 * @param[out] bE_1 Inverted complex exponencial window basis
 * @param[in] normalize Normalization flag (optional)
 */
void CompressHelper::generateFunctions(hsize_t bSize, hsize_t oSize, hsize_t period, hsize_t harmonics, float *b, floatC *e, floatC *bE, floatC *bE_1, bool normalize) const
{
    // Generate basis function (window)
    triangular(oSize, b);  // Triangular window
    //hann(oSize, b);        // Hann window

    // Generate complex exponential window basis functions
    for (hsize_t ih = 0; ih < harmonics; ih++) {
        generateE(period, ih, ih + 1, bSize, e);
        generateBE(ih, bSize, oSize, b, e, bE, bE_1, normalize);
    }
}

/**
 * @brief Generates triangular window
 * @param[in] oSize Overlap size
 * @param[out] w Window
 */
void CompressHelper::triangular(hsize_t oSize, float *w) const
{
    for (hsize_t x = 0; x < oSize; x++) {
        w[x] = float(x) / oSize;
    }
    for (hsize_t x = oSize; x < 2 * oSize + 1; x++) {
        w[x] = 2.0f - float(x) / oSize;
    }
}

/**
 * @brief Generates Hann window
 * @param[in] oSize Overlap size
 * @param[out] w Window
 */
void CompressHelper::hann(hsize_t oSize, float *w) const
{
    for (hsize_t x = 0; x < 2 * oSize + 1; x++) {
        w[x] = float(pow(sin(M_PI * x / (2 * oSize)), 2));
    }
}

/**
 * @brief Generates complex exponencial basis
 * @param[in] period Period
 * @param[in] ih Harmonics index
 * @param[in] h Harmonic multiple
 * @param[in] bSize Base size
 * @param[out] e Exponencial basis
 */
void CompressHelper::generateE(hsize_t period, hsize_t ih, hsize_t h, hsize_t bSize, floatC *e) const
{
    floatC i(0, -1);
    for (hsize_t x = 0; x < bSize; x++) {
        hsize_t hx = ih * bSize + x;
        e[hx] = std::exp(i * (2.0f * float(M_PI) / (float(period) / (h))) * float(x));
    }
}

/**
 * @brief Generates complex exponencial window basis
 * @param[in] ih Harmonics index
 * @param[in] bSize Base size
 * @param[in] oSize Overlap size
 * @param[in] b Window basis
 * @param[in] e Exponencial basis
 * @param[out] bE Complex exponencial window basis
 * @param[out] bE_1 Inverted complex exponencial window basis
 * @param[in] normalize Normalization flag (optional)
 */
void CompressHelper::generateBE(hsize_t ih, hsize_t bSize, hsize_t oSize, const float *b, const floatC *e, floatC *bE, floatC *bE_1, bool normalize) const
{
    for (hsize_t x = 0; x < hsize_t(bSize); x++) {
        hsize_t hx = ih * bSize + x;
        bE[hx] = b[x] * e[hx];
        bE_1[hx] = b[(x + oSize) % (bSize - 1)] * e[ih * bSize + ((x + oSize) % (bSize - 1))];
        if (normalize) {
            bE[hx] *= (2.0f / float(oSize));
            bE_1[hx] *= (2.0f / float(oSize));
        }
    }
}
}
