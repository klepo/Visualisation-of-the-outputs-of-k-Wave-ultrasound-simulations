/**
 * @file        colormap.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with ColorMap class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef COLORMAP_H
#define COLORMAP_H

#include <QtGlobal>
#include <QtDebug>

/**
 * @brief The ColorMap class represents wrapper for color maps
 */
class ColorMap
{
public:
    /// Colormap types
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

    /// Colormap data
    const static unsigned char data[][768];

    static void applyColorMap(int length, float minValue, float maxValue, float *srcData, unsigned char *dstData, ColorMap::Type type);

private:
    ColorMap();
};

#endif // COLORMAP_H
