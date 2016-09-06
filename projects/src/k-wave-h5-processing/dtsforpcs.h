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
    HDF5Helper::HDF5Dataset *findAndGetSensorMaskIndexDataset(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile);
    HDF5Helper::HDF5Dataset *findAndGetSensorMaskCornersDataset(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile);
    void findDatasetsForProcessing(HDF5Helper::File *hDF5SimOutputFile, Settings *settings);

    HDF5Helper::HDF5Vector4D nDims;

    // TODO set NULL
    HDF5Helper::HDF5Dataset *sensorMaskIndexDataset;
    HDF5Helper::HDF5Dataset *sensorMaskCornersDataset;
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