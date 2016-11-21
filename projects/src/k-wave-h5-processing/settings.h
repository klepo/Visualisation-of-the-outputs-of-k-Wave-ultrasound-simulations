/**
 * @file        settings.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with Settings class declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <list>

#include <helper.h>
//#include <hdf5helper.h>

#include <paramsdefinition.h>

class Settings
{
public:
    Settings();
    void loadParams(int argc, char **argv);
    void init();

    // Simulation data files
    std::string getSimulationOutputFilename();
    void setSimulationOutputFilename(const std::string &value);
    std::string getSimulationInputFilename();
    void setSimulationInputFilename(const std::string &value);
    std::string getProcessingOutputFilename();
    void setProcessingOutputFilename(const std::string &value);

    // Sizes
    unsigned long long getMaxSize();
    void setMaxSize(const unsigned long long &value);
    unsigned long long getMaxChunkSize();
    void setMaxChunkSize(const unsigned long long &value);
    unsigned long long getBlockSize();
    void setBlockSize(const unsigned long long &value);

    // Period
    unsigned long long getPeriod() const;
    void setPeriod(unsigned long long value);

    // Selected names
    std::list<std::string> getNames();
    void setNames(const std::list<std::string> &value);
    bool getFlagNames();
    void setFlagNames(bool value);

    // Application modes
    bool getFlagReshape();
    void setFlagReshape(bool value);
    bool getFlagChangeChunks();
    void setFlagChangeChunks(bool value);
    bool getFlagDwnsmpl();
    void setFlagDwnsmpl(bool value);
    bool getFlagCompress() const;
    void setFlagCompress(bool value);
    bool getFlagDecompress() const;
    void setFlagDecompress(bool value);

    ParamsDefinition getParamsDefinition() const;

private:
    // Filenames
    std::string simulationOutputFilename = "";
    std::string simulationInputFilename = "";
    std::string processingOutputFilename = "";

    // Size vars
    unsigned long long maxSize = 512;
    unsigned long long maxChunkSize = 64;
    unsigned long long blockSize = 0;
    unsigned long long period = 0;

    // Filter/selection by names
    std::list<std::string> names;
    bool flagNames = false;

    // Application modes
    bool flagReshape = false;
    bool flagRechunk = false;
    bool flagDwnsmpl = false;
    bool flagCompress = false;
    bool flagDecompress = false;

    // Params definition
    ParamsDefinition paramsDefinition;
};

#endif // SETTINGS_H
