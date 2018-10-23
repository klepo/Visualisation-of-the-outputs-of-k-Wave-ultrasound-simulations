/**
 * @file        opacitygroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing OpacityGroupBox class definition.
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

#include <opacitygroupbox.h>

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

OpacityGroupBox::~OpacityGroupBox()
{
    clearLayout(ui->gridLayout);
    delete ui;
}

QVector<float> OpacityGroupBox::getValue() const
{
    return opacity;
}

float OpacityGroupBox::getValueAt(int id) const
{
    if (id < opacity.size()) {
        return opacity[id];
    } else {
        qCritical() << "Bad id" << id << "for getting opacity";
        return 0;
    }
}

void OpacityGroupBox::setValue(QVector<float> value)
{
    if (opacity.size() == value.size()) {
        for (int i = 0; i < spinBoxes.size(); i++) {
            spinBoxes[i]->setValue(double(value[i]));
        }
    }
}

void OpacityGroupBox::setValueAt(int id, float value)
{
    if (id < opacity.size()) {
        opacity[id] = value;
        spinBoxes[id]->setValue(double(opacity[id]));
    } else {
        qCritical() << "Bad id" << id << "for setting opacity";
    }
}

void OpacityGroupBox::clear()
{
    disconnect(this, SIGNAL(valueChanged(QVector<float>)), nullptr, nullptr);
    disconnect(this, SIGNAL(valueChangedAt(int, float)), nullptr, nullptr);
    opacity = QVector<float>(steps, 1);
    for (int i = 0; i < spinBoxes.size(); i++) {
        spinBoxes[i]->setValue(double(opacity[i]));
    }
}

void OpacityGroupBox::spinBoxValueChanged(int id)
{
    sliders[id]->setValue(int(spinBoxes[id]->value() * 1000));
    opacity[id] = float(spinBoxes[id]->value());
    emit valueChanged(opacity);
    emit valueChangedAt(id, opacity[id]);
}

void OpacityGroupBox::sliderValueChanged(int id)
{
    spinBoxes[id]->setValue(double(sliders[id]->value()) / 1000);
}
