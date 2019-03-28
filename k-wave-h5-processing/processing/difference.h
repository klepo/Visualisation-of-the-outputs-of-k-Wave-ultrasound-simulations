/**
 * @file        difference.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with Difference class declaration.
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

#ifndef DIFFERENCE_H
#define DIFFERENCE_H

#include "processing.h"

/**
 * @brief The Difference class represents wrapper for datasets subtraction
 */
class Difference : public Processing
{
public:
    Difference(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings);
    virtual void execute();

private:
    void subtractDatasets(H5Helper::Dataset *datasetOriginal, H5Helper::Dataset *datasetDecoded, bool log = true);
};

#endif // DIFFERENCE_H
