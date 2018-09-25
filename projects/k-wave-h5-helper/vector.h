/**
 * @file        vector.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with H5Helper::Vector class declaration.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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

#include <hdf5.h>  // HDF5

namespace H5Helper
{
/**
 * @brief The Vector class represents wrapper for 64-bit unsigned integer vectors
 */
class Vector
{
public:
    Vector();
    Vector(hsize_t length, hsize_t value = 0);
    Vector(int length, hsize_t value = 0);
    Vector(const Vector &vector);
    Vector(Vector &&vector);
    virtual ~Vector();
    virtual Vector &operator=(const Vector &vector);
    virtual Vector &operator=(Vector &&vector);
    virtual bool operator==(const Vector &vector) const final;
    virtual bool operator!=(const Vector &vector) const final;
    virtual hsize_t &operator[](hsize_t i) final;
    virtual hsize_t &operator[](hssize_t i) final;
    virtual hsize_t &operator[](int i) final;
    virtual hsize_t &operator[](unsigned int i) final;
    virtual hsize_t getSize() const final;
    virtual hsize_t *getVectorPtr() final;
    virtual hsize_t at(hsize_t i) const final;
    virtual hsize_t at(hssize_t i) const final;
    virtual hsize_t at(int i) const final;
    virtual hsize_t at(unsigned int i) const final;
    virtual hsize_t getLength() const final;
    virtual bool hasZeros() const final;

    /**
     * @brief Operator <<
     * @param[in] os std::ostream
     * @param[in] vector Vector
     * @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &os, const Vector &vector) {
        os << std::string(vector);
        return os;
    }

    operator std::string() const;

protected:
    /// Copy data or part of data
    void copy(const Vector &vectorSrc);
    /// Vector data
    hsize_t *vector = nullptr;
    /// Vector length
    hsize_t length = 0;


private:
    void assign(const Vector &vector, bool deleteFlag);
    void move(Vector &vector, bool deleteFlag);

    void checkIndex(hsize_t i) const;
};
}

#endif // VECTOR_H
