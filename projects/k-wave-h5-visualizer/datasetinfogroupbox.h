#ifndef DATASETINFOGROUPBOX_H
#define DATASETINFOGROUPBOX_H

#include <QGroupBox>
#include <QLabel>

#include <abstractobjectwidget.h>

namespace Ui {
class DatasetInfoGroupBox;
}

class DatasetInfoGroupBox : public QGroupBox, public AbstractObjectWidget
{
    Q_OBJECT

public:
    explicit DatasetInfoGroupBox(QWidget *parent = 0);
    ~DatasetInfoGroupBox();

public slots:
    void setObject(H5ObjectToVisualize *value);
    void clear();

private:
    Ui::DatasetInfoGroupBox *ui;
};

#endif // DATASETINFOGROUPBOX_H
