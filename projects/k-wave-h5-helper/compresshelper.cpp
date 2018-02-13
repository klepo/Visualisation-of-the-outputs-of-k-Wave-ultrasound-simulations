/**
 * @file        compresshelper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing CompressHelper class definition.
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

#include "compresshelper.h"

namespace H5Helper {

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

CompressHelper::~CompressHelper()
{
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;
}

/**
 * @brief Cross correlation
 * @param[in] dataSrc1 Source data 1
 * @param[in] dataSrc2 Source data 2
 * @param[out] dataDst Destination
 * @param[in] lengthSrc1 Source length 1
 * @param[in] lengthSrc2 Source length 2
 */
void CompressHelper::xcorr(float *dataSrc1, float *dataSrc2, float *dataDst, const hsize_t lengthSrc1, const hsize_t lengthSrc2)
{
    hsize_t i, j;
    long long i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1 = static_cast<long long>(i);
        tmp = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < static_cast<long long>(lengthSrc1))
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
void CompressHelper::conv(float *dataSrc1, float *dataSrc2, float *dataDst, const hsize_t lengthSrc1, const hsize_t lengthSrc2)
{
    hsize_t i, j;
    long long i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1 = static_cast<long long>(i);
        tmp = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < static_cast<long long>(lengthSrc1))
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
void CompressHelper::findPeaks(float *dataSrc, hsize_t *dataDst, const hsize_t lengthSrc, hsize_t &lengthDst)
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
void CompressHelper::diff(float *dataSrc, float *dataDst, const hsize_t length)
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
void CompressHelper::diff(hsize_t *dataSrc, hsize_t *dataDst, const hsize_t length)
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
float CompressHelper::mean(float *dataSrc, const hsize_t length)
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
hsize_t CompressHelper::mean(hsize_t *dataSrc, const hsize_t length)
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
hsize_t CompressHelper::median(hsize_t *dataSrc, const hsize_t length)
{
    std::vector<hsize_t> dataSrcVector(dataSrc, dataSrc + length);
    std::sort(dataSrcVector.begin(), dataSrcVector.end());
    return dataSrcVector[size_t(length / 2)];
}

/**
 * @brief Computes period form data
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Period
 */
hsize_t CompressHelper::findPeriod(float *dataSrc, const hsize_t length)
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

    delete[] dataTmp;
    delete[] peaksTmp;
    delete[] peaks;

    return period;
}

float CompressHelper::computeTimeStep(float *cC, float *lC, hsize_t stepLocal)
{
    float stepValue = 0;
    for (hsize_t h = 0; h < harmonics; h++) {
        hsize_t sH = h * bSize + stepLocal;
        hsize_t h2 = h * 2;
        floatC lCC = conj(floatC(cC[h2], cC[h2 + 1]));
        floatC cCC = conj(floatC(lC[h2], lC[h2 + 1]));
        stepValue += real(cCC * getBE()[sH]) + real(lCC * getBE_1()[sH]);
    }
    return stepValue;
}

/**
 * @brief Generates triangular window
 * @param[in] oSize Overlap size
 * @param[out] w Window
 */
void CompressHelper::triangular(hsize_t oSize, float *w)
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
void CompressHelper::hann(hsize_t oSize, float *w)
{
    for (hsize_t x = 0; x < 2 * oSize + 1; x++) {
        w[x] = float(pow(sin(M_PI * x / (2 * oSize)), 2));
    }
}

void CompressHelper::generateE(hsize_t period, hsize_t ih, hsize_t h, hsize_t bSize, floatC *e)
{
    floatC i(0, -1);
    //#pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        hssize_t hx = hssize_t(ih) * hssize_t(bSize) + x;
        e[hx] = std::exp(i * (2.0f * float(M_PI) / (float(period) / (h))) * float(x));
    }
}

void CompressHelper::generateBE(hsize_t ih, hsize_t bSize, hsize_t oSize, float *b, floatC *e, floatC *bE, floatC *bE_1, bool normalize)
{
    //#pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        hssize_t hx = hssize_t(ih) * hssize_t(bSize) + x;
        bE[hx] = b[x] * e[hx];
        bE_1[hx] = b[(x + hssize_t(oSize)) % (hssize_t(bSize) - 1)] * e[hssize_t(ih) * hssize_t(bSize) + ((x + hssize_t(oSize)) % (hssize_t(bSize) - 1))];
        if (normalize) {
                bE[hx] *= (2.0f / float(oSize));
                bE_1[hx] *= (2.0f / float(oSize));
        }
    }
}

hsize_t CompressHelper::getStride() const
{
    return stride;
}

hsize_t CompressHelper::getMos() const
{
    return mos;
}

hsize_t CompressHelper::getPeriod() const
{
    return period;
}

hsize_t CompressHelper::getHarmonics() const
{
    return harmonics;
}

hsize_t CompressHelper::getBSize() const
{
    return bSize;
}

hsize_t CompressHelper::getOSize() const
{
    return oSize;
}

floatC *CompressHelper::getBE_1() const
{
    return bE_1;
}

floatC *CompressHelper::getBE() const
{
    return bE;
}



void CompressHelper::generateFunctions(hsize_t bSize, hsize_t oSize, hsize_t period, hsize_t harmonics, float *b, floatC *e, floatC *bE, floatC *bE_1, bool normalize)
{
    // Generate basis function (window)
    triangular(oSize, b);  // Triangular window
    //hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
        generateE(period, hsize_t(h), hsize_t(h + 1), bSize, e);
        generateBE(hsize_t(h), bSize, oSize, b, e, bE, bE_1, normalize);
    }
}
}
