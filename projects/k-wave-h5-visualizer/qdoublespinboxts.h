/**
 * @file        qdoublespinboxts.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with QDoubleSpinBoxTS class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef QDOUBLESPINBOXTS_H
#define QDOUBLESPINBOXTS_H

#include <QDoubleSpinBox>

/**
 * @brief The QDoubleSpinBoxTS class represents wrapper for modified QDoubleSpinBox
 */
class QDoubleSpinBoxTS : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit QDoubleSpinBoxTS(QWidget *parent = 0);
    virtual QString textFromValue(double value) const;
    virtual double valueFromText(const QString &text) const;

signals:

public slots:

};

#endif // QDOUBLESPINBOXTS_H
