/**
 * @file        settings.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with Settings class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#include <iomanip>

#include <helper.h>

#include <paramsdefinition.h>

/**
 * @brief The Settings class represents wrapper for processing settings
 */
class Settings
{
public:
    Settings();
    void loadParams(int argc, char **argv);

    // Simulation data files
    std::string getSimulationOutputFilename() const;
    void setSimulationOutputFilename(const std::string &value);
    std::string getSimulationInputFilename() const;
    void setSimulationInputFilename(const std::string &value);
    std::string getProcessingOutputFilename() const;
    void setProcessingOutputFilename(const std::string &value);
    std::string getProcessingInputFilename() const;
    void setProcessingInputFilename(const std::string &value);

    // Sizes
    unsigned long long getMaxSize() const;
    void setMaxSize(const unsigned long long &value);
    unsigned long long getMaxChunkSize() const;
    void setMaxChunkSize(const unsigned long long &value);
    unsigned long long getBlockSize() const;
    void setBlockSize(const unsigned long long &value);
    unsigned long long getMOS() const;
    void setMOS(const unsigned long long &value);

    // Period
    unsigned long long getPeriod() const;
    void setPeriod(const unsigned long long &value);

    // Harmonic
    unsigned long long getHarmonic() const;
    void setHarmonic(const unsigned long long &value);

    // Selected names
    std::list<std::string> getNames() const;
    void setNames(const std::list<std::string> &value);
    bool getFlagNames() const;
    void setFlagNames(bool value);

    // Application modes
    bool getFlagReshape() const;
    void setFlagReshape(bool value);
    bool getFlagChangeChunks() const;
    void setFlagChangeChunks(bool value);
    bool getFlagDwnsmpl() const;
    void setFlagDwnsmpl(bool value);
    bool getFlagCompress() const;
    void setFlagCompress(bool value);
    bool getFlagDecompress() const;
    void setFlagDecompress(bool value);
    bool getFlagDifference() const;
    void setFlagDifference(bool value);
    bool getFlagInfo() const;
    void setFlagInfo(bool value);
    bool getFlagComputePeriod() const;
    void setFlagComputePeriod(bool value);
    bool getFlagFindMinMax() const;
    void setFlagFindMinMax(bool value);

    ParamsDefinition getParamsDefinition() const;

private:
    // Filenames
    std::string simulationOutputFilename = "";
    std::string simulationInputFilename = "";
    std::string processingOutputFilename = "";
    std::string processingInputFilename = "";

    // Size vars
    unsigned long long maxSize = 512;
    unsigned long long maxChunkSize = 64;
    unsigned long long blockSize = 0;
    unsigned long long period = 0;
    unsigned long long harmonic = 1;
    unsigned long long mOS = 1;

    // Filter/selection by names
    std::list<std::string> names;
    bool flagNames = false;

    // Application modes
    bool flagReshape = false;
    bool flagRechunk = false;
    bool flagDwnsmpl = false;
    bool flagCompress = false;
    bool flagDecompress = false;
    bool flagDifference = false;
    bool flagInfo = false;
    bool flagComputePeriod = false;
    bool flagFindMinMax = false;

    // Params definition
    ParamsDefinition paramsDefinition;
};

#endif // SETTINGS_H
