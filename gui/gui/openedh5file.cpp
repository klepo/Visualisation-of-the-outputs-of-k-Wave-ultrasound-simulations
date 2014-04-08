#include "openedh5file.h"
#include "h5objecttovisualize.h"

OpenedH5File::OpenedH5File(QString fileName, QObject *parent) :
    QObject(parent)
{
    // Open HDF5 file
    file = new HDF5File(fileName.toStdString());

    // Load dimensions
    nT = file->getNT();
    nX = file->getNX();
    nY = file->getNY();
    nZ = file->getNZ();

    // Load info
    HDF5File::HDF5Group *group = file->openGroup("/");
    for (int i = 0; i < group->getNumAttrs(); i++) {
        QString value((const char *) group->getAttribute(i)->getData());
        info.insert(QString::fromStdString(group->getAttribute(i)->getName()), value);
    }
    file->closeGroup("/");

    qRegisterMetaType<OpenedH5File::H5ObjectToVisualize *>("OpenedH5File::H5ObjectToVisualize");

    // Find datasets for visualization
    for (hsize_t i = 0; i < file->getNumObjs(); i++) {
        try {
            HDF5File::HDF5Dataset *dataset = file->openDataset(i);
            hsize_t *size = dataset->getDims();

            // 3D type
            if (dataset->getDataType() == H5T_FLOAT && dataset->getRank() == 3 && size[0] == nZ && size[1] == nY && size[2] == nX) {
                std::cout << "----> 3D type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                if (!objects.contains(QString::fromStdString(dataset->getName()))) {
                    objects.insert(QString::fromStdString(dataset->getName()), new OpenedH5File::H5ObjectToVisualize(QString::fromStdString(dataset->getName()), H5ObjectToVisualize::DATASET_TYPE, this));
                    objects[QString::fromStdString(dataset->getName())]->addName(QString::fromStdString(dataset->getName()));
                } else {
                    objects[QString::fromStdString(dataset->getName())]->addName(QString::fromStdString(dataset->getName()));
                }
            }
            // Downsampled 3D type
            else if (dataset->getDataType() == H5T_FLOAT) {
                try {
                    std::string name = dataset->readAttributeS("src_dataset_name");
                    dataset->readAttributeI("src_dataset_id");
                    std::cout << "----> 3D type dataset (downsampled): "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

                    if (!objects.contains(QString::fromStdString(name))) {
                        objects.insert(QString::fromStdString(dataset->getName()), new OpenedH5File::H5ObjectToVisualize(QString::fromStdString(name), H5ObjectToVisualize::DATASET_TYPE, this));
                        objects[QString::fromStdString(name)]->addName(QString::fromStdString(dataset->getName()));
                    } else {
                        objects[QString::fromStdString(name)]->addName(QString::fromStdString(dataset->getName()));
                    }
                } catch(std::exception &) {
                }
            }
            file->closeDataset(dataset->getName());
        } catch(std::exception &) {
            std::cout << "Object " << i << " is not dataset" << std::endl;
            // Reshaped mask type group
            try {
                HDF5File::HDF5Group *group = file->openGroup(i);
                // Downsampled reshaped mask type group
                try {
                    group->readAttributeI("src_group_id");
                    group->readAttributeI("count");
                    std::string name = group->readAttributeS("src_group_name");
                    std::cout << "----> Reshaped mask type group (downsampled): "<< group->getName() << std::endl;
                    if (!objects.contains(QString::fromStdString(name))) {
                        objects.insert(QString::fromStdString(group->getName()), new H5ObjectToVisualize(QString::fromStdString(name), H5ObjectToVisualize::GROUP_TYPE, this));
                        objects[QString::fromStdString(name)]->addName(QString::fromStdString(group->getName()));
                    } else {
                        objects[QString::fromStdString(name)]->addName(QString::fromStdString(group->getName()));
                    }
                }
                // Original reshaped mask type group
                catch(std::exception &) {
                    group->readAttributeI("count");
                    std::cout << "----> Reshaped mask type group: "<< group->getName() << std::endl;
                    if (!objects.contains(QString::fromStdString(group->getName()))) {
                        objects.insert(QString::fromStdString(group->getName()), new H5ObjectToVisualize(QString::fromStdString(group->getName()), H5ObjectToVisualize::GROUP_TYPE, this));
                        objects[QString::fromStdString(group->getName())]->addName(QString::fromStdString(group->getName()));
                    } else {
                        objects[QString::fromStdString(group->getName())]->addName(QString::fromStdString(group->getName()));
                    }
                }
            } catch(std::exception &) {
                std::cout << "Object " << i << " is not original reshaped group" << std::endl;
            }
        }
    }
}

QMap<QString, OpenedH5File::H5ObjectToVisualize *> OpenedH5File::getObjects()
{
    return objects;
}

OpenedH5File::~OpenedH5File()
{
    delete file;
    foreach (QString key, objects.keys())
        delete objects.value(key);
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
