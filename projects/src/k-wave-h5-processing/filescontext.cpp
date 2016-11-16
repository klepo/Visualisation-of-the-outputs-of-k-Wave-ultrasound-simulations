/**
 * @file        filescontext.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
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
        if (settings->getBlockSize() != 0)
            hDF5SimOutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    if (!settings->getSimulationInputFilename().empty()) {
        // Load simulation input file
        Helper::printDebugTitle("Loading of simulation input file");
        hDF5SimInputFile = loadSimulationFile(settings->getSimulationInputFilename());
        if (settings->getBlockSize() != 0)
            hDF5SimInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    // Create new file
    Helper::printDebugTitle("Create or open output file");
    hDF5PcsOutputFile = createOrOpenOutputFile(settings->getProcessingOutputFilename(), settings);
    if (settings->getBlockSize() != 0)
        hDF5PcsOutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
}

FilesContext::~FilesContext()
{
    if (hDF5SimOutputFile != 0) {
        delete hDF5SimOutputFile;
        hDF5SimOutputFile = 0;
    }
    if (hDF5SimInputFile != 0) {
        delete hDF5SimInputFile;
        hDF5SimInputFile = 0;
    }
    if (hDF5PcsOutputFile != 0) {
        delete hDF5PcsOutputFile;
        hDF5PcsOutputFile = 0;
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

HDF5Helper::File *FilesContext::getHDF5PcsOutputFile() const
{
    return hDF5PcsOutputFile;
}

HDF5Helper::File *FilesContext::loadSimulationFile(std::string simulationFilename)
{
    HDF5Helper::File *hDF5SimulationFile = 0;
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
    HDF5Helper::File *file = 0;
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
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::File::NX_DATASET);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::File::NY_DATASET);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::File::NZ_DATASET);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::File::NT_DATASET);
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
