#ifndef ABSTRACTFILEWIDGET_H
#define ABSTRACTFILEWIDGET_H

#include <abstractwidget.h>
#include <h5openedfile.h>

class AbstractFileWidget : public AbstractWidget
{
public:
    virtual H5OpenedFile *getFile() const final;

public slots:
    virtual void setFile(H5OpenedFile *value) = 0;

protected:
    H5OpenedFile *file = 0;
};

#endif // ABSTRACTFILEWIDGET_H
