#ifndef HDF5FILE_H
#define HDF5FILE_H

#include <H5Cpp.h>  // HDF5

#include <stdint.h>
#include <string>
#include <map>

class HDF5File
{
public:
    HDF5File(std::string filename, unsigned int flag = HDF5File::OPEN);
    ~HDF5File();

private:
    class HDF5Object
    {
    public:
        HDF5Object(H5::H5Object *object);
        ~HDF5Object();

        class HDF5Attribute
        {
        public:
            HDF5Attribute(H5::Attribute attribute);
            ~HDF5Attribute();

            H5::DataType getDataType();
            hsize_t getSize();
            H5std_string getName();
            H5::DataSpace getSpace();
            void *getData();

        protected:
            H5::DataType type;
            hsize_t size;
            H5std_string name;
            H5::DataSpace space;
            void *buffer;
        };

        void setAttribute(const H5std_string name, float value);
        void setAttribute(const H5std_string name, double value);
        void setAttribute(const H5std_string name, int value);
        void setAttribute(const H5std_string name, uint64_t value);
        void setAttribute(const H5std_string name, const H5std_string value);

        float readAttributeF(const H5std_string name);
        uint64_t readAttributeI(const H5std_string name);
        H5std_string readAttributeS(const H5std_string name);

        HDF5Attribute getAttribute(H5std_string name);
        HDF5Attribute getAttribute(const unsigned int idx);
        void setAttribute(HDF5Attribute attribute);

        void removeAttribute(const unsigned int idx);
        void removeAttribute(H5std_string name);

        int getNumAttrs();

    protected:
        H5::H5Object *object;
    };

public:
    class HDF5Dataset : public HDF5Object
    {
    public:
        HDF5Dataset(H5::DataSet dataset, H5std_string name);
        ~HDF5Dataset();

        float *readFullDatasetF();
        uint64_t *readFullDatasetI();

        float *readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &height, hsize_t &width, float &minVF, float &maxVF);
        float *readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &height, hsize_t &width);

        float *readDatasetF(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, float &minVF, float &maxVF);
        uint64_t *readDatasetI(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, uint64_t &minVI, uint64_t &maxVI);

        void writeDataset(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, float *data);

        hsize_t getRank();
        hsize_t *getDims();
        hsize_t *getChunkDims();

        H5std_string getName();

        void findAndSetGlobalMinAndMaxValue(bool reset = false);

        uint64_t getGlobalMaxValueI(bool reset = false);
        uint64_t getGlobalMinValueI(bool reset = false);

        float getGlobalMaxValueF(bool reset = false);
        float getGlobalMinValueF(bool reset = false);

        H5T_class_t getDataType();

        void setDownsampling(hsize_t downsampling = 1);
        hsize_t getDownsampling();

    private:
        void getMinAndMaxValueF(float *data, hsize_t size, float &minVF, float &maxVF);
        void getMinAndMaxValueI(uint64_t *data, hsize_t size, uint64_t &minVI, uint64_t &maxVI);

        void findGlobalMinAndMaxValueF();
        void findGlobalMinAndMaxValueI();

        void checkOffsetAndCountParams(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC);

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

        H5std_string name;

        uint64_t maxVI;
        uint64_t minVI;

        float maxVF;
        float minVF;

        uint64_t *dataI;
        float *dataF;

        bool issetGlobalMinAndMaxValue;
    };

    class HDF5Group : public HDF5Object
    {
    public:
        HDF5Group(H5::Group group);
        ~HDF5Group();
    private:
        H5::Group group;
    };

    HDF5Dataset *openDataset(const H5std_string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const H5std_string datasetName);

    void createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = NULL);
    void createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = NULL);

    HDF5Group *getGroup(const H5std_string groupName);
    void createGroup(const H5std_string groupName);

    void unlinkLocation(const H5std_string name);

    hsize_t getNumObjs();

    static const H5std_string NT;
    static const H5std_string NX;
    static const H5std_string NY;
    static const H5std_string NZ;
    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;

    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

protected:
    uint64_t nT;
    uint64_t nX;
    uint64_t nY;
    uint64_t nZ;
    H5std_string filename;

    H5::H5File file;
    std::map<const H5std_string, HDF5Dataset *> datasets;
    std::map<const H5std_string, HDF5Group *> groups;

    void insertDataset(const H5std_string datasetName);
    void insertGroup(const H5std_string groupName);
};

#endif // HDF5FILE_H
