/**
 * @file        qdoublespinboxts.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing QDoubleSpinBoxTS class definition.
 *
 * Double spin box with group separator.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "qdoublespinboxts.h"

/**
 * @brief Creates QDoubleSpinBoxTS object
 * @param[in] parent Parent (optional)
 */
QDoubleSpinBoxTS::QDoubleSpinBoxTS(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}

/**
 * @brief String from double value
 * @param[in] value Value
 * @return Value string
 */
QString QDoubleSpinBoxTS::textFromValue(double value) const
{
    return QWidget::locale().toString(value, 'f', 3);
}

/**
 * @brief Value from text
 * @param[in] text Text
 * @return Double value
 */
double QDoubleSpinBoxTS::valueFromText(const QString &text) const
{
    QString str = text;
    str = str.remove(QWidget::locale().groupSeparator());
    //str = str.remove(" ");
    double d = QWidget::locale().toDouble(str);
    return d;
}
