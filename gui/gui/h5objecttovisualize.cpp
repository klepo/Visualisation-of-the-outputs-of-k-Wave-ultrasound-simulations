#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"

OpenedH5File::H5ObjectToVisualize::H5ObjectToVisualize(QString name, const int type, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = name;
    this->type = type;

    selectedSubobject = NULL;
    selected = false;
}

OpenedH5File::H5ObjectToVisualize::~H5ObjectToVisualize()
{

}

void OpenedH5File::H5ObjectToVisualize::addSubobject(HDF5File::HDF5Dataset *dataset)
{
    if (type == OpenedH5File::DATASET_TYPE && !subobjects.contains(QString::fromStdString(dataset->getName()))) {
        H5SubobjectToVisualize *subobject = new H5SubobjectToVisualize(dataset, openedH5File);
        subobjects.insert(QString::fromStdString(dataset->getName()), subobject);
        selectedSubobject = subobject;
    }
}

void OpenedH5File::H5ObjectToVisualize::addSubobject(HDF5File::HDF5Group *group)
{
    if (type == OpenedH5File::GROUP_TYPE && !subobjects.contains(QString::fromStdString(group->getName()))) {
        H5SubobjectToVisualize *subobject = new H5SubobjectToVisualize(group, openedH5File);
        subobjects.insert(QString::fromStdString(group->getName()), subobject);
        selectedSubobject = subobject;
    }
}

/*HDF5File::HDF5Dataset *OpenedH5File::H5ObjectToVisualize::getSelectedDataset()
{
    if ((type == OpenedH5File::DATASET_TYPE || type == GROUP_TYPE ) && selectedSubobject != NULL) {
        return selectedSubobject->getDataset();
    } else {
        return NULL;
    }
}

HDF5File::HDF5Group *OpenedH5File::H5ObjectToVisualize::getSelectedGroup()
{
    if (type = GROUP_TYPE && selectedSubobject != NULL) {
        return selectedSubobject->getGroup();
    } else {
        return NULL;
    }
}*/

QString OpenedH5File::H5ObjectToVisualize::getName()
{
    return name;
}

int OpenedH5File::H5ObjectToVisualize::getType()
{
    return type;
}

QList<QString> OpenedH5File::H5ObjectToVisualize::getSubobjectNames()
{
    return subobjects.keys();
}

QMap<QString, OpenedH5File::H5SubobjectToVisualize *> OpenedH5File::H5ObjectToVisualize::getSubobjects()
{
    return subobjects;
}

OpenedH5File::H5SubobjectToVisualize *OpenedH5File::H5ObjectToVisualize::getSelectedSubobject()
{
    return selectedSubobject;
}


void OpenedH5File::H5ObjectToVisualize::setSelectedSubobject(QString name)
{
    if (subobjects.contains(name)) {
        selectedSubobject = subobjects.value(name);
    }
}

/*QString OpenedH5File::H5ObjectToVisualize::getSelectedSubobjectName()
{
    return selectedSubobject->getName();
}*/

void OpenedH5File::H5ObjectToVisualize::toogleSelected()
{
    if (selected)
        selected = false;
    else
        selected = true;
}

void OpenedH5File::H5ObjectToVisualize::setSelected(bool value)
{
    selected = value;
}

