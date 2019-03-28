/**
 * @file        opacitygroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing OpacityGroupBox class definition.
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

#include "opacitygroupbox.h"

/**
 * @brief Creates OpacityGroupBox object
 * @param[in] parent Parent (optional)
 */
OpacityGroupBox::OpacityGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::OpacityGroupBox)
{
    ui->setupUi(this);
    opacity = QVector<float>(steps, 1);

    for (int i = 0; i < steps; i++) {
        spinBoxes.append(new QDoubleSpinBoxTS());
        sliders.append(new QSlider(Qt::Vertical));

        spinBoxes[i]->setMaximum(1);
        spinBoxes[i]->setDecimals(3);
        spinBoxes[i]->setSingleStep(0.001);
        spinBoxes[i]->setValue(1);

        sliders[i]->setMaximum(1000);
        sliders[i]->setSingleStep(1);
        sliders[i]->setPageStep(10);
        sliders[i]->setValue(1000);

        ui->gridLayout->addWidget(sliders[i], 0, i, Qt::AlignHCenter);
        ui->gridLayout->addWidget(spinBoxes[i], 1, i, Qt::AlignHCenter);

        sliderMapper.setMapping(sliders[i], i);
        spinBoxesMapper.setMapping(spinBoxes[i], i);
        connect(sliders[i], SIGNAL(valueChanged(int)), &sliderMapper, SLOT(map()));
        connect(spinBoxes[i], SIGNAL(valueChanged(double)), &spinBoxesMapper, SLOT(map()));
    }
    connect(&sliderMapper, SIGNAL(mapped(int)), this, SLOT(sliderValueChanged(int)));
    connect(&spinBoxesMapper, SIGNAL(mapped(int)), this, SLOT(spinBoxValueChanged(int)));
}

/**
 * @brief Destructor of OpacityGroupBox
 *
 * Deletes ui and clears layout.
 */
OpacityGroupBox::~OpacityGroupBox()
{
    clearLayout(ui->gridLayout);
    delete ui;
}

/**
 * @brief Returns opacity vector
 * @return Opacity vector
 */
QVector<float> OpacityGroupBox::getValue() const
{
    return opacity;
}

/**
 * @brief Returns opacity value at index
 * @param[in] index Index
 * @return Opacity value at index
 */
float OpacityGroupBox::getValueAt(int index) const
{
    if (index < opacity.size()) {
        return opacity[index];
    } else {
        qCritical() << "Bad id" << index << "for getting opacity";
        return 0;
    }
}

/**
 * @brief Sets opacity vector
 * @param[in] opacity Opacity vector
 */
void OpacityGroupBox::setValue(QVector<float> opacity)
{
    if (this->opacity.size() == opacity.size()) {
        for (int i = 0; i < spinBoxes.size(); i++) {
            spinBoxes[i]->setValue(double(opacity[i]));
        }
    }
}

/**
 * @brief Sets opacity value at index
 * @param[in] index Index
 * @param[in] value Opacity value at index
 */
void OpacityGroupBox::setValueAt(int index, float value)
{
    if (index < opacity.size()) {
        opacity[index] = value;
        spinBoxes[index]->setValue(double(opacity[index]));
    } else {
        qCritical() << "Bad id" << index << "for setting opacity";
    }
}

/**
 * @brief Disconnects signals and sets opacity to 0
 */
void OpacityGroupBox::clear()
{
    disconnect(this, SIGNAL(valueChanged(QVector<float>)), nullptr, nullptr);
    disconnect(this, SIGNAL(valueChangedAt(int, float)), nullptr, nullptr);
    opacity = QVector<float>(steps, 1);
    for (int i = 0; i < spinBoxes.size(); i++) {
        spinBoxes[i]->setValue(double(opacity[i]));
    }
}

/**
 * @brief Spin box value changed slot
 * @param[in] id Spin box id
 *
 * Sets value to sliders and opacity.
 */
void OpacityGroupBox::spinBoxValueChanged(int id)
{
    sliders[id]->setValue(int(spinBoxes[id]->value() * 1000));
    opacity[id] = float(spinBoxes[id]->value());
    emit valueChanged(opacity);
    emit valueChangedAt(id, opacity[id]);
}

/**
 * @brief Slider value changed slot
 * @param[in] id Slider id
 *
 * Sets value to spin boxes.
 */
void OpacityGroupBox::sliderValueChanged(int id)
{
    spinBoxes[id]->setValue(double(sliders[id]->value()) / 1000);
}
