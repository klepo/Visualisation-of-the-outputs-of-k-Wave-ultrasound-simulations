/**
 * @file        abstractwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with AbstractWidget class declaration.
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

#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

#include <QtCore>
#include <QLayout>
#include <QWidget>

/**
 * @brief The AbstractWidget class represents wrapper for widgets
 */
class AbstractWidget
{
public:
    virtual ~AbstractWidget();

public slots:
    /// Pure virtual clear function
    virtual void clear() = 0;

protected:
    virtual void clearLayout(QLayout *layout) final;
};

#endif // ABSTRACTWIDGET_H
