#include "HDF5Group.h"

HDF5File::HDF5Group::HDF5Group(H5::Group _group, std::string _name) : HDF5Object(&_group) {
    group = _group;
    object = &group;
    name = _name;
}

HDF5File::HDF5Group::~HDF5Group()
{
    group.close();
}

H5std_string HDF5File::HDF5Group::getName()
{
    return name;
}

hsize_T HDF5File::HDF5Group::getNumObjs()
{
    return group.getNumObjs();
}
