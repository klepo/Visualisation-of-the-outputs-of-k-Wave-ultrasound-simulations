/**
 * @file        vector4d.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The implementation file containing H5Helper::Vector4DT class definition.
 *
 * This class is used for 4D 64-bit unsigned integer vector representation.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "vector4d.h"
#include "vector.cpp"

#ifndef VECTOR4D_CPP
#define VECTOR4D_CPP

namespace H5Helper {

template class Vector4DT<hsize_t>;
template class Vector4DT<float>;

/**
 * @brief Creates Vector4D with zero values
 */
template <class T>
Vector4DT<T>::Vector4DT()
    : VectorT<T>(4)
{
    set(0, 0, 0, 0);
}

/**
 * @brief Copy constructor for general vector
 * @param[in] vector Original Vector object to copy
 */
template <class T>
Vector4DT<T>::Vector4DT(const VectorT<T> &vector)
    : VectorT<T>(4)
{
    copy(vector);
}

/**
 * @brief Creates Vector4D with given fill value
 * @param[in] value Fill value
 */
template <class T>
Vector4DT<T>::Vector4DT(T value)
    : VectorT<T>(4, value)
{
}

/**
 * @brief Copy constructor for Vector3D
 * @param[in] value W value
 * @param[in] vector Original Vector3D object to copy
 */
template <class T>
Vector4DT<T>::Vector4DT(T value, const Vector3DT<T> &vector)
    : VectorT<T>(4)
{
    set(value, vector.z(), vector.y(), vector.x());
}

/**
 * @brief Creates Vector4D with given values
 * @param[in] w W value
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
template <class T>
Vector4DT<T>::Vector4DT(T w, T z, T y, T x)
    : VectorT<T>(4)
{
    set(w, z, y, x);
}

/**
 * @brief Sets vector values
 * @param[in] w W value
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
template <class T>
void Vector4DT<T>::set(T w, T z, T y, T x)
{
    this->vector[0] = w;
    this->vector[1] = z;
    this->vector[2] = y;
    this->vector[3] = x;
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
template <class T>
void Vector4DT<T>::x(T x) const
{
    this->vector[3] = x;
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
template <class T>
void Vector4DT<T>::y(T y) const
{
    this->vector[2] = y;
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
template <class T>
void Vector4DT<T>::z(T z) const
{
    this->vector[1] = z;
}

/**
 * @brief Sets vector w value
 * @param[in] w W value
 */
template <class T>
void Vector4DT<T>::w(T w) const
{
    this->vector[0] = w;
}

/**
 * @brief Sets vector t value (same as w)
 * @param[in] t T value
 */
template <class T>
void Vector4DT<T>::t(T t) const
{
    this->vector[0] = t;
}

/**
 * @brief Returns x value
 * @return X value
 */
template <class T>
T Vector4DT<T>::x() const
{
    return this->vector[3];
}

/**
 * @brief Returns y value
 * @return Y value
 */
template <class T>
T Vector4DT<T>::y() const
{
    return this->vector[2];
}

/**
 * @brief Returns z value
 * @return Z value
 */
template <class T>
T Vector4DT<T>::z() const
{
    return this->vector[1];
}

/**
 * @brief Returns w value
 * @return W value
 */
template <class T>
T Vector4DT<T>::w() const
{
    return this->vector[0];
}

/**
 * @brief Returns t value (same as w)
 * @return T value
 */
template <class T>
T Vector4DT<T>::t() const
{
    return this->vector[0];
}
}

#endif // VECTOR4D_CPP
