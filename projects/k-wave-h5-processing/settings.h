/**
 * @file        settings.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The header file with Settings class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <algorithm>

#include "paramsdefinition.h"
#include "helper.h"

/**
 * @brief The Settings class represents wrapper for processing settings
 */
class Settings
{
public:
    Settings();
    Settings(int argc, const char **argv);
    void loadParams(int argc, const char **argv);

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
    ParamsDefinition::VectorOfULongLongs getMaxChunkSizes() const;
    void setMaxChunkSizes(const ParamsDefinition::VectorOfULongLongs &value);
    unsigned long long getBlockSize() const;
    void setBlockSize(const unsigned long long &value);
    unsigned long long getMOS() const;
    void setMOS(const unsigned long long &value);

    unsigned long long getMaxChunkSizeX() const;
    unsigned long long getMaxChunkSizeY() const;
    unsigned long long getMaxChunkSizeZ() const;
    unsigned long long getMaxChunkSizeW() const;
    unsigned long long getMaxChunkSizeT() const;

    // Period or frequency
    float getPeriod() const;
    void setPeriod(const float &value);
    float getFrequency() const;
    void setFrequency(const float &value);

    // Harmonic
    unsigned long long getHarmonics() const;
    void setHarmonic(const unsigned long long &value);

    // Selected names
    ParamsDefinition::ListOfStrings getNames() const;
    void setNames(const ParamsDefinition::ListOfStrings &value);
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
    bool getFlagLog() const;
    void setFlagLog(bool value);

    ParamsDefinition getParamsDefinition() const;

private:
    // Filenames
    /// Simulation output filename
    std::string simulationOutputFilename = "";
    /// Simulation input filename
    std::string simulationInputFilename = "";
    /// Processing output filename
    std::string processingOutputFilename = "";
    /// Processing input filename
    std::string processingInputFilename = "";

    // Size vars
    /// Maximal size
    unsigned long long maxSize = 512;
    /// Block size
    unsigned long long blockSize = 0;
    /// Period
    float period = 0.0f;
    /// Input frequency
    float frequency = 0.0f;
    /// Number of harmonics
    unsigned long long harmonics = 1;
    /// Multiple of overlap size
    unsigned long long mOS = 1;

    /// Maximal chunk sizes
    ParamsDefinition::VectorOfULongLongs maxChunkSizes = ParamsDefinition::VectorOfULongLongs{64, 64, 64, 1};
    /// Maximal chunk sizes flag
    bool flagMaxChunkSizes = false;

    /// Selection by names
    ParamsDefinition::ListOfStrings names;
    /// Selection by names flag
    bool flagNames = false;

    // Application modes
    /// Reshape flag
    bool flagReshape = false;
    /// Rechunk flag
    bool flagRechunk = false;
    /// Downsampling flag
    bool flagDwnsmpl = false;
    /// Compression flag
    bool flagCompress = false;
    /// Decompression flag
    bool flagDecompress = false;
    /// Difference flag
    bool flagDifference = false;
    /// Info flag
    bool flagInfo = false;
    /// Compute period flag
    bool flagComputePeriod = false;
    /// Minimal and maximal flag
    bool flagFindMinMax = false;
    /// Logging flag
    bool flagLog = false;

    /// Params definition
    ParamsDefinition paramsDefinition;
};

#endif // SETTINGS_H
