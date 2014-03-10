#ifndef HDF5OBJECT_H
#define HDF5OBJECT_H

#include "HDF5File.h"

class HDF5File::HDF5Object
{
public:
    HDF5Object(H5::H5Object *object);
    ~HDF5Object();

    class HDF5Attribute;

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

#endif // HDF5OBJECT_H
