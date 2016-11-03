/**
 * @file        colormap.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created)
 *
 * @brief       The header file with ColorMap class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef COLORMAP_H
#define COLORMAP_H

#include <QtGlobal>
#include <QtDebug>

class ColorMap
{
public:
    enum Type
    {
        AUTUMN = 0,
        BONE = 1,
        JET = 2,
        WINTER = 3,
        RAINBOW = 4,
        OCEAN = 5,
        SUMMER = 6,
        SPRING = 7,
        COOL = 8,
        HSV = 9,
        PINK = 10,
        HOT = 11
    };

    const static unsigned char data[][768];

    static void applyColorMap(int length, float minValue, float maxValue, float *srcData, unsigned char *dstData, ColorMap::Type type);

private:
    ColorMap();
};

#endif // COLORMAP_H
