#include "HDF5Group.h"

HDF5File::HDF5Group::HDF5Group(H5::Group group, std::string name, HDF5File *hDF5File) : HDF5Object(&group) {
    this->hDF5File = hDF5File;
    this->group = group;
    object = &this->group;
    this->name = name;
}

HDF5File::HDF5Group::~HDF5Group()
{
    std::cout << "Closing group \"" << name << "\"";
    group.close();
    std::cout << " ... OK" << std::endl;

}

hsize_t HDF5File::HDF5Group::getId()
{
    return group.getId();
}


H5std_string HDF5File::HDF5Group::getName()
{
    return name;
}

hsize_t HDF5File::HDF5Group::getNumObjs()
{
    //mutex.lock();
    hsize_t num = group.getNumObjs();
    //mutex.unlock();
    return num;
}
