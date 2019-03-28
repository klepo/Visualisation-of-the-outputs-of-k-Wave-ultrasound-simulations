/**
 * @file        vector4d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with H5Helper::Vector4DT class declaration.
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

#ifndef VECTOR4D_H
#define VECTOR4D_H

#include "vector3d.h"

namespace H5Helper
{
/**
 * @brief The Vector4DT class represents wrapper for 4D vectors
 */
template <class T>
class Vector4DT : public VectorT<T>
{
public:
    Vector4DT();
    Vector4DT(const VectorT<T> &vector);
    Vector4DT(T value);
    Vector4DT(T value, const Vector3DT<T> &vector);
    Vector4DT(T w, T z, T y, T x);
    void set(T w, T z, T y, T x);
    void x(T x) const;
    void y(T y) const;
    void z(T z) const;
    void w(T w) const;
    void t(T t) const;
    T x() const;
    T y() const;
    T z() const;
    T w() const;
    T t() const;
};

/// Unsigned long long 4D vector datatype
typedef Vector4DT<hsize_t> Vector4D;
/// Float 4D vector datatype
typedef Vector4DT<float> Vector4DF;

}

#endif // VECTOR4D_H
