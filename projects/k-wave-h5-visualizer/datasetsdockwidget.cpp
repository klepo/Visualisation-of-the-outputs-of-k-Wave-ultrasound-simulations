#include "datasetsdockwidget.h"
#include "ui_datasetsdockwidget.h"

DatasetsDockWidget::DatasetsDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DatasetsDockWidget)
{
    ui->setupUi(this);
}

DatasetsDockWidget::~DatasetsDockWidget()
{
    delete ui;
}

void DatasetsDockWidget::setFile(H5OpenedFile *value)
{
    clear();
    file = value;
    ui->formLayout->addRow("Dataset", new QLabel("Show"));

    // Load objects to visualize from file
    foreach (H5ObjectToVisualize *object, file->getObjects()) {
        // Create radioButton and checkbox
        QRadioButton *radioButton = new QRadioButton(object->getName());
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setAccessibleName(object->getName());

        // Set actions
        connect(radioButton, SIGNAL(clicked()), this, SLOT(selectDataset()));
        //connect(checkBox, SIGNAL(clicked()), this, SLOT(selectDataset()));

        //radioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        ui->formLayout->addRow(radioButton, checkBox);
        radioButton->setChecked(true);
    }
    //QList<QRadioButton *> radioButtons = findChildren<QRadioButton *>();
    //radioButtons.at(0)->setChecked(true);
    adjustSize();
    selectDataset();
}

void DatasetsDockWidget::clear()
{
    file = 0;
    clearLayout(ui->formLayout);
    adjustSize();
}

void DatasetsDockWidget::selectDataset()
{
    QRadioButton *radioButton = qobject_cast<QRadioButton *>(sender());
    /*// Find selected datasets or groups to view
    QList<QCheckBox *> checkBoxes = findChildren<QCheckBox *>();
    foreach (QCheckBox *checkBox, checkBoxes) {
        file->getObject(checkBox->accessibleName())->setSelected(checkBox->isChecked());
    }*/
    if (!radioButton) {
        QList<QRadioButton *> radioButtons = findChildren<QRadioButton *>();
        if (radioButtons.size() > 0)
            radioButton = radioButtons.at(0);
    }

    if (radioButton) {
        if (!radioButton->isChecked())
            radioButton->setChecked(true);
        H5ObjectToVisualize *object = file->getObject(radioButton->text());
        object->setSelected(true);
        emit datasetSelected(object);
    }
}
