/**
 * @file        opacitygroupbox.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with OpacityGroupBox class declaration.
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

#ifndef OPACITYGROUPBOX_H
#define OPACITYGROUPBOX_H

#include <QGroupBox>
#include <QSlider>

#include "ui_opacitygroupbox.h"

#include "abstractwidget.h"
#include "qdoublespinboxts.h"

namespace Ui {
class OpacityGroupBox;
}

/**
 * @brief The OpacityGroupBox class represents wrapper for the opacity group box
 */
class OpacityGroupBox : public QGroupBox, public AbstractWidget
{
    Q_OBJECT

public:
    explicit OpacityGroupBox(QWidget *parent = nullptr);
    ~OpacityGroupBox();
    QVector<float> getValue() const;
    float getValueAt(int id) const;

signals:
    /**
     * @brief Value changed signal
     * @param[in] opacity Opacity
     */
    void valueChanged(QVector<float> opacity);
    /**
     * @brief Value changed at index signal
     * @param[in] index Opacity index
     * @param[in] value Opacity value
     */
    void valueChangedAt(int index, float value);

public slots:
    void setValue(QVector<float> opacity);
    void setValueAt(int index, float value);
    void clear();

private slots:
    void spinBoxValueChanged(int id);
    void sliderValueChanged(int id);

private:
    Q_DISABLE_COPY(OpacityGroupBox)

    /// User interface
    Ui::OpacityGroupBox *ui;
    /// Opacity
    QVector<float> opacity;
    /// Spin boxes
    QVector<QDoubleSpinBoxTS *> spinBoxes;
    /// Sliders
    QVector<QSlider *> sliders;
    /// Spin boxes mapper
    QSignalMapper spinBoxesMapper;
    /// Slider mapper
    QSignalMapper sliderMapper;
    /// Number of steps (sliders/spin boxes)
    static const int steps = 7;
};

#endif // OPACITYGROUPBOX_H
