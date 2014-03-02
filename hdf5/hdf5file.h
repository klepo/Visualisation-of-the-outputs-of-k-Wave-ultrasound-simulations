#ifndef HDF5FILE_H
#define HDF5FILE_H

#include <cpp/H5Cpp.h>  // HDF5
#include <stdint.h>
#include <string>
#include <map>

class HDF5File;

class HDF5Dataset
{
public:
    HDF5Dataset(H5::DataSet dataset);
    ~HDF5Dataset();
    float *readFullDatasetF();
    float *readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &height, hsize_t &width, float &minVF, float &maxVF);
    float *readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &height, hsize_t &width);
    uint64_t *readFullDatasetI();
    uint64_t *readSlabOfDatasetI(int dimension, hsize_t index);
    hsize_t getRank();
    hsize_t *getDims();
    hsize_t *getChunkDims();
    void findAndSetGlobalMinAndMaxValue();
    uint64_t getGlobalMaxValueI();
    uint64_t getGlobalMinValueI();
    float getGlobalMaxValueF();
    float getGlobalMinValueF();
    H5T_class_t getDataType();
    void setDownsampling(hsize_t downsampling = 1);
    hsize_t getDownsampling();

private:
    void getMinAndMaxValueF(float *data, hsize_t size, float &minVF, float &maxVF);
    void getMinAndMaxValueI(uint64_t *data, hsize_t size, uint64_t &minVF, uint64_t &maxVF);
    static float FNULL;
    H5::DataSet dataset;
    H5::DataSpace dataspace;
    H5T_class_t type_class;
    H5::IntType typeI;
    H5::FloatType typeF;
    hsize_t rank;
    hsize_t *dims;
    hsize_t size;
    hsize_t downsampling;

    uint64_t maxVI;
    uint64_t minVI;

    float maxVF;
    float minVF;

    uint64_t *dataI;
    float *dataF;

    bool issetGlobalMinAndMaxValue;
};

class HDF5File {

public:
    HDF5File(std::string filename);
    ~HDF5File();
    HDF5Dataset *getDataset(const H5std_string datasetName);
    static const H5std_string NT;
    static const H5std_string NX;
    static const H5std_string NY;
    static const H5std_string NZ;

    static const H5std_string P_DATASET;
    static const H5std_string P_MODIFIED_DATASET;

    static const H5std_string P_MAX_DATASET;

    static const H5std_string P_FINAL_DATASET;
    static const H5std_string P_MAX_ALL_DATASET;

    static const H5std_string SENSOR_MASK_INDEX_DATASET;

protected:
    uint64_t nT;
    uint64_t nX;
    uint64_t nY;
    uint64_t nZ;
    H5::H5File file;
    std::map<const H5std_string, HDF5Dataset *> datasets;

    void insertDataset(const H5std_string datasetName);
};

#endif // HDF5FILE_H
