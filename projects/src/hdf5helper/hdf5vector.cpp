/*
 * @file        HDF5Vector.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@stud.fit.vutbr.cz
 * @version     0.0
 * @date        16 June 2016
 *
 * @brief       The implementation file containing HDF5Vector class definition.
 *              This class is for multidimensional point representation.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Vector.h"

namespace HDF5Helper {

HDF5Vector::HDF5Vector()
    : vector(0)
    , length(0)
{

}

HDF5Vector::HDF5Vector(hsize_t length, hsize_t value)
{
    this->length = length;
    vector = new hsize_t[length]();
    std::fill_n(vector, length, value);
}

HDF5Vector::HDF5Vector(int length, hsize_t value)
{
    if (length >= 0) {
        this->length = static_cast<hsize_t>(length);
        vector = new hsize_t[static_cast<hsize_t>(length)]();
        std::fill_n(vector, length, value);
    } else {
        throw std::runtime_error("Length of HDF5Vector is less than zero");
    }
}

HDF5Vector::HDF5Vector(const HDF5Vector &hDF5Vector)
{
    assign(hDF5Vector, false);
}

HDF5Vector &HDF5Vector::operator =(const HDF5Vector &hDF5Vector)
{
    if (this != &hDF5Vector) {
        assign(hDF5Vector, true);
    }
    return *this;
}

HDF5Vector::~HDF5Vector()
{
    delete[] vector;
}

hsize_t &HDF5Vector::operator [](hsize_t i)
{
    if (i >= length){
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[i];
}

hsize_t &HDF5Vector::operator [](int i)
{
    if (static_cast<hsize_t>(i) >= length){
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

hsize_t &HDF5Vector::operator [](unsigned int i)
{
    if (static_cast<hsize_t>(i) >= length){
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

hsize_t HDF5Vector::getSize() const
{
    hsize_t size = 1;
    for (hsize_t i = 0; i < length; i++) {
        size *= vector[i];
    }
    return size;
}

hsize_t *HDF5Vector::getVectorPtr()
{
    return vector;
}

void HDF5Vector::assign(const HDF5Vector &hDF5Vector, bool deleteFlag)
{
    if (deleteFlag) {
        delete[] vector;
    }
    length = hDF5Vector.length;

    vector = new hsize_t[length]();
    std::memcpy(vector, hDF5Vector.vector, static_cast<size_t>(length) * sizeof(hsize_t));
}

hsize_t HDF5Vector::getLength() const
{
    return length;
}

bool HDF5Vector::hasZeros() const
{
    for (hsize_t i = 0; i < length; i++) {
        if (vector[i] == 0)
            return true;
    }
    return false;
}

HDF5Helper::HDF5Vector::operator std::string() const
{
    std::string str = "";
    for (hsize_t i = 0; i < length; i++) {
        str += std::to_string(vector[i]);
        if (i < length - 1)
            str += " x ";
    }
    return str;
}
}
