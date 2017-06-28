/**
 * @file        filescontext.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing context for HDF5 files.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "filescontext.h"

FilesContext::FilesContext(Settings *settings)
{
    Helper::printDebugTitle("Loading of simulation output file");
    // Load simulation output file
    if (!settings->getSimulationOutputFilename().empty()) {
        hDF5SimOutputFile = loadSimulationFile(settings->getSimulationOutputFilename());
        if (settings->getBlockSize() != 0)
            hDF5SimOutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    } else {
        Helper::printErrorMsg("Missing parameter -f (simulation output filename)");
        Helper::printDebugMsg(settings->getParamsDefinition().getHelp());
        exit(EXIT_FAILURE);
    }

    // Load simulation input file
    if (!settings->getSimulationInputFilename().empty()) {
        Helper::printDebugTitle("Loading of simulation input file");
        hDF5SimInputFile = loadSimulationFile(settings->getSimulationInputFilename());
        if (settings->getBlockSize() != 0)
            hDF5SimInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    // Create or open processing output file
    Helper::printDebugTitle("Create or open processing output file");
    resolveOutputFilename(settings);
    hDF5PcsOutputFile = createOrOpenOutputFile(outputFilename);

    // Load processing input file
    if (!settings->getProcessingInputFilename().empty()) {
        Helper::printDebugTitle("Loading of processing input file");
        hDF5PcsInputFile = loadSimulationFile(settings->getProcessingInputFilename());
        if (settings->getBlockSize() != 0)
            hDF5PcsInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }
}

FilesContext::~FilesContext()
{
    if (hDF5SimOutputFile == hDF5PcsOutputFile) {
        if (hDF5SimOutputFile) {
            delete hDF5SimOutputFile;
            hDF5SimOutputFile = 0;
        }
        if (hDF5SimInputFile) {
            delete hDF5SimInputFile;
            hDF5SimInputFile = 0;
        }
        if (hDF5PcsInputFile) {
            delete hDF5PcsInputFile;
            hDF5PcsInputFile = 0;
        }
    } else {
        if (hDF5SimOutputFile) {
            delete hDF5SimOutputFile;
            hDF5SimOutputFile = 0;
        }
        if (hDF5SimInputFile) {
            delete hDF5SimInputFile;
            hDF5SimInputFile = 0;
        }
        if (hDF5PcsOutputFile) {
            delete hDF5PcsOutputFile;
            hDF5PcsOutputFile = 0;
        }
        if (hDF5PcsInputFile) {
            delete hDF5PcsInputFile;
            hDF5PcsInputFile = 0;
        }
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

HDF5Helper::File *FilesContext::getHDF5PcsOutputFile()
{
    if (newEmptyOutputFileFlag)
        hDF5PcsOutputFile = createOrOpenOutputFile(outputFilename);
    return hDF5PcsOutputFile;
}

HDF5Helper::File *FilesContext::getHDF5PcsInputFile() const
{
    return hDF5PcsInputFile;
}

HDF5Helper::File *FilesContext::loadSimulationFile(std::string filename)
{
    HDF5Helper::File *hDF5SimulationFile = 0;
    try {
        hDF5SimulationFile = new HDF5Helper::File(filename, HDF5Helper::File::OPEN);
    } catch (std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
    return hDF5SimulationFile;
}

void FilesContext::resolveOutputFilename(Settings *settings)
{
    if (settings->getProcessingOutputFilename().empty()) {
        // Create auto filename
        size_t lastindex = settings->getSimulationOutputFilename().find_last_of(".");
        std::string rawname = settings->getSimulationOutputFilename().substr(0, lastindex);
        outputFilename = rawname + "_modified.h5";
    } else {
        outputFilename = settings->getProcessingOutputFilename();
    }

    if (outputFilename == settings->getSimulationOutputFilename()) {
        Helper::printDebugMsg("Simulation output file == processing output file");
        //file = hDF5SimOutputFile;
        //return hDF5SimOutputFile;
    }
}


HDF5Helper::File *FilesContext::createOrOpenOutputFile(std::string filename)
{
    HDF5Helper::File *file = 0;
    if (!HDF5Helper::fileExists(filename)) {
        try {
            // Try create file
            if (newEmptyOutputFileFlag) {
                file = new HDF5Helper::File(filename, HDF5Helper::File::CREATE);
                newEmptyOutputFileFlag = false;
            } else {
                std::cout << "File \"" << filename << "\" will be created for the processing output" << std::endl;
                newEmptyOutputFileFlag = true;
                return 0;
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    } else {
        try {
            // Try open file
            file = new HDF5Helper::File(filename, HDF5Helper::File::OPEN);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    file->setNumberOfElmsToLoad(hDF5SimOutputFile->getNumberOfElmsToLoad());

    Helper::printDebugTitle("Copy dimensions and root attributes to output file");

    // Copy nT, nX, nY, nZ
    std::cout << "Copy nT, nX, nY, nZ ... ";
    try {
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::NX_DATASET, true, true);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::NY_DATASET, true, true);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::NZ_DATASET, true, true);
        HDF5Helper::copyDataset(hDF5SimOutputFile, file, HDF5Helper::NT_DATASET, true, true);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    // Copy root (info) attributes to destination h5 file
    std::cout << "Copy root (info) attributes ... ";
    try {
        HDF5Helper::HDF5Group *srcGroup = hDF5SimOutputFile->openGroup("/", false);
        HDF5Helper::HDF5Group *dstGroup = file->openGroup("/", false);
        for (hsize_t i = 0; i < srcGroup->getNumAttrs(); i++) {
            HDF5Helper::HDF5Attribute *attr = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attr, false);
            delete attr;
        }
        hDF5SimOutputFile->closeGroup(srcGroup, false);
        file->closeGroup(dstGroup, false);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    return file;
}
