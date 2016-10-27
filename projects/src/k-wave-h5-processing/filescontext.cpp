/**
 * @file        filescontext.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        8  September 2016 (created)
 *
 * @brief       The implementation file containing context for HDF5 files.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "filescontext.h"

FilesContext::FilesContext(Settings *settings)
{
    Helper::printDebugTitle("Loading of simulation output file");
    // Load simulation output file
    if (settings->getSimulationOutputFilename().empty()) {
        Helper::printErrorMsg("Missing parameter -f (simulation output filename)");
        Helper::printDebugMsg(settings->getParamsDefinition().getHelp());
        exit(EXIT_FAILURE);
    } else {
        hDF5SimOutputFile = loadSimulationFile(settings->getSimulationOutputFilename());
        hDF5SimOutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    if (!settings->getSimulationInputFilename().empty()) {
        // Load simulation input file
        Helper::printDebugTitle("Loading of simulation input file");
        hDF5SimInputFile = loadSimulationFile(settings->getSimulationInputFilename());
        hDF5SimInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    // Create new file
    Helper::printDebugTitle("Create or open output file");
    hDF5OutputFile = createOrOpenOutputFile(settings->getOutputFilename(), settings);
    hDF5OutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
}

FilesContext::~FilesContext()
{
    if (hDF5SimOutputFile != NULL) {
        delete hDF5SimOutputFile;
        hDF5SimOutputFile = NULL;
    }
    if (hDF5SimInputFile != NULL) {
        delete hDF5SimOutputFile;
        hDF5SimInputFile = NULL;
    }
    if (hDF5OutputFile != NULL) {
        delete hDF5SimOutputFile;
        hDF5SimOutputFile = 0;
    }
    if (hDF5ViewFile != NULL) {
        delete hDF5SimOutputFile;
        hDF5SimOutputFile = 0;
    }
}

HDF5Helper::File *FilesContext::getHDF5SimOutputFile() const
{
    return hDF5SimOutputFile;
}

HDF5Helper::File *FilesContext::getHDF5SimInputFile() const
{
    return hDF5SimInputFile;
}

HDF5Helper::File *FilesContext::getHDF5OutputFile() const
{
    return hDF5OutputFile;
}

HDF5Helper::File *FilesContext::getHDF5ViewFile() const
{
    return hDF5ViewFile;
}

HDF5Helper::File *FilesContext::loadSimulationFile(std::string simulationFilename)
{
    HDF5Helper::File *hDF5SimulationFile = NULL;
    try {
        hDF5SimulationFile = new HDF5Helper::File(simulationFilename, HDF5Helper::File::OPEN);
    } catch (std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
    return hDF5SimulationFile;
}

HDF5Helper::File *FilesContext::createOrOpenOutputFile(std::string outputFilename, Settings *settings)
{
    std::string filename = "";
    HDF5Helper::File *file = NULL;
    if (outputFilename.empty()) {
        // Create auto filename
        size_t lastindex = settings->getSimulationOutputFilename().find_last_of(".");
        std::string rawname = settings->getSimulationOutputFilename().substr(0, lastindex);
        filename = rawname + "_modified.h5";
    } else {
        filename = outputFilename;
    }

    try {
        // Try open file
        file = new HDF5Helper::File(filename, HDF5Helper::File::OPEN);
    } catch (std::exception) {
        try {
            // Try create file
            file = new HDF5Helper::File(filename, HDF5Helper::File::CREATE);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    Helper::printDebugTitle("Copy dimensions and root attributes to output file");

    // Copy nT, nX, nY, nZ
    try {
        HDF5Helper::HDF5Vector3D size(1, 1, 1);
        hsize_t data;
        data = hDF5SimOutputFile->getNT();
        file->createDatasetI(Settings::NT_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        file->openDataset(Settings::NT_DATASET)->writeDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), &data);
        data = hDF5SimOutputFile->getNX();
        file->createDatasetI(Settings::NX_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        file->openDataset(Settings::NX_DATASET)->writeDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), &data);
        data = hDF5SimOutputFile->getNY();
        file->createDatasetI(Settings::NY_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        file->openDataset(Settings::NY_DATASET)->writeDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), &data);
        data = hDF5SimOutputFile->getNZ();
        file->createDatasetI(Settings::NZ_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        file->openDataset(Settings::NZ_DATASET)->writeDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), &data);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Copy root (info) attributes to destination h5 file
    try {
        HDF5Helper::HDF5Group *srcGroup = hDF5SimOutputFile->openGroup("/");
        HDF5Helper::HDF5Group *dstGroup = file->openGroup("/");
        for (hsize_t i = 0; i < srcGroup->getNumAttrs(); i++) {
            HDF5Helper::HDF5Attribute *attr = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attr, false);
            delete attr;
        }
        hDF5SimOutputFile->closeGroup(srcGroup);
        file->closeGroup(dstGroup);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }


    return file;
}

void FilesContext::setHDF5ViewFile(HDF5Helper::File *value)
{
    hDF5ViewFile = value;
}
