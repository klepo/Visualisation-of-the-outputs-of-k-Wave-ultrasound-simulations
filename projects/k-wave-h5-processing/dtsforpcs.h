/**
 * @file        dtsforpcs.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              23 October   2018 (updated)
 *
 * @brief       The header file with DtsForPcs class declaration.
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

#ifndef DTSFORPCS_H
#define DTSFORPCS_H

#include <k-wave-h5-helper.h>

#include "filescontext.h"

/**
 * @brief The DtsForPcs class represents wrapper for datasets for processing
 */
class DtsForPcs
{
public:
    DtsForPcs(FilesContext *filesContext, Settings *settings);
    H5Helper::Vector4D getNDims() const;
    H5Helper::Dataset *getSensorMaskIndexDataset() const;
    H5Helper::Dataset *getSensorMaskCornersDataset() const;
    H5Helper::Dataset *getSourceInputDataset() const;
    hsize_t getSensorMaskSize() const;
    hsize_t getSensorMaskType() const;
    H5Helper::MapOfDatasets getDatasets(H5Helper::DatasetType datasetType = H5Helper::DatasetType::ALL) const;

private:
    DtsForPcs(const DtsForPcs &);
    DtsForPcs &operator=(const DtsForPcs &);
    H5Helper::Dataset *findAndGetDataset(const std::string name, H5Helper::File *simOutputFile, H5Helper::File *simInputFile);
    void findDatasetsForProcessing(H5Helper::Group *group, Settings *settings);
    bool isFiltered(std::string name, Settings *settings);

    H5Helper::Vector4D nDims;

    H5Helper::Dataset *sensorMaskIndexDataset = 0;
    H5Helper::Dataset *sensorMaskCornersDataset = 0;
    H5Helper::Dataset *sourceInputDataset = 0;
    hsize_t sensorMaskSize = 0;
    hsize_t sensorMaskType = 0;
    H5Helper::MapOfDatasets datasets;
};

#endif // DTSFORPCS_H
