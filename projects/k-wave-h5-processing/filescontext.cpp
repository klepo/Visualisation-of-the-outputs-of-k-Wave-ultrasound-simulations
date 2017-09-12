/**
 * @file        filescontext.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing FilesContext class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "filescontext.h"

/**
 * @brief Creates FilesContext object
 * @param[in] settings Processing settings
 */
FilesContext::FilesContext(Settings *settings)
{
    Helper::printDebugTitle("Loading of simulation output file");
    // Load simulation output file
    if (!settings->getSimulationOutputFilename().empty()) {
        simOutputFile = loadSimulationFile(settings->getSimulationOutputFilename());
        if (settings->getBlockSize() != 0)
            simOutputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    } else {
        Helper::printErrorMsg("Missing parameter -f (simulation output filename)");
        Helper::printDebugMsg(settings->getParamsDefinition().getHelp());
        exit(EXIT_FAILURE);
    }

    // Load simulation input file
    if (!settings->getSimulationInputFilename().empty()) {
        Helper::printDebugTitle("Loading of simulation input file");
        simInputFile = loadSimulationFile(settings->getSimulationInputFilename());
        if (settings->getBlockSize() != 0)
            simInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }

    // Create or open processing output file
    Helper::printDebugTitle("Create or open processing output file");
    resolveOutputFilename(settings);
    pcsOutputFile = createOrOpenOutputFile(outputFilename);

    // Load processing input file
    if (!settings->getProcessingInputFilename().empty()) {
        Helper::printDebugTitle("Loading of processing input file");
        pcsInputFile = loadSimulationFile(settings->getProcessingInputFilename());
        if (settings->getBlockSize() != 0)
            pcsInputFile->setNumberOfElmsToLoad(settings->getBlockSize());
    }
}

/**
 * @brief Destructor of Processing object
 *
 * Deletes opened files.
 */
FilesContext::~FilesContext()
{
    if (simOutputFile != pcsOutputFile) {
        if (pcsOutputFile) {
            delete pcsOutputFile;
            pcsOutputFile = 0;
        }
    }
    if (simOutputFile) {
        delete simOutputFile;
        simOutputFile = 0;
    }
    if (simInputFile) {
        delete simInputFile;
        simInputFile = 0;
    }
    if (pcsInputFile) {
        delete pcsInputFile;
        pcsInputFile = 0;
    }
}

/**
 * @brief Returns simulation output file
 * @return Simulation output file
 */
HDF5Helper::File *FilesContext::getSimOutputFile() const
{
    return simOutputFile;
}

/**
 * @brief Returns simulation input file
 * @return Simulation input file
 */
HDF5Helper::File *FilesContext::getSimInputFile() const
{
    return simInputFile;
}

/**
 * @brief Returns processing output file
 * @return Processing output file
 */
HDF5Helper::File *FilesContext::getPcsOutputFile()
{
    if (newEmptyOutputFileFlag)
        pcsOutputFile = createOrOpenOutputFile(outputFilename);
    return pcsOutputFile;
}

/**
 * @brief Returns processing input file
 * @return Processing input file
 */
HDF5Helper::File *FilesContext::getPcsInputFile() const
{
    return pcsInputFile;
}

/**
 * @brief Loads simulation file
 * @param[in] filename Filename
 * @return File
 */
HDF5Helper::File *FilesContext::loadSimulationFile(std::string filename)
{
    HDF5Helper::File *simulationFile = 0;
    try {
        simulationFile = new HDF5Helper::File(filename, HDF5Helper::File::OPEN);
    } catch (std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
    return simulationFile;
}

/**
 * @brief Resolves output filename
 * @param[in] settings Settings
 */
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
        //file = simOutputFile;
        //return simOutputFile;
    }
}

/**
 * @brief Creates or opens output file
 * @param[in] filename Filename
 * @return File
 */
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

    file->setNumberOfElmsToLoad(simOutputFile->getNumberOfElmsToLoad());

    Helper::printDebugTitle("Copy dimensions and root attributes to output file");

    // Copy nT, nX, nY, nZ
    std::cout << "Copy nT, nX, nY, nZ ... ";
    try {
        HDF5Helper::copyDataset(simOutputFile, file, HDF5Helper::NX_DATASET, true, true);
        HDF5Helper::copyDataset(simOutputFile, file, HDF5Helper::NY_DATASET, true, true);
        HDF5Helper::copyDataset(simOutputFile, file, HDF5Helper::NZ_DATASET, true, true);
        HDF5Helper::copyDataset(simOutputFile, file, HDF5Helper::NT_DATASET, true, true);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    // Copy root (info) attributes to destination h5 file
    std::cout << "Copy root (info) attributes ... ";
    try {
        HDF5Helper::Group *srcGroup = simOutputFile->openGroup("/", false);
        HDF5Helper::Group *dstGroup = file->openGroup("/", false);
        for (hsize_t i = 0; i < srcGroup->getNumAttrs(); i++) {
            HDF5Helper::Attribute *attr = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attr, false);
            delete attr;
        }
        simOutputFile->closeGroup(srcGroup, false);
        file->closeGroup(dstGroup, false);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    return file;
}
