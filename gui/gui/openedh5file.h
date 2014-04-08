#ifndef H5FILE_H
#define H5FILE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QDebug>

#include <HDF5File.h>
#include <HDF5Group.h>
#include <HDF5Dataset.h>
#include <HDF5Attribute.h>

class OpenedH5File : public QObject
{
    Q_OBJECT
public:
    explicit OpenedH5File(QString fileName, QObject *parent = 0);
    ~OpenedH5File();

    class H5ObjectToVisualize;

    QMap<QString, H5ObjectToVisualize *> getObjects();
    QMap<QString, QString> getInfo();
    HDF5File *getFile();

    QString getFilename();
    QString getRawFilename();
    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

signals:

public slots:

protected:
    HDF5File *file;
    uint64_t nT, nX, nY, nZ;

    QMap<QString, QString> info;
    QMap<QString, H5ObjectToVisualize *> objects;
};

#endif // H5FILE_H
