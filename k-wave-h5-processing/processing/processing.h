/**
 * @file        processing.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with Processing class declaration.
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

#ifndef PROCESSING_H
#define PROCESSING_H

#include <omp.h>
#include <immintrin.h>

#include "dtsforpcs.h"

/// Maximal number of frames, only for reshape.
#define MAX_NUMBER_OF_FRAMES 0 // TODO set 0 -> all steps (frames)

/**
 * @brief The Processing class represents wrapper for the HDF5 datasets processing
 */
class Processing
{
public:
    Processing(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings);
    virtual ~Processing();
    H5Helper::File *getOutputFile() const;
    DtsForPcs *getDtsForPcs() const;
    const Settings *getSettings() const;
    double getTotalProcessingTime();
    hsize_t getTotalProcessingSize();

protected:
    /// Pure virtual execute function
    virtual void execute() = 0;
    virtual void copyAttributes(const H5Helper::Dataset *srcDataset, H5Helper::Dataset *dstDataset) final;
    bool checkDatasetType(H5Helper::DatasetType datasetType, std::vector<H5Helper::DatasetType> types);
    void addTime(double t);
    void addSize(hsize_t s);

private:
    /// Disable copy contructor
    Processing(const Processing &);
    /// Disable assignment operator
    /// \return Processing
    Processing &operator=(const Processing &);

    /// Output file
    H5Helper::File *outputFile;
    /// Datasets for processing
    DtsForPcs *dtsForPcs;
    /// Settings
    const Settings *settings;
    /// Time
    double time = 0;
    /// Size
    hsize_t size = 0;
};

#endif // PROCESSING_H
