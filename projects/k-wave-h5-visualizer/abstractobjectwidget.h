#ifndef ABSTRACTOBJECTWIDGET_H
#define ABSTRACTOBJECTWIDGET_H

#include <abstractwidget.h>
#include <h5objecttovisualize.h>

class AbstractObjectWidget : public AbstractWidget
{
public:
    virtual H5ObjectToVisualize *getObject() const final;

public slots:
    virtual void setObject(H5ObjectToVisualize *value) = 0;

protected:
    H5ObjectToVisualize *object = 0;
};

#endif // ABSTRACTOBJECTWIDGET_H
