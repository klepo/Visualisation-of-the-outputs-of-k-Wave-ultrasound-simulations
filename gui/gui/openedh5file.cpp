#include "openedh5file.h"
#include "h5objecttovisualize.h"

#include <QDebug>

OpenedH5File::OpenedH5File(QString fileName, QObject *parent) :
    QObject(parent)
{
    // Open HDF5 file
    file = new HDF5File(fileName.toStdString(), HDF5File::OPEN);

    selectedObject = NULL;

    // Load dimensions
    nT = file->getNT();
    nX = file->getNX();
    nY = file->getNY();
    nZ = file->getNZ();

    qDebug() << "Load info (root attributes)...";

    // Load info
    HDF5File::HDF5Group *group = file->openGroup("/");
    for (int i = 0; i < group->getNumAttrs(); i++) {
        HDF5File::HDF5Group::HDF5Attribute *attribute = group->getAttribute(i);
        QString value((const char *) attribute->getData());
        info.insert(QString::fromStdString(attribute->getName()), value);
    }
    file->closeGroup("/");

    qRegisterMetaType<OpenedH5File::H5ObjectToVisualize *>("OpenedH5File::H5ObjectToVisualize");

     qDebug() << "Find datasets for visualization...";

    // Find datasets for visualization
    for (hsize_t i = 0; i < file->getNumObjs(); i++) {
        H5G_obj_t type = file->getObjTypeById(i);
        if (type == H5G_DATASET) {
            HDF5File::HDF5Dataset *dataset = file->openDataset(i);
            hsize_t *size = dataset->getDims();
            // 3D type
            if (dataset->getDataType() == H5T_FLOAT && dataset->getRank() == 3 && size[0] == nZ && size[1] == nY && size[2] == nX) {
                QString name = QString::fromStdString(dataset->getName());

                qDebug() << "----> 3D type dataset: " << name << "; size: " << size[0] << " x " << size[1] << " x " << size[2];

                if (!objects.contains(name)) {
                    objects.insert(name, new OpenedH5File::H5ObjectToVisualize(name, OpenedH5File::DATASET_TYPE, this));
                    objects[name]->addSubobject(dataset);
                } else {
                    objects[name]->addSubobject(dataset);
                }
            }
            // Downsampled 3D type
            else if (dataset->getDataType() == H5T_FLOAT && dataset->hasAttribute("dwnsmpl")) {
                QString name = QString::fromStdString(dataset->readAttributeS("src_dataset_name"));
                // TODO Check other attributes...

                qDebug() << "----> 3D type dataset (downsampled): " << name << "; size: " << size[0] << " x " << size[1] << " x " << size[2];

                if (!objects.contains(name)) {
                    objects.insert(name, new OpenedH5File::H5ObjectToVisualize(name, OpenedH5File::DATASET_TYPE, this));
                    objects[name]->addSubobject(dataset);
                } else {
                    objects[name]->addSubobject(dataset);
                }
            } else
                file->closeDataset(dataset->getName());

        } else if (type == H5G_GROUP) {
            // Reshaped mask type group
            try {
                HDF5File::HDF5Group *group = file->openGroup(i);
                // Downsampled reshaped mask type group
                if (group->hasAttribute("src_group_name") && group->hasAttribute("count")) {
                    // TODO Check other attributes...

                    QString name = QString::fromStdString(group->readAttributeS("src_group_name"));

                    qDebug() << "----> Reshaped mask type group (downsampled): "<< name;

                    if (!objects.contains(name)) {
                        objects.insert(name, new H5ObjectToVisualize(name, OpenedH5File::GROUP_TYPE, this));
                        objects[name]->addSubobject(group);
                    } else {
                        objects[name]->addSubobject(group);
                    }
                }
                // Original reshaped mask type group
                else if (group->hasAttribute("count")) {
                    QString name = QString::fromStdString(group->getName());

                    qDebug() << "----> Reshaped mask type group: "<< name;

                    if (!objects.contains(name)) {
                        objects.insert(name, new H5ObjectToVisualize(name, OpenedH5File::GROUP_TYPE, this));
                        objects[name]->addSubobject(group);
                    } else {
                        objects[name]->addSubobject(group);
                    }
                }
            } catch(std::exception &) {
                std::cout << "Object " << file->getObjNameById(i) << " is reshaped group" << std::endl;
                file->closeGroup(group->getName());
            }
        }
    }
}

QMap<QString, OpenedH5File::H5ObjectToVisualize *> OpenedH5File::getObjects()
{
    return objects;
}

OpenedH5File::H5ObjectToVisualize *OpenedH5File::getObject(QString mainName)
{
    if (objects.contains(mainName)) {
        return objects[mainName];
        //if (objects[name]->getNames().contains(name))
    } else
        return NULL;
}

OpenedH5File::H5ObjectToVisualize *OpenedH5File::getObjectBySubobjectName(QString name)
{
    foreach (QString key, objects.keys()) {
        if (objects.value(key)->getSubobjectNames().contains(name)) {
            objects.value(key)->setSelectedSubobject(name);
            return objects.value(key);
        }
    }
    return NULL;
}

void OpenedH5File::setSelectedSubobject(QString name)
{
    foreach (QString key, objects.keys()) {
        if (objects.value(key)->getSubobjectNames().contains(name)) {
            objects.value(key)->setSelectedSubobject(name);
        }
    }
}

void OpenedH5File::setObjectSelected(QString mainName, bool value)
{
    if (objects.contains(mainName)) {
        objects.value(mainName)->setSelected(value);
    }
}

void OpenedH5File::toogleObjectSelected(QString mainName)
{
    if (objects.contains(mainName)) {
        objects.value(mainName)->toogleSelected();
    }
}


OpenedH5File::~OpenedH5File()
{
    foreach (QString key, objects.keys())
        delete objects.value(key);
    delete file;
}

uint64_t OpenedH5File::getNT()
{
    return nT;
}

uint64_t OpenedH5File::getNX()
{
    return nX;
}

uint64_t OpenedH5File::getNY()
{
    return nY;
}

uint64_t OpenedH5File::getNZ()
{
    return nZ;
}

QMap<QString, QString> OpenedH5File::getInfo()
{
    return info;
}

HDF5File *OpenedH5File::getFile()
{
    return file;
}

QString OpenedH5File::getFilename()
{
    return QString::fromStdString(file->getFilename());
}

QString OpenedH5File::getRawFilename()
{
    std::string fileName = file->getFilename();
    size_t lastindex = fileName.find_last_of(".");
    std::string rawname = fileName.substr(0, lastindex);
    return QString::fromStdString(rawname);
}
