/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The implementation file containing Processing class definition.
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

#include "processing.h"

/**
 * @brief Creates Processing object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Processing::Processing(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
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
void Processing::copyAttributes(H5Helper::Dataset *srcDataset, H5Helper::Dataset *dstDataset)
{
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        H5Helper::Attribute *attribute = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attribute, false);
        delete attribute;
        attribute = nullptr;
    }
}

/**
 * @brief Checks dataset type
 * @param[in] datasetType Type to check
 * @param[in] types Allowed types
 * @return True/False
 */
bool Processing::checkDatasetType(H5Helper::DatasetType datasetType, std::vector<H5Helper::DatasetType> types)
{
    for(std::vector<H5Helper::DatasetType>::iterator it = types.begin(); it != types.end(); ++it) {
        if (*it == datasetType)
            return true;
    }
    return false;
}

void Processing::addTime(double t)
{
    time += t;
}

void Processing::addSize(hsize_t s)
{
    size += s;
}

/**
 * @brief Returns settings
 * @return Settings
 */
Settings *Processing::getSettings() const
{
    return settings;
}

double Processing::getTotalProcessingTime()
{
    return time;
}

hsize_t Processing::getTotalProcessingSize()
{
    return size;
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
H5Helper::File *Processing::getOutputFile() const
{
    return outputFile;
}
