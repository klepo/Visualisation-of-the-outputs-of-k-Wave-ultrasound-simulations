#include "HDF5Group.h"

HDF5File::HDF5Group::HDF5Group(H5::Group _group, std::string _name) : HDF5Object(&_group) {
    group = _group;
    object = &group;
    name = _name;
}

HDF5File::HDF5Group::~HDF5Group()
{
    std::cout << "Closing group \"" << name << "\"";
    group.close();
    std::cout << " ... OK" << std::endl;

}

H5std_string HDF5File::HDF5Group::getName()
{
    return name;
}

hsize_t HDF5File::HDF5Group::getNumObjs()
{
    return group.getNumObjs();
}
