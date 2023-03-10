/**
 * @file        compresshelper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        08 September 2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing H5Helper::CompressHelper class definition.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "compresshelper.h"

namespace H5Helper
{

const float CompressHelper::complexSize40bit = 1.25f;

/**
 * @brief Creates CompressHelper object with period, mos, harmonics and normalize flag
 * @param[in] period Period
 * @param[in] mos Multiple of overlap size
 * @param[in] harmonics Number of harmonics
 * @param[in] normalize Normalizes basis functions for compression (optional)
 * @param[in] shift Shifts phases of complex exponential basis function (velocity time shift) (optional)
 */
CompressHelper::CompressHelper(float period, hsize_t mos, hsize_t harmonics, bool normalize, bool shift,
                               float complexSize, int32_t kMaxExp)
{
    this->period      = period;
    this->mos         = mos;
    this->harmonics   = harmonics;
    this->complexSize = complexSize;
    this->kMaxExp     = kMaxExp;

    oSize = hsize_t(period * mos);
    bSize = oSize * 2 + 1;
    // TODO
    stride = float(this->harmonics) * this->complexSize;
    b      = new float[bSize]();
    e      = new floatC[this->harmonics * bSize]();
    bE     = new floatC[this->harmonics * bSize]();
    bE_1   = new floatC[this->harmonics * bSize]();

    generateFunctions(bSize, oSize, this->period, this->harmonics, b, e, bE, bE_1, normalize, shift);
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
 * @brief Computes period from data
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Period
 */
float CompressHelper::findPeriod(const float *dataSrc, hsize_t length)
{
    float *dataTmp     = new float[length]();
    float *peaksTmp    = new float[length]();
    float *locsTmp     = new float[length]();
    hsize_t peaksCount = 0;
    float period       = 0;
    float *newLocsTmp  = nullptr;
    float *newLocs     = nullptr;
    float *locs        = nullptr;

    // xcorr(dataSrc, dataSrc, dataTmp, length, length);
    findPeaks(dataSrc, locsTmp, peaksTmp, length, peaksCount);

    if (peaksCount > 0) {
        newLocsTmp = new float[peaksCount]();
        newLocs    = new float[peaksCount]();

        // Find max peak
        float m = std::numeric_limits<float>::min();
        for (hsize_t i = 0; i < peaksCount; i++) {
            if (peaksTmp[i] > m) {
                m = peaksTmp[i];
            }
        }

        // Filter peaks under 0.5 * max
        hsize_t j = 0;
        for (hsize_t i = 0; i < peaksCount; i++) {
            if (peaksTmp[i] > 0.5f * m) {
                newLocsTmp[j] = locsTmp[i];
                j++;
            }
        }

        // TODO check minimal peaksCount, j
        locs = new float[peaksCount]();
        diff(newLocsTmp, locs, j);
        period = median(locs, j - 1);
    }

    if (dataTmp) {
        delete[] dataTmp;
        dataTmp = nullptr;
    }
    if (peaksTmp) {
        delete[] peaksTmp;
        peaksTmp = nullptr;
    }
    if (locsTmp) {
        delete[] locsTmp;
        locsTmp = nullptr;
    }
    if (newLocsTmp) {
        delete[] newLocsTmp;
        newLocsTmp = nullptr;
    }
    if (newLocs) {
        delete[] newLocs;
        newLocs = nullptr;
    }
    if (locs) {
        delete[] locs;
        locs = nullptr;
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
    hsize_t sH      = stepLocal;
    for (hsize_t h = 0; h < harmonics; h++) {
        floatC sC1;
        floatC sC2;
        if (complexSize == complexSize40bit) {
            convert40bToFloatC(&(reinterpret_cast<const uint8_t *>(cC))[h * 5], sC1, kMaxExp);
            convert40bToFloatC(&(reinterpret_cast<const uint8_t *>(lC))[h * 5], sC2, kMaxExp);
        } else {
            sC1 = reinterpret_cast<const floatC *>(cC)[h];
            sC2 = reinterpret_cast<const floatC *>(lC)[h];
        }
        stepValue += real(conj(sC1) * getBE()[sH]) + real(conj(sC2) * getBE_1()[sH]);
        sH += bSize;
    }
    return stepValue;
}

/**
 * @brief Convert 40-bit coded complex number to float complex number
 * @param[in] iValues 40-bit coded complex number
 * @param[out] cValue float complex number
 * @param[in] e Exponent constant, usually 138 for acoustic pressure and 114 for particle velocity
 */
void CompressHelper::convert40bToFloatC(const uint8_t *iValues, floatC &cValue, const int32_t e)
{
    // Get mantissas, signs and exponent
    uint32_t mR = (*reinterpret_cast<const uint8_t *>(&iValues[0]) & 0x20U) << 11
                  | *reinterpret_cast<const uint16_t *>(&iValues[1]);
    uint32_t mI = (*reinterpret_cast<const uint8_t *>(&iValues[0]) & 0x10U) << 12
                  | *reinterpret_cast<const uint16_t *>(&iValues[3]);
    uint32_t sR = *reinterpret_cast<const uint8_t *>(&iValues[0]) >> 7;
    uint32_t sI = (*reinterpret_cast<const uint8_t *>(&iValues[0]) & 0x40) >> 6;
    uint8_t eS  = (*reinterpret_cast<const uint8_t *>(&iValues[0]) & 0xF);
    // Add 6 bits, now we have 23 bit mantissas
    mR <<= 6;
    mI <<= 6;
    // Add e constant (138 or 114)
    int32_t eR = eS + e;
    int32_t eI = eS + e;
    // Zero mantissa means zero float number
    if (mR != 0) {
// Find index of most left one bit
#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
        int index = 0;
        index     = 31 ^ __builtin_clz(mR);
#elif defined _WIN32
        unsigned long index = 0;
        _BitScanReverse(&index, mR);
#else
        uint32_t index = 0;
        _BitScanReverse(&index, mR);
#endif
        // Shift left by the index
        mR <<= 23 - index;
        // Recompute final exponent by the index
        eR -= 22 - index;
    } else {
        eR = 0;
    }
    if (mI != 0) {
#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
        int index = 0;
        index     = 31 ^ __builtin_clz(mI);
#elif defined _WIN32
        unsigned long index = 0;
        _BitScanReverse(&index, mI);
#else
        uint32_t index = 0;
        _BitScanReverse(&index, mI);
#endif
        mI <<= 23 - index;
        eI -= 22 - index;
    } else {
        eI = 0;
    }
    uint32_t ccR = (sR << 31) | (eR << 23) | (mR & 0x007fffff);
    uint32_t ccI = (sI << 31) | (eI << 23) | (mI & 0x007fffff);
    cValue       = floatC(*reinterpret_cast<float *>(&ccR), *reinterpret_cast<float *>(&ccI));
}

/**
 * @brief Convert float complex number to 40-bit coded complex number
 * @param[in] cValue float complex number
 * @param[out] iValues 40-bit coded complex number
 * @param[in] e Exponent constant, usually 138 for acoustic pressure and 114 for particle velocity
 */
void CompressHelper::convertFloatCTo40b(const floatC cValue, uint8_t *iValues, const int32_t e)
{
    // Get real and imaginary part
    float cR    = cValue.real();
    float cI    = cValue.imag();
    uint32_t mR = *reinterpret_cast<uint32_t *>(&cR);
    uint32_t mI = *reinterpret_cast<uint32_t *>(&cI);
    // Get 1-bit signs
    uint8_t sR = mR >> 31;
    uint8_t sI = mI >> 31;
    // e = 138 p, max exponent is 2^26 (15 + 138 = 153, 153 - 127 = 26)
    // p max value is pow(2, 26 - 16) * 0x1FFFF = 134216704
    // p min value is pow(2, 26 - 16 - 15) * 0x1 = 0.0312500000
    // e = 114 u, max exponent is 2^2  (15 + 114 = 129, 129 - 127 = 2)
    // u max value is pow(2, 2 - 16) * 0x1FFFF = 7.99993896484375
    // u max value is pow(2, 2 - 16 - 15) * 0x1 = 0.000000001862645149230957031250
    //  Get 8-bit exponents, subtracts e, (exponent will have 4 bits, values from 0 to 15)
    int32_t eRS = ((mR & 0x7f800000) >> 23) - e;
    int32_t eIS = ((mI & 0x7f800000) >> 23) - e;
    int32_t eS  = eRS;
    // Get 23-bit mantissas
    mR = (mR & 0x007fffff);
    mI = (mI & 0x007fffff);
    // Right shifts of real and imaginary part, we drop 6 bits, the mantissa will have 16 + 1 bits
    uint8_t rSR = 6;
    uint8_t rSI = 6;
    // Find the higher exponent, smaller part will be shifted to right
    if (eRS > eIS) {
        // Add right shift of imaginary part
        rSI += (eRS - eIS);
        eS = eRS;
    } else if (eIS > eRS) {
        // Add right shift of real part
        rSR += (eIS - eRS);
        eS = eIS;
    }
    // Crop small values
    if (eS < 0) {
        // Shift back to left
        rSR += -(eS);
        rSI += -(eS);
        // Exponent will be 0
        eS = 0;
    }
    // Shift overflow
    if (rSR > 23) {
        rSR = 23;
    }
    if (rSI > 23) {
        rSI = 23;
    }
    // Shift right
    mR >>= rSR;
    mI >>= rSI;
    // Rounding
    if (mR > 0) {
        // Check possible overflow
        if (mR != (0x7FFFFFU >> rSR)) {
            mR++;
        }
    }
    if (mI > 0) {
        if (mI != (0x7FFFFFU >> rSI)) {
            mI++;
        }
    }
    // Set 1 left flag bit
    mR |= 1UL << (23 - rSR);
    // Align to 17 bits
    mR >>= 1;
    mI |= 1UL << (23 - rSI);
    mI >>= 1;

    // Exponent overflow, set maximum values
    if (eS > 0xF) {
        mR = 0xFFFF;
        mI = 0xFFFF;
        eS = 0xF;
    }
    // bits: | 1         | 1              | 17            | 17                 | 4                |
    //       | real sign | imaginary sign | real mantissa | imaginary mantissa | shifted exponent |
    // Mantissa is composed from: 0-16 zero bits, 1 flag bit, 0-16 data (mantissa or fraction) bits
    // Number of zero bits means exponent shift from the stored exponent eS
    iValues[0] = (sR << 7) | (sI << 6) | ((mR & 0x10000) >> 11) | ((mI & 0x10000) >> 12) | (eS & 0xF);
    *reinterpret_cast<uint16_t *>(&iValues[1]) = mR;
    *reinterpret_cast<uint16_t *>(&iValues[3]) = mI;
}

/**
 * @brief Returns complex exponential basis
 * @return Complex exponential basis
 */
const floatC *CompressHelper::getE() const
{
    return e;
}

/**
 * @brief Returns complex exponential window basis
 * @return Complex exponential window basis
 */
const floatC *CompressHelper::getBE() const
{
    return bE;
}

/**
 * @brief Returns inverted complex exponential window basis
 * @return Inverted complex exponential window basis
 */
const floatC *CompressHelper::getBE_1() const
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
float CompressHelper::getPeriod() const
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
float CompressHelper::getStride() const
{
    return stride;
}

/**
 * @brief Returns coefficients complex number size (number of floats) for one step
 * @return Coefficients complex number size (number of floats) for one step
 */
float CompressHelper::getComplexSize() const
{
    return complexSize;
}

/**
 * @brief Cross correlation
 * @param[in] dataSrc1 Source data 1
 * @param[in] dataSrc2 Source data 2
 * @param[out] dataDst Destination
 * @param[in] lengthSrc1 Source length 1
 * @param[in] lengthSrc2 Source length 2
 */
void CompressHelper::xcorr(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1,
                           hsize_t lengthSrc2)
{
    hsize_t i, j;
    hssize_t i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1         = hssize_t(i);
        tmp        = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < hssize_t(lengthSrc1)) {
                tmp = tmp + (dataSrc1[i1] * dataSrc2[lengthSrc2 - 1 - j]);
            }
            i1         = i1 - 1;
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
void CompressHelper::conv(const float *dataSrc1, const float *dataSrc2, float *dataDst, hsize_t lengthSrc1,
                          hsize_t lengthSrc2)
{
    hsize_t i, j;
    hssize_t i1;
    float tmp;

    for (i = 0; i < lengthSrc1 + lengthSrc2 - 1; i++) {
        dataDst[i] = 0;
        i1         = hssize_t(i);
        tmp        = 0.0;
        for (j = 0; j < lengthSrc2; j++) {
            if (i1 >= 0 && i1 < hssize_t(lengthSrc1)) {
                tmp = tmp + (dataSrc1[i1] * dataSrc2[j]);
            }
            i1         = i1 - 1;
            dataDst[i] = tmp;
        }
    }
}

/**
 * @brief Finds peaks in source signal
 * @param[in] dataSrc Source data
 * @param[out] locsDst Destination for locations
 * @param[out] peaksDst Destination for peaks
 * @param[in] lengthSrc Source length
 * @param[out] lengthDst Destination length
 */
void CompressHelper::findPeaks(const float *dataSrc, float *locsDst, float *peaksDst, hsize_t lengthSrc,
                               hsize_t &lengthDst)
{
    for (hsize_t i = 0; i < lengthSrc; i++) {
        locsDst[i]  = 0.0f;
        peaksDst[i] = 0.0f;
    }

    lengthDst = 0;

    for (hsize_t i = 0; i < lengthSrc; i++) {
        // Peaks between
        if (i > 0 && (i < lengthSrc - 1) && lengthSrc > 2 && dataSrc[i] > dataSrc[i - 1]
            && dataSrc[i] >= dataSrc[i + 1]) {
            float d1            = dataSrc[i] - dataSrc[i - 1];
            float d2            = dataSrc[i] - dataSrc[i + 1];
            locsDst[lengthDst]  = i + d1 / (d1 + d2) - 0.5f;
            peaksDst[lengthDst] = dataSrc[i];
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
    float sum = 0.0f;
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
    float sum = 0.0f;
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
float CompressHelper::median(const float *dataSrc, hsize_t length)
{
    std::vector<float> dataSrcVector(dataSrc, dataSrc + length);
    std::sort(dataSrcVector.begin(), dataSrcVector.end());
    return dataSrcVector[size_t(length / 2)];
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
 * @param[out] e Exponential basis
 * @param[out] bE Complex exponential window basis
 * @param[out] bE_1 Inverted complex exponential window basis
 * @param[in] normalize Normalization flag (optional)
 */
void CompressHelper::generateFunctions(hsize_t bSize, hsize_t oSize, float period, hsize_t harmonics, float *b,
                                       floatC *e, floatC *bE, floatC *bE_1, bool normalize, bool shift) const
{
    // Generate basis function (window)
    triangular(oSize, b); // Triangular window
    // hann(oSize, b);        // Hann window

    // Generate complex exponential window basis functions
    for (hsize_t ih = 0; ih < harmonics; ih++) {
        generateE(period, ih, ih + 1, bSize, e, shift);
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
        w[x] = float(pow(sin(float(M_PI) * x / (2.0f * oSize)), 2));
    }
}

/**
 * @brief Generates complex exponential basis
 * @param[in] period Period
 * @param[in] ih Harmonics index
 * @param[in] h Harmonic multiple
 * @param[in] bSize Base size
 * @param[out] e Exponential basis
 */
void CompressHelper::generateE(float period, hsize_t ih, hsize_t h, hsize_t bSize, floatC *e, bool shift) const
{
    floatC i(0.0f, -1.0f);
    for (hsize_t x = 0; x < bSize; x++) {
        hsize_t hx = ih * bSize + x;
        e[hx]      = std::exp(i * (2.0f * float(M_PI) / (period / float(h))) * float(x));
        if (shift) {
            e[hx] *= std::exp(-i * float(M_PI) / (period / float(h)));
        }
    }
}

/**
 * @brief Generates complex exponential window basis
 * @param[in] ih Harmonics index
 * @param[in] bSize Base size
 * @param[in] oSize Overlap size
 * @param[in] b Window basis
 * @param[in] e Exponential basis
 * @param[out] bE Complex exponential window basis
 * @param[out] bE_1 Inverted complex exponential window basis
 * @param[in] normalize Normalization flag (optional)
 */
void CompressHelper::generateBE(hsize_t ih, hsize_t bSize, hsize_t oSize, const float *b, const floatC *e, floatC *bE,
                                floatC *bE_1, bool normalize) const
{
    for (hsize_t x = 0; x < hsize_t(bSize); x++) {
        hsize_t hx = ih * bSize + x;
        bE[hx]     = b[x] * e[hx];
        bE_1[hx]   = b[(x + oSize) % (bSize - 1)] * e[ih * bSize + ((x + oSize) % (bSize - 1))];
        if (normalize) {
            bE[hx] *= (2.0f / float(oSize));
            bE_1[hx] *= (2.0f / float(oSize));
        }
    }
}
} // namespace H5Helper
