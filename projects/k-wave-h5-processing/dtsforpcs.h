/**
 * @file        dtsforpcs.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with DtsForPcs class declaration.
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

#ifndef DTSFORPCS_H
#define DTSFORPCS_H

#include <hdf5helper.h>
#include <filescontext.h>

/**
 * @brief The DtsForPcs class represents wrapper for datasets for processing
 */
class DtsForPcs
{
public:
    DtsForPcs(FilesContext *filesContext, Settings *settings);
    HDF5Helper::Vector4D getNDims() const;
    HDF5Helper::Dataset *getSensorMaskIndexDataset() const;
    HDF5Helper::Dataset *getSensorMaskCornersDataset() const;
    HDF5Helper::Dataset *getSourceInputDataset() const;
    hsize_t getSensorMaskSize() const;
    hsize_t getSensorMaskType() const;
    HDF5Helper::MapOfDatasets getDatasets(HDF5Helper::DatasetType datasetType = HDF5Helper::DatasetType::ALL) const;

private:
    HDF5Helper::Dataset *findAndGetDataset(const std::string name, HDF5Helper::File *simOutputFile, HDF5Helper::File *simInputFile);
    void findDatasetsForProcessing(HDF5Helper::Group *group, Settings *settings);
    bool isFiltered(std::string name, Settings *settings);

    HDF5Helper::Vector4D nDims;

    HDF5Helper::Dataset *sensorMaskIndexDataset = 0;
    HDF5Helper::Dataset *sensorMaskCornersDataset = 0;
    HDF5Helper::Dataset *sourceInputDataset = 0;
    hsize_t sensorMaskSize = 0;
    hsize_t sensorMaskType = 0;
    HDF5Helper::MapOfDatasets datasets;

    // Disable copy
    DtsForPcs(const DtsForPcs &);
};

#endif // DTSFORPCS_H
