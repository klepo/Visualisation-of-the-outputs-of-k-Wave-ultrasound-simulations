#include "datasetinfogroupbox.h"
#include "ui_datasetinfogroupbox.h"

DatasetInfoGroupBox::DatasetInfoGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::DatasetInfoGroupBox)
{
    ui->setupUi(this);
}

DatasetInfoGroupBox::~DatasetInfoGroupBox()
{
    clear();
    delete ui;
}

void DatasetInfoGroupBox::clear()
{
    object = 0;
    clearLayout(ui->formLayoutSelectedDatasetInfo);
}

void DatasetInfoGroupBox::setObject(H5ObjectToVisualize *value)
{
    clear();
    object = value;
    QList<QPair<QString, QString>> info = object->getInfo();
    for (int i = 0; i < info.count(); ++i)
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel(info[i].first + ":"), new QLabel(info[i].second));
}
