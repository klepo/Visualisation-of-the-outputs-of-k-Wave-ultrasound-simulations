#include "HDF5Group.h"

HDF5File::HDF5Group::HDF5Group(H5::Group _group) : HDF5Object(&_group) {
    group = _group;
    object = &group;
}

HDF5File::HDF5Group::~HDF5Group()
{
    group.close();
}
