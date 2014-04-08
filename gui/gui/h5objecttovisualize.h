#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include <QObject>

#include "openedh5file.h"

class OpenedH5File::H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(QString name, const int type, OpenedH5File *openedH5File, QObject *parent = 0);

    void addName(QString name);
    QString getName();
    int getType();
    QList<QString> getNames();

    static const int DATASET_TYPE = 0;
    static const int GROUP_TYPE = 1;

signals:

public slots:
    void selectObject(QString name);

private:
    OpenedH5File *openedH5File;
    QString name;
    QString selectedName;
    HDF5File::HDF5Dataset *selectedDataset;
    HDF5File::HDF5Group *selectedGroup;
    int type;
    QList<QString> names;
};

#endif // H5OBJECTTOVISUALIZE_H
