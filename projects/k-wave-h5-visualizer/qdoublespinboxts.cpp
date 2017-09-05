/**
 * @file        qdoublespinboxts.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing QDoubleSpinBoxTS class definition.
 *
 * Double spin box with group separator.
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

#include "qdoublespinboxts.h"

QDoubleSpinBoxTS::QDoubleSpinBoxTS(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}


QString QDoubleSpinBoxTS::textFromValue(double value) const
{
    return QWidget::locale().toString(value, 'f', 3);
}

double QDoubleSpinBoxTS::valueFromText(const QString &text) const
{
    QString str = text;
    str = str.remove(QWidget::locale().groupSeparator());
    //str = str.remove(" ");
    double d = QWidget::locale().toDouble(str);
    return d;
}
