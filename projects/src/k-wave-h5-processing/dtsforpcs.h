/**
 * @file        dtsforpcs.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with HDF5Attribute class declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef DTSFORPCS_H
#define DTSFORPCS_H

#include <hdf5helper.h>
#include <filescontext.h>

class DtsForPcs
{
public:
    DtsForPcs(FilesContext *filesContext, Settings *settings);
    HDF5Helper::HDF5Vector4D getNDims() const;
    HDF5Helper::HDF5Dataset *getSensorMaskIndexDataset() const;
    HDF5Helper::HDF5Dataset *getSensorMaskCornersDataset() const;
    hsize_t getSensorMaskSize() const;
    hsize_t getSensorMaskType() const;
    HDF5Helper::MapOfDatasets getDts3DType() const;
    HDF5Helper::MapOfDatasets getDts3DTypeDsp() const;
    HDF5Helper::MapOfDatasets getDtsMaskType() const;
    HDF5Helper::MapOfGroups getGroupsCuboidType() const;
    HDF5Helper::MapOfGroups getGroupsCuboidTypeAttr() const;
    HDF5Helper::MapOfGroups getGroupsCuboidTypeAttrDsp() const;

private:
    HDF5Helper::HDF5Dataset *findAndGetDataset(const std::string name, HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile);
    void findDatasetsForProcessing(HDF5Helper::File *hDF5SimOutputFile, Settings *settings);

    HDF5Helper::HDF5Vector4D nDims;

    HDF5Helper::HDF5Dataset *sensorMaskIndexDataset = 0;
    HDF5Helper::HDF5Dataset *sensorMaskCornersDataset = 0;
    HDF5Helper::HDF5Dataset *sourceInputDataset = 0;
    hsize_t sensorMaskSize = 0;
    hsize_t sensorMaskType = 0;
    HDF5Helper::MapOfDatasets dts3DType;
    HDF5Helper::MapOfDatasets dts3DTypeDsp;
    HDF5Helper::MapOfDatasets dtsMaskType;
    HDF5Helper::MapOfGroups groupsCuboidType;
    HDF5Helper::MapOfGroups groupsCuboidTypeAttr;
    HDF5Helper::MapOfGroups groupsCuboidTypeAttrDsp;


private:
    // Disable copy
    DtsForPcs(const DtsForPcs &);
};

#endif // DTSFORPCS_H
