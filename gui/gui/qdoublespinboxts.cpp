#include "qdoublespinboxts.h"
#include <QDebug>

QDoubleSpinBoxTS::QDoubleSpinBoxTS(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}


QString QDoubleSpinBoxTS::textFromValue(double value) const
{
    return QWidget::locale().toString(value, 'f', 4);
}

double QDoubleSpinBoxTS::valueFromText(const QString &text) const
{
    QString str = text;
    str = str.remove(QWidget::locale().groupSeparator());
    str = str.remove(" ");
    double d = QWidget::locale().toDouble(str);
    return d;
}
