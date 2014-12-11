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

#include <H5Cpp.h>  // HDF5

#include <stdint.h> // int64_t
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __unix
typedef u_int64_t uint64_t;
#include <stdexcept>
#include <sys/time.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define SIZE_OF_DATA_PART 256 * 256 * 256

#include <mutex>

namespace HDF5Helper
{
    double getTime();
}

class HDF5File
{
public:
    HDF5File(std::string filename, unsigned int flag = HDF5File::OPEN, bool log = false);
    ~HDF5File();

    class HDF5Dataset;
    class HDF5Group;

    HDF5Dataset *openDataset(const H5std_string datasetName);
    HDF5Dataset *openDataset(const hsize_t idx);

    void closeDataset(const H5std_string datasetName);

    void createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = ZERO_CHUNK, bool rewrite = false);
    void createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = ZERO_CHUNK, bool rewrite = false);

    HDF5Group *openGroup(const H5std_string groupName);
    HDF5Group *openGroup(hsize_t idx);

    void closeGroup(const H5std_string groupName);

    void createGroup(const H5std_string groupName, bool rewrite = false);

    hsize_t getNumObjs();
    H5std_string getObjNameById(hsize_t id);
    H5G_obj_t getObjTypeById(hsize_t id);

    H5std_string getFilename();

    void convertlinearTo3D(hsize_t index, hsize_t &z, hsize_t &y, hsize_t &x);
    void convert3DToLinear(hsize_t z, hsize_t y, hsize_t x, hsize_t &index);

    void setSizeOfDataPart(uint64_t size);
    uint64_t getSizeOfDataPart();

    static const H5std_string NT;
    static const H5std_string NX;
    static const H5std_string NY;
    static const H5std_string NZ;
    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;
    static hsize_t ZERO_CHUNK[3];

    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

    std::ofstream *getLogFileStream();

protected:
    uint64_t nT;
    uint64_t nX;
    uint64_t nY;
    uint64_t nZ;
    H5std_string filename;

    std::ofstream logFileStream;

    static std::mutex mutex;

    H5::H5File file;
    std::map<const H5std_string, HDF5Dataset *> datasets;
    std::map<const H5std_string, HDF5Group *> groups;

    void insertDataset(const H5std_string datasetName);
    void insertGroup(const H5std_string groupName);

private:
    class HDF5Object;
    uint64_t sizeOfDataPart;

};

#endif // HDF5FILE_H
