/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Processing class definition.
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

#include "processing.h"

Processing::Processing(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
{
    this->outputFile = outputFile;
    this->dtsForPcs = dtsForPcs;
    this->settings = settings;
}

Processing::~Processing()
{

}

void Processing::copyAttributes(HDF5Helper::Dataset *srcDataset, HDF5Helper::Dataset *dstDataset)
{
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr, false);
        delete attr;
    }
}

bool Processing::checkDatasetType(HDF5Helper::DatasetType datasetType, std::vector<HDF5Helper::DatasetType> types)
{
    for(std::vector<HDF5Helper::DatasetType>::iterator it = types.begin(); it != types.end(); ++it) {
        if (*it == datasetType)
            return true;
    }
    return false;
}


Settings *Processing::getSettings() const
{
    return settings;
}

DtsForPcs *Processing::getDtsForPcs() const
{
    return dtsForPcs;
}

HDF5Helper::File *Processing::getOutputFile() const
{
    return outputFile;
}
