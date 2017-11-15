#ifndef OPACITYGROUPBOX_H
#define OPACITYGROUPBOX_H

#include <QGroupBox>
#include <QSlider>

#include <abstractobjectwidget.h>
#include <qdoublespinboxts.h>

namespace Ui {
class OpacityGroupBox;
}

class OpacityGroupBox : public QGroupBox, public AbstractObjectWidget
{
    Q_OBJECT

public:
    explicit OpacityGroupBox(QWidget *parent = 0);
    ~OpacityGroupBox();

public slots:
    void setObject(H5ObjectToVisualize *value);
    void clear();

private slots:
    void spinBoxValueChanged(int id);
    void sliderValueChanged(int id);

private:
    Ui::OpacityGroupBox *ui;
    QVector<float> opacity;
    QVector<QDoubleSpinBoxTS *> spinBoxes;
    QVector<QSlider *> sliders;
    QSignalMapper spinBoxesMapper;
    QSignalMapper sliderMapper;
    static const int steps = 7;

};

#endif // OPACITYGROUPBOX_H
