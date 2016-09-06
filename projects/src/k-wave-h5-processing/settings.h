#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <list>

#include <helper.h>
#include <hdf5helper.h>

#include <paramsdefinition.h>

class Settings
{
public:
    Settings();
    void loadParams(int argc, char **argv);
    void init();

    // Dataset names
    static const std::string SENSOR_MASK_TYPE_DATASET;
    static const std::string SENSOR_MASK_INDEX_DATASET;
    static const std::string SENSOR_MASK_CORNERS_DATASET;
    static const std::string NT_DATASET;
    static const std::string NX_DATASET;
    static const std::string NY_DATASET;
    static const std::string NZ_DATASET;

    std::string getSimulationOutputFilename();
    void setSimulationOutputFilename(const std::string &value);
    std::string getSimulationInputFilename();
    void setSimulationInputFilename(const std::string &value);
    std::string getOutputFilename();
    void setOutputFilename(const std::string &value);

    hsize_t getMaxSize();
    void setMaxSize(const hsize_t &value);
    hsize_t getMaxChunkSize();
    void setMaxChunkSize(const hsize_t &value);
    hsize_t getBlockSize();
    void setBlockSize(const hsize_t &value);

    std::list<std::string> getNames();
    void setNames(const std::list<std::string> &value);
    bool getFlagNames();
    void setFlagNames(bool value);

    bool getFlagReshape();
    void setFlagReshape(bool value);
    bool getFlagChangeChunks();
    void setFlagChangeChunks(bool value);
    bool getFlagDwnsmpl();
    void setFlagDwnsmpl(bool value);

    ParamsDefinition getParamsDefinition() const;

private:
    // Filenames
    std::string simulationOutputFilename;
    std::string simulationInputFilename;
    std::string outputFilename;

    // Size vars
    hsize_t maxSize;
    hsize_t maxChunkSize;
    hsize_t blockSize;

    // Filter by names
    std::list<std::string> names;
    bool flagNames;

    // Application modes
    bool flagReshape;
    bool flagRechunk;
    bool flagDwnsmpl;

    ParamsDefinition paramsDefinition;
};

#endif // SETTINGS_H
