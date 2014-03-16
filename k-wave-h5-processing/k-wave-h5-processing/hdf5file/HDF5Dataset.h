#ifndef HDF5DATASET_H
#define HDF5DATASET_H

#include "HDF5Object.h"

class HDF5File::HDF5Dataset : public HDF5File::HDF5Object
{
public:
    HDF5Dataset(H5::DataSet dataset, H5std_string name);
    ~HDF5Dataset();

    void readFullDataset(float *&data);
    void readFullDataset(uint64_t *&data);

    void read3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float *&data, float &minVF, float &maxVF);
    void read3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, uint64_t *&data, uint64_t &minVI, uint64_t &maxVI);

    void write3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float *data, bool log = false);
    void write3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, uint64_t *data, bool log = false);

    void readBlock(hsize_t &zO, hsize_t &yO, hsize_t &xO, hsize_t &zC, hsize_t &yC, hsize_t &xC, float *&data, float &minVFTmp, float &maxVFTmp);
    void readBlock(hsize_t &zO, hsize_t &yO, hsize_t &xO, hsize_t &zC, hsize_t &yC, hsize_t &xC, uint64_t *&data, uint64_t &minVFTmp, uint64_t &maxVFTmp);

    void initBlockReading(hsize_t maxSize = HDF5File::SIZE_OF_DATA_PART);
    void setOffset(hsize_t zO_, hsize_t yO_, hsize_t xO_);

    bool isLastBlock();

    hsize_t getBlockSize();

    hsize_t getRank();
    hsize_t getSize();
    hsize_t *getDims();
    hsize_t *getChunkDims();

    hsize_t getId();
    H5std_string getName();

    void findAndSetGlobalMinAndMaxValue(bool reset = false);

    uint64_t getGlobalMaxValueI(bool reset = false);
    uint64_t getGlobalMinValueI(bool reset = false);

    float getGlobalMaxValueF(bool reset = false);
    float getGlobalMinValueF(bool reset = false);

    H5T_class_t getDataType();

private:
    void getMinAndMaxValue(float *data, hsize_t size, float &minVF, float &maxVF);
    void getMinAndMaxValue(uint64_t *data, hsize_t size, uint64_t &minVI, uint64_t &maxVI);

    void findGlobalMinAndMaxValueF();
    void findGlobalMinAndMaxValueI();

    void checkOffsetAndCountParams(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC);

    void recomputeBlock();

    hsize_t x,y,z;
    hsize_t xO, yO, zO;
    hsize_t xC, yC, zC;
    bool blockInitialized;
    bool lastBlock;
    hsize_t blockSize;

    H5::DataSet dataset;
    H5::DataSpace dataspace;
    H5T_class_t type_class;
    H5::IntType typeI;
    H5::FloatType typeF;

    hsize_t rank;
    hsize_t size;
    hsize_t *dims;
    hsize_t *chunk_dims;

    H5std_string name;

    uint64_t maxVI;
    uint64_t minVI;

    float maxVF;
    float minVF;

    bool issetGlobalMinAndMaxValue;
};

#endif // HDF5DATASET_H
