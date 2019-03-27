/**
 * @file        vector.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The header file with H5Helper::VectorT class declaration.
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

#ifndef VECTOR_H
#define VECTOR_H

#ifdef __unix
#include <stdexcept>
#endif

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#pragma warning(disable:4068)
#pragma clang diagnostic ignored "-Wfloat-equal"

namespace H5Helper
{
/// Unsigned long long datatype
typedef unsigned long long hsize_t;
/// Long long datatype
typedef long long hssize_t;

/**
 * @brief The VectorT class represents wrapper for vectors
 */
template <class T>
class VectorT
{
public:
    VectorT();
    VectorT(hsize_t length, T value = 0);
    VectorT(int length, T value = 0);
    VectorT(const VectorT<T> &vector);
    VectorT(VectorT<T> &&vector);
    virtual ~VectorT();
    virtual VectorT<T> &operator=(const VectorT<T> &vector);
    virtual VectorT<T> &operator=(VectorT<T> &&vector);
    virtual bool operator==(const VectorT<T> &vector) const final;
    virtual bool operator!=(const VectorT<T> &vector) const final;
    virtual T &operator[](hsize_t i) final;
    virtual T &operator[](hssize_t i) final;
    virtual T &operator[](int i) final;
    virtual T &operator[](unsigned int i) final;
    virtual T getSize() const final;
    virtual T *getVectorPtr() final;
    virtual T at(hsize_t i) const final;
    virtual T at(hssize_t i) const final;
    virtual T at(int i) const final;
    virtual T at(unsigned int i) const final;
    virtual hsize_t getLength() const final;
    virtual bool hasZeros() const final;

    /**
     * @brief Operator <<
     * @param[in] os std::ostream
     * @param[in] vector Vector
     * @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &os, const VectorT<T> &vector) {
        os << std::string(vector);
        return os;
    }
    operator std::string() const;

protected:
    /// Copy data or part of data
    void copy(const VectorT<T> &vectorSrc);
    /// Vector data
    T *vector = nullptr;
    /// Vector length
    hsize_t length = 0;

private:
    void assign(const VectorT<T> &vector, bool deleteFlag);
    void move(VectorT<T> &vector, bool deleteFlag);
    void checkIndex(hsize_t i) const;
};

/// Unsigned long long vector datatype
typedef VectorT<hsize_t> Vector;
/// Float vector datatype
typedef VectorT<float> VectorF;

}

#endif // VECTOR_H
