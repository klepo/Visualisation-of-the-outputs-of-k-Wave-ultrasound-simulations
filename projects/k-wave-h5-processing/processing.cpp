/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Processing class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "processing.h"

/**
 * @brief Creates Processing object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Processing::Processing(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
{
    this->outputFile = outputFile;
    this->dtsForPcs = dtsForPcs;
    this->settings = settings;
}

/**
 * @brief Destructor of Processing object
 *
 * Does nothing.
 */
Processing::~Processing()
{

}

/**
 * @brief Copies attributes
 * @param[in] srcDataset Source dataset
 * @param[in] dstDataset Destination dataset
 */
void Processing::copyAttributes(HDF5Helper::Dataset *srcDataset, HDF5Helper::Dataset *dstDataset)
{
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr, false);
        delete attr;
    }
}

/**
 * @brief Checks dataset type
 * @param[in] datasetType Type to check
 * @param[in] types Allowed types
 * @return True/False
 */
bool Processing::checkDatasetType(HDF5Helper::DatasetType datasetType, std::vector<HDF5Helper::DatasetType> types)
{
    for(std::vector<HDF5Helper::DatasetType>::iterator it = types.begin(); it != types.end(); ++it) {
        if (*it == datasetType)
            return true;
    }
    return false;
}

/**
 * @brief Returns settings
 * @return Settings
 */
Settings *Processing::getSettings() const
{
    return settings;
}

/**
 * @brief Returns datasets for processing
 * @return datasets for processing
 */
DtsForPcs *Processing::getDtsForPcs() const
{
    return dtsForPcs;
}

/**
 * @brief Returns output file
 * @return output file
 */
HDF5Helper::File *Processing::getOutputFile() const
{
    return outputFile;
}
