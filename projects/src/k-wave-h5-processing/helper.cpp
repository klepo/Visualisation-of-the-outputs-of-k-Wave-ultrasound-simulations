/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing helper stuff.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper
{
void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

void printDebugMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

void printErrorMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

unsigned long long round(float number)
{
    return static_cast<unsigned long long>(floor(double(number) + 0.5));
}

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

void diff(float *dataSrc, float *dataDst, const unsigned long long length)
{
    for (unsigned long long i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

void diff(unsigned long long *dataSrc, unsigned long long *dataDst, const unsigned long long length)
{
    for (unsigned long long i = 0; i < length - 1; i++) {
        dataDst[i] = dataSrc[i + 1] - dataSrc[i];
    }
}

float mean(float *dataSrc, const unsigned long long length)
{
    float sum = 0;
    for (unsigned long long i = 0; i < length; i++) {
        sum += dataSrc[i];
    }
    return sum / length;
}

unsigned long long mean(unsigned long long *dataSrc, const unsigned long long length)
{
    float sum = 0;
    for (unsigned long long i = 0; i < length; i++) {
        sum += dataSrc[i];
    }
    return round(sum / length);
}

unsigned long long median(unsigned long long *dataSrc, const unsigned long long length)
{
    std::vector<unsigned long long> dataSrcVector(dataSrc, dataSrc + length);
    std::sort(dataSrcVector.begin(), dataSrcVector.end());
    return dataSrcVector[size_t(length / 2)];
}

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

void triangular(unsigned long long oSize, float *b)
{
    for (unsigned long long x = 0; x < oSize; x++) {
        b[x] = float(x) / oSize;
    }
    for (unsigned long long x = oSize; x < 2 * oSize + 1; x++) {
        b[x] = 2.0f - float(x) / oSize;
    }
}

void hann(unsigned long long oSize, float *b)
{
    for (unsigned long long x = 0; x < 2 * oSize + 1; x++) {
        b[x] = float(pow(sin(M_PI * x / (2 * oSize)), 2));
    }
}
}
