/**
 * @file        colormap.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The header file with ColorMap class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef COLORMAP_H
#define COLORMAP_H

#include <QtGlobal>

/**
 * @brief The ColorMap class represents wrapper for color maps
 */
class ColorMap
{
public:
    /// Colormap types
    typedef enum Type
    {
        AUTUMN  = 0,
        BONE    = 1,
        JET     = 2,
        WINTER  = 3,
        RAINBOW = 4,
        OCEAN   = 5,
        SUMMER  = 6,
        SPRING  = 7,
        COOL    = 8,
        HSV     = 9,
        PINK    = 10,
        HOT     = 11
    } Type;

    /// Colormap data
    const static unsigned char data[][768];

    static void applyColorMap(int length, float minValue, float maxValue, const float *srcData, unsigned char *dstData,
                              ColorMap::Type type);

private:
    /// Disable instantiation
    ColorMap();
};

#endif // COLORMAP_H
