/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Helper class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper {

/**
 * @brief Prints debug title
 * @param[in] msg Debugging message
 */
void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

/**
 * @brief Prints debug message
 * @param[in] msg Debugging message
 */
void printDebugMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

/**
 * @brief Prints error message
 * @param[in] msg Debugging message
 */
void printErrorMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

/**
 * @brief Round
 * @param[in] number Float numebr
 * @return Rounded number
 */
unsigned long long round(float number)
{
    return static_cast<unsigned long long>(floor(double(number) + 0.5));
}

/**
 * @brief Cross correlation
 * @param[in] dataSrc1 Source data 1
 * @param[in] dataSrc2 Source data 2
 * @param[out] dataDst Destination
 * @param[in] lengthSrc1 Source length 1
 * @param[in] lengthSrc2 Source length 2
 */
void xcorr(float *dataSrc1, float *dataSrc2, float *dataDst, const unsigned long long lengthSrc1, const unsigned long long lengthSrc2)
{
    unsigned long long i, j;
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
void conv(float *dataSrc1, float *dataSrc2, float *dataDst, const unsigned long long lengthSrc1, const unsigned long long lengthSrc2)
{
    unsigned long long i, j;
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
void findPeaks(float *dataSrc, unsigned long long *dataDst, const unsigned long long lengthSrc, unsigned long long &lengthDst)
{
    for (unsigned long long i = 0; i < lengthSrc; i++) {
        dataDst[i] = 0;
    }

    lengthDst = 0;

    for (unsigned long long i = 0; i < lengthSrc; i++) {
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
void diff(float *dataSrc, float *dataDst, const unsigned long long length)
{
    for (unsigned long long i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

/**
 * @brief Differences
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination
 * @param[in] length Source length
 */
void diff(unsigned long long *dataSrc, unsigned long long *dataDst, const unsigned long long length)
{
    for (unsigned long long i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

/**
 * @brief Mean
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Mean
 */
float mean(float *dataSrc, const unsigned long long length)
{
    float sum = 0;
    for (unsigned long long i = 0; i < length; i++) {
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
unsigned long long mean(unsigned long long *dataSrc, const unsigned long long length)
{
    float sum = 0;
    for (unsigned long long i = 0; i < length; i++) {
        sum += dataSrc[i];
    }
    return round(sum / length);
}

/**
 * @brief Median
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Median
 */
unsigned long long median(unsigned long long *dataSrc, const unsigned long long length)
{
    std::vector<unsigned long long> dataSrcVector(dataSrc, dataSrc + length);
    std::sort(dataSrcVector.begin(), dataSrcVector.end());
    return dataSrcVector[size_t(length / 2)];
}

/**
 * @brief Computes period form data
 * @param[in] dataSrc Source data
 * @param[in] length Source length
 * @return Period
 */
unsigned long long getPeriod(float *dataSrc, const unsigned long long length)
{
    float *dataTmp = new float[length];
    unsigned long long *peaksTmp = new unsigned long long[length];
    unsigned long long peaksCount;
    unsigned long long period;

    //xcorr(dataSrc, dataSrc, dataTmp, length, length);
    findPeaks(dataSrc, peaksTmp, length, peaksCount);
    unsigned long long *peaks = new unsigned long long[peaksCount - 1];
    diff(peaksTmp, peaks, peaksCount);
    period = median(peaks, peaksCount - 1);

    delete[] dataTmp;
    delete[] peaksTmp;
    delete[] peaks;

    return period;
}

/**
 * @brief Generates triangular window
 * @param[in] oSize Overlap size
 * @param[out] w Window
 */
void triangular(unsigned long long oSize, float *w)
{
    for (unsigned long long x = 0; x < oSize; x++) {
        w[x] = float(x) / oSize;
    }
    for (unsigned long long x = oSize; x < 2 * oSize + 1; x++) {
        w[x] = 2.0f - float(x) / oSize;
    }
}

/**
 * @brief Generates Hann window
 * @param[in] oSize Overlap size
 * @param[out] w Window
 */
void hann(unsigned long long oSize, float *w)
{
    for (unsigned long long x = 0; x < 2 * oSize + 1; x++) {
        w[x] = float(pow(sin(M_PI * x / (2 * oSize)), 2));
    }
}
}
