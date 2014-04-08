#include "h5objecttovisualize.h"

OpenedH5File::H5ObjectToVisualize::H5ObjectToVisualize(QString _name, const int _type, OpenedH5File *_openedH5File, QObject *parent) : QObject(parent)
{
    openedH5File = _openedH5File;
    name = _name;
    type = _type;
    selectedDataset = NULL;
    selectedGroup = NULL;
}

void OpenedH5File::H5ObjectToVisualize::addName(QString name)
{
    names.append(name);
    selectedName = name;
}

QString OpenedH5File::H5ObjectToVisualize::getName()
{
    return name;
}

int OpenedH5File::H5ObjectToVisualize::getType()
{
    return type;
}

QList<QString> OpenedH5File::H5ObjectToVisualize::getNames()
{
    return names;
}

void OpenedH5File::H5ObjectToVisualize::selectObject(QString name)
{
    if (names.contains(name)) {
        if (type == DATASET_TYPE) {
            if (selectedDataset != NULL) openedH5File->file->closeDataset(selectedName.toStdString());
            selectedName = name;
            selectedDataset = openedH5File->file->openDataset(selectedName.toStdString());
        } else {
            if (selectedGroup != NULL) openedH5File->file->closeGroup(selectedName.toStdString());
            selectedName = name;
            selectedGroup = openedH5File->file->openGroup(selectedName.toStdString());
        }
    }
}
