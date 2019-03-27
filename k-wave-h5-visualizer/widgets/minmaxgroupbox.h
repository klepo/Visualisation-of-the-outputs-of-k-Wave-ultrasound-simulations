/**
 * @file        minmaxgroupbox.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              29 October   2018 (updated)
 *
 * @brief       The header file with MinMaxGroupBox class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef MINMAXGROUPBOX_H
#define MINMAXGROUPBOX_H

#include <QGroupBox>

#include "ui_minmaxgroupbox.h"

#include "abstractwidget.h"

namespace Ui {
class MinMaxGroupBox;
}

/**
 * @brief The MinMaxGroupBox class represents wrapper for the min/max group box
 */
class MinMaxGroupBox : public QGroupBox, public AbstractWidget
{
    Q_OBJECT

public:
    explicit MinMaxGroupBox(QWidget *parent = nullptr);
    ~MinMaxGroupBox();

    float getMinValue() const;
    float getMaxValue() const;
    bool getMinMaxValuesTrim() const;

signals:
    /**
     * @brief Min value changed signal
     * @param[in] min Minimal value
     */
    void minValueChanged(float min);
    /**
     * @brief Max value changed signal
     * @param[in] max Maximal value
     */
    void maxValueChanged(float max);
    /**
     * @brief Min/Max value trim changed signal
     * @param[in] value True/False
     */
    void minMaxValuesTrimChanged(bool value);

public slots:
    void setRange(float min, float max);
    void setValues(float min, float max);
    void setMinValue(float min);
    void setMaxValue(float max);
    void setMinMaxValuesTrim(bool value);
    void clear();

private slots:
    void on_horizontalSliderGlobalMin_valueChanged(int value);
    void on_horizontalSliderGlobalMax_valueChanged(int value);
    void on_doubleSpinBoxMinGlobal_valueChanged(double value);
    void on_doubleSpinBoxMaxGlobal_valueChanged(double value);

private:
    Q_DISABLE_COPY(MinMaxGroupBox)

    /// User interface
    Ui::MinMaxGroupBox *ui;
};

#endif // MINMAXGROUPBOX_H
