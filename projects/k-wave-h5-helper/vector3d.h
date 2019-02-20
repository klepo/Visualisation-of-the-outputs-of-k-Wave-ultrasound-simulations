/**
 * @file        vector3d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The header file with H5Helper::Vector3DT class declaration.
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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include "vector.h"

namespace H5Helper
{
/**
 * @brief The Vector3DT class represents wrapper for 3D vectors
 */
template <class T>
class Vector3DT : public VectorT<T>
{
public:
    Vector3DT();
    Vector3DT(const VectorT<T> &vector);
    Vector3DT(T value);
    Vector3DT(T z, T y, T x);
    void set(T z, T y, T x);
    void x(T x) const;
    void y(T y) const;
    void z(T z) const;
    T x() const;
    T y() const;
    T z() const;
};

/// Unsigned long long 3D vector datatype
typedef Vector3DT<hsize_t> Vector3D;
/// Float 3D vector datatype
typedef Vector3DT<float> Vector3DF;

}

#endif // VECTOR3D_H
