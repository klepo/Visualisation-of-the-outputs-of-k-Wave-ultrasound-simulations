/**
 * @file        compress.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The header file with Compress class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef COMPRESS_H
#define COMPRESS_H

#include "processing.h"
#include "compresshelper.h"

/**
 * @brief The Compress class represents wrapper for compression
 */
class Compress : public Processing
{
public:
    Compress(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings);
    virtual void execute();

private:
    void compressDataset(H5Helper::Dataset *srcDataset);
};

#endif // COMPRESS_H
