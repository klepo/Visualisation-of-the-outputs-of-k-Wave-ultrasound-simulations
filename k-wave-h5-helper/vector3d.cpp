/**
 * @file        vector3d.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing H5Helper::Vector3DT class definition.
 *
 * This class is used for 3D 64-bit unsigned integer vector representation.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "vector3d.h"
#include "vector.cpp"

#ifndef VECTOR3D_CPP
#define VECTOR3D_CPP

namespace H5Helper {

template class Vector3DT<hsize_t>;
template class Vector3DT<float>;

/**
 * @brief Creates Vector3D with zero values
 */
template <class T>
Vector3DT<T>::Vector3DT()
    : VectorT<T>(3)
{
    set(0, 0, 0);
}

/**
 * @brief Copy constructor for general vector
 * @param[in] vector Original Vector object to copy
 */
template <class T>
Vector3DT<T>::Vector3DT(const VectorT<T> &vector)
    : VectorT<T>(3)
{
    copy(vector);
}

/**
 * @brief Creates Vector3D with given fill value
 * @param[in] value Fill value
 */
template <class T>
Vector3DT<T>::Vector3DT(T value)
    : VectorT<T>(3 , value)
{
}

/**
 * @brief Creates Vector3D with given values
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
template <class T>
Vector3DT<T>::Vector3DT(T z, T y, T x)
    : VectorT<T>(3)
{
    set(z, y, x);
}

/**
 * @brief Sets vector values
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
template <class T>
void Vector3DT<T>::set(T z, T y, T x)
{
    this->vector[0] = z;
    this->vector[1] = y;
    this->vector[2] = x;
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
template <class T>
void Vector3DT<T>::x(T x) const
{
    this->vector[2] = x;
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
template <class T>
void Vector3DT<T>::y(T y) const
{
    this->vector[1] = y;
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
template <class T>
void Vector3DT<T>::z(T z) const
{
    this->vector[0] = z;
}

/**
 * @brief Returns x value
 * @return X value
 */
template <class T>
T Vector3DT<T>::x() const
{
    return this->vector[2];
}

/**
 * @brief Returns y value
 * @return Y value
 */
template <class T>
T Vector3DT<T>::y() const
{
    return this->vector[1];
}

/**
 * @brief Returns z value
 * @return Z value
 */
template <class T>
T Vector3DT<T>::z() const
{
    return this->vector[0];
}
}

#endif // VECTOR3D_CPP
