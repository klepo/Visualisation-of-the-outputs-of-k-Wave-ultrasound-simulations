/**
 * @file        processing.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with Processing class declaration.
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

#ifndef PROCESSING_H
#define PROCESSING_H

#include <dtsforpcs.h>
#include <omp.h>

/// Maximal number of frames, only for reshape.
#define MAX_NUMBER_OF_FRAMES 0 // TODO set 0 -> all steps (frames)

/**
 * @brief The Processing class represents wrapper for the HDF5 datasets processing
 */
class Processing
{
public:
    Processing(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings);
    virtual ~Processing();
    HDF5Helper::File *getOutputFile() const;
    DtsForPcs *getDtsForPcs() const;
    Settings *getSettings() const;

protected:
    /// Pure virtual execute function
    virtual void execute() = 0;
    virtual void copyAttributes(HDF5Helper::Dataset *srcDataset, HDF5Helper::Dataset *dstDataset) final;
    bool checkDatasetType(HDF5Helper::DatasetType datasetType, std::vector<HDF5Helper::DatasetType> types);

private:
    HDF5Helper::File *outputFile;
    DtsForPcs *dtsForPcs;
    Settings *settings;
};

#endif // PROCESSING_H
