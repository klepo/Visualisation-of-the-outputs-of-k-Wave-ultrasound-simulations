/*
 * @file        HDF5File.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5File class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
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
#include <windows.h>
#endif

#include <stdint.h> // int64_t
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <limits>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mutex>

#ifdef PARALLEL_HDF5
#include <mpi.h>
#endif

#include <hdf5.h>  // HDF5

namespace HDF5Helper
{
double getTime();
size_t getTotalSystemPhysicalMemory();
size_t getAvailableSystemPhysicalMemory();

class HDF5Vector;
class HDF5Vector3D;
class HDF5Vector4D;
class HDF5Dataset;
class HDF5Group;
class HDF5Attribute;

void convertlinearToMultiDim(hsize_t index, HDF5Vector &position, HDF5Vector size);
void convertMultiDimToLinear(HDF5Vector position, hsize_t &index, HDF5Vector size);


static const unsigned int NUMBER_OF_ELEMENTS_TO_LOAD = 100 * 64 * 64 * 64;

typedef std::map<const std::string, HDF5Dataset *> MapOfDatasets;
typedef std::map<const std::string, HDF5Group *> MapOfGroups;
typedef std::pair<const std::string, HDF5Dataset *> PairOfDatasets;
typedef std::pair<const std::string, HDF5Group *> PairOfGroups;

class File
{
public:
    #ifdef PARALLEL_HDF5
        File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log = false);
    #else
        File(std::string filename, unsigned int flag, bool log = false);
    #endif

    ~File();

    HDF5Dataset *openDataset(const std::string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const std::string datasetName);
    void closeDataset(hsize_t idx);

    void createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false);
    void createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false);

    HDF5Group *openGroup(const std::string groupName);
    HDF5Group *openGroup(hsize_t idx);

    void closeGroup(const std::string groupName);

    void createGroup(const std::string groupName, bool rewrite = false);

    hsize_t getNumObjs();
    std::string getObjNameByIdx(hsize_t idx, hid_t fileGroupId = -1);
    H5G_obj_t getObjTypeByIdx(hsize_t idx, hid_t fileGroupId = -1);

    bool objExistsByName(const std::string name);

    std::string getFilename();

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

    static const std::string NT;
    static const std::string NX;
    static const std::string NY;
    static const std::string NZ;
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
    hsize_t nT;
    hsize_t nX;
    hsize_t nY;
    hsize_t nZ;
    std::string filename;

    hsize_t numberOfElementsToLoad;

    hid_t plist_FILE_ACCESS;

    std::ofstream logFileStream;

    static std::mutex mutex;

    hid_t file; // HDF file handle
    MapOfDatasets datasets;
    MapOfGroups groups;

    void insertDataset(const std::string datasetName);
    void insertGroup(const std::string groupName);

    void closeFileAndObjects();

    class HDF5Object;
    herr_t err;
    int mPISize;

    void createDataset(const std::string datasetName, hid_t type, HDF5Vector size, HDF5Vector chunkSize, bool rewrite = false);

};
}

#endif // HDF5FILE_H
