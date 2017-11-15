#include "opacitygroupbox.h"
#include "ui_opacitygroupbox.h"

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
    object = 0;
    clearLayout(ui->gridLayout);
    delete ui;
}

void OpacityGroupBox::spinBoxValueChanged(int id)
{
    sliders[id]->setValue(int(spinBoxes[id]->value() * 1000));
    opacity[id] = float(spinBoxes[id]->value());
    if (object != 0)
        object->setOpacity(opacity);
}

void OpacityGroupBox::sliderValueChanged(int id)
{
    spinBoxes[id]->setValue(double(sliders[id]->value()) / 1000);
}

void OpacityGroupBox::setObject(H5ObjectToVisualize *value)
{
    clear();
    object = value;
    if (object->getOpacity().size() == opacity.size()) {
        opacity = object->getOpacity();
    } else {
        opacity = QVector<float>(steps, 1);
        object->setOpacity(opacity);
    }

    for (int i = 0; i < spinBoxes.size(); i++)
        spinBoxes[i]->setValue(opacity[i]);
}

void OpacityGroupBox::clear()
{
    object = 0;
    opacity = QVector<float>(steps, 1);
    for (int i = 0; i < spinBoxes.size(); i++)
        spinBoxes[i]->setValue(opacity[i]);
}
