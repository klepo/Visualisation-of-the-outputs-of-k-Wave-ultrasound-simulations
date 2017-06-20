/**
 * @file        filescontext.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with FilesContext class declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

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
    HDF5Helper::File *getHDF5PcsOutputFile() const;
    HDF5Helper::File *getHDF5PcsInputFile() const;

private:
    HDF5Helper::File *loadSimulationFile(std::string simulationFilename);
    HDF5Helper::File *createOrOpenOutputFile(std::string outputFilename, Settings *settings);
    HDF5Helper::File *hDF5SimOutputFile = 0;
    HDF5Helper::File *hDF5SimInputFile = 0;
    HDF5Helper::File *hDF5PcsOutputFile = 0;
    HDF5Helper::File *hDF5PcsInputFile = 0;

private:
    // Disable copy
    FilesContext(const FilesContext &);
};

#endif // FILESCONTEXT_H
