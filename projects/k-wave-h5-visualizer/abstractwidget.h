#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

#include <QLayout>
#include <QWidget>

class AbstractWidget
{
public:

public slots:
    virtual void clear() = 0;

protected:
    virtual void clearLayout(QLayout *layout) final;
};

#endif // ABSTRACTWIDGET_H
