#include "HDF5Attribute.h"

HDF5File::HDF5Object::HDF5Attribute::HDF5Attribute(H5::Attribute attribute)
{
    try {
        //mutex.lock();
        type = attribute.getDataType();
        size = attribute.getInMemDataSize();
        name = attribute.getName();
        space = attribute.getSpace();
        buffer = malloc(size);
        attribute.read(type, buffer);
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

HDF5File::HDF5Object::HDF5Attribute::~HDF5Attribute()
{
    free(buffer);
}

H5::DataType HDF5File::HDF5Object::HDF5Attribute::getDataType()
{
    return type;
}

hsize_t HDF5File::HDF5Object::HDF5Attribute::getSize()
{
    return size;
}

H5std_string HDF5File::HDF5Object::HDF5Attribute::getName()
{
    return name;
}

H5::DataSpace HDF5File::HDF5Object::HDF5Attribute::getSpace()
{
    return space;
}

void *HDF5File::HDF5Object::HDF5Attribute::getData()
{
    return buffer;
}

