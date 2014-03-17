#ifndef QDOUBLESPINBOXTS_H
#define QDOUBLESPINBOXTS_H

#include <QDoubleSpinBox>

class QDoubleSpinBoxTS : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit QDoubleSpinBoxTS(QWidget *parent = 0);
    virtual QString textFromValue(double value) const;
    virtual double valueFromText(const QString &text) const;

signals:

public slots:

};

#endif // QDOUBLESPINBOXTS_H
