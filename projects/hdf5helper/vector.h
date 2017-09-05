/**
 * @file        vector.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with HDF5Helper::Vector class declaration.
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
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

namespace HDF5Helper
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
    virtual ~Vector();
    virtual Vector &operator =(const Vector &vector);
    virtual bool operator ==(const Vector &vector) const;
    virtual bool operator !=(const Vector &vector) const;
    virtual hsize_t &operator [](hsize_t i) final;
    virtual hsize_t &operator [](hssize_t i) final;
    virtual hsize_t &operator [](int i) final;
    virtual hsize_t &operator [](unsigned int i) final;
    virtual hsize_t getSize() const final;
    virtual hsize_t *getVectorPtr() final;
    hsize_t getLength() const;
    bool hasZeros() const;

    /**
     * @brief Operator <<
     * @param os std::ostream
     * @param vector Vector
     * @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &os, const Vector &vector) {
        os << std::string(vector);
        return os;
    }

    operator std::string() const;

private:
    void assign(const Vector &vector, bool deleteFlag);

protected:
    /// Vector data
    hsize_t *vector = 0;
    /// Vector length
    hsize_t length = 0;
};
}

#endif // VECTOR_H
