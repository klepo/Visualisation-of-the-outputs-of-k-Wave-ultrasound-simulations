#include "minmaxgroupbox.h"
#include "ui_minmaxgroupbox.h"

MinMaxGroupBox::MinMaxGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::MinMaxGroupBox)
{
    ui->setupUi(this);
    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), this, SIGNAL(setTrim(bool)));
}

MinMaxGroupBox::~MinMaxGroupBox()
{
    object = 0;
    delete ui;
}

void MinMaxGroupBox::setObject(H5ObjectToVisualize *value)
{
    clear();

    // Init min and max controls
    ui->doubleSpinBoxMinGlobal->setRange(value->getOriginalMinValue(), value->getOriginalMaxValue());
    ui->doubleSpinBoxMaxGlobal->setRange(value->getOriginalMinValue(), value->getOriginalMaxValue());
    ui->doubleSpinBoxMinGlobal->setSingleStep((value->getOriginalMaxValue() - value->getOriginalMinValue()) / 1000);
    ui->doubleSpinBoxMaxGlobal->setSingleStep((value->getOriginalMaxValue() - value->getOriginalMinValue()) / 1000);
    ui->doubleSpinBoxMinGlobal->setValue(value->getMinValue());
    ui->doubleSpinBoxMaxGlobal->setValue(value->getMaxValue());
    on_doubleSpinBoxMinGlobal_valueChanged(value->getMinValue());
    on_doubleSpinBoxMaxGlobal_valueChanged(value->getMaxValue());

    object = value;
}

void MinMaxGroupBox::clear()
{
    object = 0;
    ui->doubleSpinBoxMinGlobal->setRange(0, 0);
    ui->doubleSpinBoxMaxGlobal->setRange(0, 0);
    ui->doubleSpinBoxMinGlobal->setSingleStep(0.1);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(0.1);
    ui->doubleSpinBoxMinGlobal->setValue(0);
    ui->doubleSpinBoxMaxGlobal->setValue(0);
}

void MinMaxGroupBox::on_horizontalSliderGlobalMin_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMinGlobal->setValue(double(value) / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MinMaxGroupBox::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    ui->doubleSpinBoxMaxGlobal->setValue(double(value) / 1000 * (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum()) + ui->doubleSpinBoxMaxGlobal->minimum());
}

void MinMaxGroupBox::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    if (object != 0)
        object->setMinValue(value);

    // Recompute integers to doubles
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition(qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);
}

void MinMaxGroupBox::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    if (object != 0)
        object->setMaxValue(value);

    ui->horizontalSliderGlobalMax->setTracking(false);
    ui->horizontalSliderGlobalMax->setSliderPosition(qRound(1000 * (value - ui->doubleSpinBoxMaxGlobal->minimum()) / (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum())));
    ui->horizontalSliderGlobalMax->setTracking(true);
}
