#ifndef DATASETSDOCKWIDGET_H
#define DATASETSDOCKWIDGET_H

#include <QDockWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>

#include <abstractfilewidget.h>
#include <h5objecttovisualize.h>

namespace Ui {
class DatasetsDockWidget;
}

class DatasetsDockWidget : public QDockWidget, public AbstractFileWidget
{
    Q_OBJECT

public:
    explicit DatasetsDockWidget(QWidget *parent = 0);
    ~DatasetsDockWidget();

public slots:
    void setFile(H5OpenedFile *value);
    void clear();

signals:
    void datasetSelected(H5ObjectToVisualize *object);

private slots:
    void selectDataset();

private:
    Ui::DatasetsDockWidget *ui;
};

#endif // DATASETSDOCKWIDGET_H
