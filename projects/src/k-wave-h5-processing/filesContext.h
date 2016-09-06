#ifndef FILESCONTEXT_H
#define FILESCONTEXT_H

#include <hdf5helper.h>
#include <helper.h>
#include <settings.h>

class FilesContext
{
public:
    FilesContext(Settings *settings);
    ~FilesContext();

    HDF5Helper::File *getHDF5SimOutputFile() const;
    HDF5Helper::File *getHDF5SimInputFile() const;
    HDF5Helper::File *getHDF5OutputFile() const;
    HDF5Helper::File *getHDF5ViewFile() const;
    void setHDF5ViewFile(HDF5Helper::File *value);

private:
    HDF5Helper::File *loadSimulationFile(std::string simulationFilename);
    HDF5Helper::File *createOrOpenOutputFile(std::string outputFilename, Settings *settings);
    HDF5Helper::File *hDF5SimOutputFile = NULL;
    HDF5Helper::File *hDF5SimInputFile = NULL;
    HDF5Helper::File *hDF5OutputFile = NULL;
    HDF5Helper::File *hDF5ViewFile = NULL;

private:
    // Disable copy
    FilesContext(const FilesContext &);
};

#endif // FILESCONTEXT_H
