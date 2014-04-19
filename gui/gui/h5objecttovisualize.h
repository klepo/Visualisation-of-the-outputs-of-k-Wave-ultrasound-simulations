#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include <QObject>

#include "openedh5file.h"
#include "hdf5readingthread.h"

class OpenedH5File::H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(QString name, const int type, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5ObjectToVisualize();

    void addSubobject(HDF5File::HDF5Dataset *dataset);
    void addSubobject(HDF5File::HDF5Group *group);

    void setSelectedSubobject(QString name);
    //QString getSelectedSubobjectName();

    //HDF5File::HDF5Dataset *getSelectedDataset();
    //HDF5File::HDF5Group *getSelectedGroup();
    OpenedH5File::H5SubobjectToVisualize *getSelectedSubobject();

    QString getName();
    int getType();
    QList<QString> getSubobjectNames();
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> getSubobjects();

signals:

public slots:
    void toogleSelected();
    void setSelected(bool value);

private:
    OpenedH5File::H5SubobjectToVisualize *selectedSubobject;

    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> subobjects;

    int type;

    OpenedH5File *openedH5File;

    QString name;

    bool selected;

};

#endif // H5OBJECTTOVISUALIZE_H
