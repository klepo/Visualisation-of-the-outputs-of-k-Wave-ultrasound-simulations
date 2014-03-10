#ifndef HDF5ATTRIBUTE_H
#define HDF5ATTRIBUTE_H

#include "HDF5Object.h"

class HDF5File::HDF5Object::HDF5Attribute
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

#endif // HDF5ATTRIBUTE_H
