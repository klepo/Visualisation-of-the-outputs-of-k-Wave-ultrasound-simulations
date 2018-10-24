/**
 * @file        decompress.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              23 October   2018 (updated)
 *
 * @brief       The header file with Decompress class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */


#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "processing.h"
#include "compresshelper.h"

/**
 * @brief The Decompress class represents wrapper for decompression
 */
class Decompress : public Processing
{
public:
    Decompress(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings);
    virtual void execute();

private:
    void decompressDataset(H5Helper::Dataset *srcDataset, bool log = true);

};

#endif // DECOMPRESS_H
