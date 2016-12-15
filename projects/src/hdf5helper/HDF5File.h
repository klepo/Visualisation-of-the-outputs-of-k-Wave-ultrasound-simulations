/**
 * @file        HDF5File.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with HDF5File class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5FILE_H
#define HDF5FILE_H

#ifdef __unix
#include <stdexcept>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h> // GetTickCount etc.
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>
//#include <sys/stat.h>

#ifdef PARALLEL_HDF5
#include <mpi.h>
#endif

#include <HDF5Vector3D.h>
#include <HDF5Vector4D.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>

namespace HDF5Helper
{
double getTime();
size_t getTotalSystemPhysicalMemory();
size_t getAvailableSystemPhysicalMemory();
void convertlinearToMultiDim(hsize_t index, HDF5Vector &position, HDF5Vector size);
void convertMultiDimToLinear(HDF5Vector position, hsize_t &index, HDF5Vector size);

/*inline bool fileExists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}*/

inline bool fileExists(const std::string& name) {
    std::ifstream infile(name);
    return infile.good();}

// Dataset names
const std::string SENSOR_MASK_TYPE_DATASET("sensor_mask_type");
const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");
const std::string SENSOR_MASK_CORNERS_DATASET("sensor_mask_corners");
const std::string NT_DATASET("Nt");
const std::string NX_DATASET("Nx");
const std::string NY_DATASET("Ny");
const std::string NZ_DATASET("Nz");
const std::string P_SOURCE_INPUT_DATASET("p_source_input");

// Attribute names
const std::string MIN_ATTR("min");
const std::string MAX_ATTR("max");
const std::string SRC_DATASET_NAME_ATTR("src_dataset_name");
const std::string C_TYPE_ATTR("c_type");
const std::string C_MOS_ATTR("c_mos");
const std::string C_PERIOD_ATTR("c_period");
const std::string POSITION_Z_ATTR("position_z");
const std::string POSITION_Y_ATTR("position_y");
const std::string POSITION_X_ATTR("position_x");
const std::string SRC_SIZE_Z_ATTR("src_size_z");
const std::string SRC_SIZE_Y_ATTR("src_size_y");
const std::string SRC_SIZE_X_ATTR("src_size_x");

class File
{
public:
    #ifdef PARALLEL_HDF5
        File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log = false);
    #else
        File(std::string filename, unsigned int flag, bool log = false);
    #endif

    ~File();

    HDF5Dataset *openDataset(const std::string datasetName, bool log = true);
    HDF5Dataset *openDataset(hsize_t idx, bool log = true);

    void closeDataset(const std::string datasetName, bool log = true);
    void closeDataset(hsize_t idx, bool log = true);
    void closeDataset(HDF5Dataset *dataset, bool log = true);

    void createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false, bool log = true);
    void createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(const std::string datasetName, hid_t type, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(HDF5Dataset *dataset, bool rewrite = false, bool log = true);

    HDF5Group *openGroup(const std::string groupName, bool log = true);
    HDF5Group *openGroup(hsize_t idx, bool log = true);

    void closeGroup(const std::string groupName, bool log = true);
    void closeGroup(hsize_t idx, bool log = true);
    void closeGroup(HDF5Group *group, bool log = true);

    void createGroup(const std::string groupName, bool rewrite = false, bool log = true);

    hsize_t getNumObjs();
    std::string getObjNameByIdx(hsize_t idx, hid_t fileGroupId = -1);
    H5G_obj_t getObjTypeByIdx(hsize_t idx, hid_t fileGroupId = -1);

    bool objExistsByName(const std::string name);

    std::string getFilename();

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;

    hsize_t getNT();
    hsize_t getNX();
    hsize_t getNY();
    hsize_t getNZ();

    HDF5Vector4D getNdims();

    std::ofstream *getLogFileStream();

    int getMPISize() const;

private:
    HDF5Vector4D nDims;
    std::string filename;

    hsize_t numberOfElementsToLoad = 1024 * 1024 * 1024;

    hid_t plist_FILE_ACCESS;

    std::ofstream logFileStream;

    hid_t file; // HDF file handle
    MapOfDatasets datasets;
    MapOfGroups groups;

    void insertDataset(const std::string datasetName, bool log = true);
    void insertGroup(const std::string groupName, bool log = true);

    void closeFileAndObjects();

    class HDF5Object;
    herr_t err;
    int mPISize;
};

void copyDataset(HDF5Dataset *srcDataset, File *dstFile, bool rewrite = true, bool log = true);
void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite = true, bool log = true);

}

#endif // HDF5FILE_H
