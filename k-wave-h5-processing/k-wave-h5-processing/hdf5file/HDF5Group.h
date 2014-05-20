#ifndef HDF5GROUP_H
#define HDF5GROUP_H

#include "HDF5Object.h"

class HDF5File::HDF5Group : public HDF5File::HDF5Object
{
public:
    HDF5Group(H5::Group group, H5std_string name, HDF5File *hDF5File);
    ~HDF5Group();

    hsize_t getId();
    H5std_string getName();
    hsize_t getNumObjs();

private:
    H5::Group group;
    H5std_string name;

};

#endif // HDF5GROUP_H
