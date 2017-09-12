/**
 * @file        file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with HDF5Helper::File class declaration.
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef FILE_H
#define FILE_H

#ifdef __unix
    #include <stdexcept>
    #include <sys/time.h>
    #include <unistd.h>
    #include <sys/mman.h>
#endif

#ifdef _WIN32
    #define NOMINMAX
    #include <Windows.h> // GetTickCount etc.
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>

#ifdef PARALLEL_HDF5
    #include <mpi.h>
#endif

#include <vector3d.h>
#include <vector4d.h>
#include <group.h>
#include <dataset.h>

namespace HDF5Helper
{
/// Dataset name sensor_mask_type
const std::string SENSOR_MASK_TYPE_DATASET = "sensor_mask_type";
/// Dataset name sensor_mask_index
const std::string SENSOR_MASK_INDEX_DATASET = "sensor_mask_index";
/// Dataset name sensor_mask_corners
const std::string SENSOR_MASK_CORNERS_DATASET = "sensor_mask_corners";
/// Dataset name Nt
const std::string NT_DATASET = "Nt";
/// Dataset name Nx
const std::string NX_DATASET = "Nx";
/// Dataset name Ny
const std::string NY_DATASET = "Ny";
/// Dataset name Nz
const std::string NZ_DATASET = "Nz";
/// Dataset name p_source_input
const std::string P_SOURCE_INPUT_DATASET = "p_source_input";

/// Attribute name min
const std::string MIN_ATTR = "min";
/// Attribute name max
const std::string MAX_ATTR = "max";
/// Attribute name min_index
const std::string MIN_INDEX_ATTR = "min_index";
/// Attribute name max_index
const std::string MAX_INDEX_ATTR = "max_index";
/// Attribute name src_dataset_name
const std::string SRC_DATASET_NAME_ATTR = "src_dataset_name";
/// Attribute name c_type
const std::string C_TYPE_ATTR = "c_type";
/// Attribute name c_mos
const std::string C_MOS_ATTR = "c_mos";
/// Attribute name c_period
const std::string C_PERIOD_ATTR = "c_period";
/// Attribute name c_harmonic
const std::string C_HARMONIC_ATTR = "c_harmonic";
/// Attribute name c_harmonics
const std::string C_HARMONICS_ATTR = "c_harmonics";
/// Attribute name position_z
const std::string POSITION_Z_ATTR = "position_z";
/// Attribute name position_y
const std::string POSITION_Y_ATTR = "position_y";
/// Attribute name position_x
const std::string POSITION_X_ATTR = "position_x";
/// Attribute name src_size_z
const std::string SRC_SIZE_Z_ATTR = "src_size_z";
/// Attribute name src_size_y
const std::string SRC_SIZE_Y_ATTR = "src_size_y";
/// Attribute name src_size_x
const std::string SRC_SIZE_X_ATTR = "src_size_x";

/**
 * @brief The File class represents wrapper for the HDF5 files
 */
class File
{
public:
    #ifdef PARALLEL_HDF5
        File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log = false);
    #else
        File(std::string filename, unsigned int flag, bool log = false);
    #endif

    ~File();

    void createDatasetI(const std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDatasetF(const std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(const std::string name, hid_t type, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(Dataset *dataset, bool rewrite = false, bool log = true);

    Dataset *openDataset(const std::string name, bool log = true);
    Dataset *openDataset(hsize_t idx, bool log = true);

    bool isDatasetOpened(const std::string name);
    bool isDatasetOpened(hsize_t idx);

    void closeDataset(const std::string name, bool log = true);
    void closeDataset(hsize_t idx, bool log = true);
    void closeDataset(Dataset *dataset, bool log = true);

    void createGroup(const std::string name, bool rewrite = false, bool log = true);

    Group *openGroup(const std::string name, bool log = true);
    Group *openGroup(hsize_t idx, bool log = true);

    void closeGroup(const std::string name, bool log = true);
    void closeGroup(hsize_t idx, bool log = true);
    void closeGroup(Group *group, bool log = true);

    hsize_t getNumObjs(hid_t groupId = -1);
    std::string getObjNameByIdx(hsize_t idx, hid_t groupId = -1);
    H5G_obj_t getObjTypeByIdx(hsize_t idx, hid_t groupId = -1);

    bool objExistsByName(const std::string name);

    std::string getFilename();

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

    Vector4D getNdims();

    std::ofstream *getLogFileStream();

    int getMPISize() const;

    /// Open file flag
    static const unsigned int OPEN = 0;
    /// Create file flag
    static const unsigned int CREATE = 1;

private:
    class HDF5Object;
    void insertDataset(const std::string name, bool log = true);
    void insertGroup(const std::string name, bool log = true);

    void closeFileAndObjects();

    Vector4D nDims;
    std::string filename;
    hsize_t numberOfElementsToLoad = 512 * 512 * 512;
    hid_t plist_FILE_ACCESS;
    std::ofstream logFileStream;

    /// HDF file handle
    hid_t file;
    /// Map of datasets
    MapOfDatasets datasets;
    /// Map of groups
    MapOfGroups groups;

    herr_t err;
    int mPISize;
};

// General functions
double getTime();
size_t getTotalSystemPhysicalMemory();
size_t getAvailableSystemPhysicalMemory();
void convertlinearToMultiDim(hsize_t index, Vector &position, Vector size);
void convertMultiDimToLinear(Vector position, hsize_t &index, Vector size);
void checkOrSetMinMaxValue(bool &first, float &minV, float &maxV, const float value);
void checkOrSetMinMaxValue(bool &first, hsize_t &minV, hsize_t &maxV, const hsize_t value);
bool fileExists(const std::string& name);
void copyDataset(Dataset *srcDataset, File *dstFile, bool rewrite = true, bool log = true);
void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite = true, bool log = true);
}

#endif // FILE_H
