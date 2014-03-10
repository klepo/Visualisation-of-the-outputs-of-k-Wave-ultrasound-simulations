#ifndef HDF5GROUP_H
#define HDF5GROUP_H

#include "HDF5Object.h"

class HDF5File::HDF5Group : public HDF5File::HDF5Object
{
public:
    HDF5Group(H5::Group group);
    ~HDF5Group();
private:
    H5::Group group;
};

#endif // HDF5GROUP_H
