/**
 * @file        HDF5Vector4D.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing HDF5Vector4D class definition.
 *              This class is for 4D point representation.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5Vector4D.h"

namespace HDF5Helper {

/**
 * @brief HDF5Vector4D::HDF5Vector4D
 */
HDF5Vector4D::HDF5Vector4D()
    : HDF5Vector(4)
{
    set(0, 0, 0, 0);
}

/**
 * @brief HDF5Vector4D::HDF5Vector4D
 * @param hDF5Vector
 * @throw std::runtime_error
 */
HDF5Vector4D::HDF5Vector4D(const HDF5Vector &hDF5Vector)
    : HDF5Vector(4)
{
    HDF5Vector tmp = hDF5Vector;
    hsize_t length = hDF5Vector.getLength();

    if (4 > length) {
        throw std::runtime_error("Convert error - Length of HDF5Vector is not 4");
    }

    set(tmp[length - 4], tmp[length - 3], tmp[length - 2], tmp[length - 1]);
}

/**
 * @brief HDF5Vector4D::HDF5Vector4D
 * @param value
 */
HDF5Vector4D::HDF5Vector4D(const hsize_t value) : HDF5Vector(4, value)
{

}

/**
 * @brief HDF5Vector4D::HDF5Vector4D
 * @param value
 * @param hDF5Vector
 */
HDF5Vector4D::HDF5Vector4D(const hsize_t value, const HDF5Vector3D &hDF5Vector)
    : HDF5Vector(4)
{
    set(value, hDF5Vector.z(), hDF5Vector.y(), hDF5Vector.x());
}

/**
 * @brief HDF5Vector4D::HDF5Vector4D
 * @param w
 * @param z
 * @param y
 * @param x
 */
HDF5Vector4D::HDF5Vector4D(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
    : HDF5Vector(4)
{
    set(w, z, y, x);
}

/**
 * @brief HDF5Vector4D::set
 * @param w
 * @param z
 * @param y
 * @param x
 */
void HDF5Vector4D::set(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = w;
    vector[1] = z;
    vector[2] = y;
    vector[3] = x;
}

/**
 * @brief HDF5Vector4D::set
 * @param w
 * @param z
 * @param y
 * @param x
 */
void HDF5Vector4D::set(const int w, const int z, const int y, const int x)
{
    set(static_cast<hsize_t>(w), static_cast<hsize_t>(z), static_cast<hsize_t>(y), static_cast<hsize_t>(x));
}

/**
 * @brief HDF5Vector4D::x
 * @param x
 */
void HDF5Vector4D::x(const hsize_t x)
{
    vector[3] = x;
}

/**
 * @brief HDF5Vector4D::y
 * @param y
 */
void HDF5Vector4D::y(const hsize_t y)
{
    vector[2] = y;
}

/**
 * @brief HDF5Vector4D::z
 * @param z
 */
void HDF5Vector4D::z(const hsize_t z)
{
    vector[1] = z;
}

/**
 * @brief HDF5Vector4D::w
 * @param w
 */
void HDF5Vector4D::w(const hsize_t w)
{
    vector[0] = w;
}

/**
 * @brief HDF5Vector4D::x
 * @param x
 */
void HDF5Vector4D::x(const int x)
{
    this->x(static_cast<hsize_t>(x));
}

/**
 * @brief HDF5Vector4D::y
 * @param y
 */
void HDF5Vector4D::y(const int y)
{
    this->y(static_cast<hsize_t>(y));
}

/**
 * @brief HDF5Vector4D::z
 * @param z
 */
void HDF5Vector4D::z(const int z)
{
    this->z(static_cast<hsize_t>(z));
}

/**
 * @brief HDF5Vector4D::w
 * @param w
 */
void HDF5Vector4D::w(const int w)
{
    this->w(static_cast<hsize_t>(w));
}

/**
 * @brief HDF5Vector4D::x
 * @return
 */
hsize_t HDF5Vector4D::x() const
{
    return vector[3];
}

/**
 * @brief HDF5Vector4D::y
 * @return
 */
hsize_t HDF5Vector4D::y() const
{
    return vector[2];
}

/**
 * @brief HDF5Vector4D::z
 * @return
 */
hsize_t HDF5Vector4D::z() const
{
    return vector[1];
}

/**
 * @brief HDF5Vector4D::w
 * @return
 */
hsize_t HDF5Vector4D::w() const
{
    return vector[0];
}
}