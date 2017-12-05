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
H5Helper::File *FilesContext::getSimOutputFile() const
{
    return simOutputFile;
}

/**
 * @brief Returns simulation input file
 * @return Simulation input file
 */
H5Helper::File *FilesContext::getSimInputFile() const
{
    return simInputFile;
}

/**
 * @brief Returns processing output file
 * @return Processing output file
 */
H5Helper::File *FilesContext::getPcsOutputFile()
{
    if (newEmptyOutputFileFlag)
        pcsOutputFile = createOrOpenOutputFile(outputFilename);
    return pcsOutputFile;
}

/**
 * @brief Returns processing input file
 * @return Processing input file
 */
H5Helper::File *FilesContext::getPcsInputFile() const
{
    return pcsInputFile;
}

/**
 * @brief Loads simulation file
 * @param[in] filename Filename
 * @return File
 */
H5Helper::File *FilesContext::loadSimulationFile(std::string filename)
{
    H5Helper::File *simulationFile = 0;
    try {
        simulationFile = new H5Helper::File(filename, H5Helper::File::OPEN, Helper::enableDebugMsgs);
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
H5Helper::File *FilesContext::createOrOpenOutputFile(std::string filename)
{
    H5Helper::File *file = 0;
    if (!H5Helper::fileExists(filename)) {
        try {
            // Try create file
            if (newEmptyOutputFileFlag) {
                file = new H5Helper::File(filename, H5Helper::File::CREATE, Helper::enableDebugMsgs);
                newEmptyOutputFileFlag = false;
            } else {
                Helper::printDebugMsg("File \"" + filename + "\" will be created for the processing output");
                newEmptyOutputFileFlag = true;
                return 0;
            }
        } catch (std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        // Try open file
        file = loadSimulationFile(filename);
    }

    file->setNumberOfElmsToLoad(simOutputFile->getNumberOfElmsToLoad());

    Helper::printDebugTitle("Copy dimensions and root attributes to output file");

    // Copy nT, nX, nY, nZ
    Helper::printDebugMsgStart("Copy nT, nX, nY, nZ");
    try {
        H5Helper::copyDataset(simOutputFile, file, H5Helper::NX_DATASET, true, false);
        H5Helper::copyDataset(simOutputFile, file, H5Helper::NY_DATASET, true, false);
        H5Helper::copyDataset(simOutputFile, file, H5Helper::NZ_DATASET, true, false);
        H5Helper::copyDataset(simOutputFile, file, H5Helper::NT_DATASET, true, false);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Helper::printDebugMsg("OK");

    // Copy root (info) attributes to destination h5 file
    Helper::printDebugMsgStart("Copy root (info) attributes");
    try {
        H5Helper::Group *srcGroup = simOutputFile->openGroup("/", false);
        H5Helper::Group *dstGroup = file->openGroup("/", false);
        for (hsize_t i = 0; i < srcGroup->getNumAttrs(); i++) {
            H5Helper::Attribute *attribute = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attribute, false);
            delete attribute;
        }
        simOutputFile->closeGroup(srcGroup, false);
        file->closeGroup(dstGroup, false);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Helper::printDebugMsg("OK");

    return file;
}
