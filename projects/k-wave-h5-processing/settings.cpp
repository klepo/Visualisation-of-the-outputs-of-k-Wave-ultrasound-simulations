/**
 * @file        settings.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              23 October   2018 (updated)
 *
 * @brief       The implementation file containing Settings class definition.
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

#include "settings.h"

/**
 * @brief Creates Settings object
 */
Settings::Settings()
{

}

Settings::Settings(int argc, char **argv)
{
    loadParams(argc, argv);
}

/**
 * @brief Loads params
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 */
void Settings::loadParams(int argc, char **argv)
{
    // Define expected params
    // Modes
    paramsDefinition.defineParamsFlag("help");
    paramsDefinition.defineParamsFlag("reshape");
    paramsDefinition.defineParamsFlag("changeChunks");
    paramsDefinition.defineParamsFlag("dwnsmpl");
    paramsDefinition.defineParamsFlag("compress");
    paramsDefinition.defineParamsFlag("decompress");
    paramsDefinition.defineParamsFlag("difference");
    paramsDefinition.defineParamsFlag("info");
    paramsDefinition.defineParamsFlag("computePeriod");
    paramsDefinition.defineParamsFlag("findMinMax");
    paramsDefinition.defineParamsFlag("log");

    // Size
    paramsDefinition.defineParamsFlag("s", ParamsDefinition::ULONGLONG);

    // Chunk sizes
    paramsDefinition.defineParamsFlag("ch", ParamsDefinition::ULONGLONG_SEPARATED);

    // Block size
    paramsDefinition.defineParamsFlag("c", ParamsDefinition::ULONGLONG);

    // Period
    paramsDefinition.defineParamsFlag("p", ParamsDefinition::ULONGLONG);

    // Harmonics
    paramsDefinition.defineParamsFlag("h", ParamsDefinition::ULONGLONG);

    // Multiple of overlap size
    paramsDefinition.defineParamsFlag("mos", ParamsDefinition::ULONGLONG);

    // Names
    paramsDefinition.defineParamsFlag("names", ParamsDefinition::STRINGS_SEPARATED);

    // HDF5 simulation output filename
    paramsDefinition.defineParamsFlag("f", ParamsDefinition::STRING);

    // HDF5 simulation input filename
    paramsDefinition.defineParamsFlag("m", ParamsDefinition::STRING);

    // HDF5 processing output filename
    paramsDefinition.defineParamsFlag("o", ParamsDefinition::STRING);

    // HDF5 processing input filename with decompressed datasets
    paramsDefinition.defineParamsFlag("d", ParamsDefinition::STRING);

    // Help message
    paramsDefinition.setHelp("\n"
                             "Usage: k-wave-h5-processing [options]\n"
                             "where options include:\n\n"
                             "  -f HDF5SimulationOutputFilename ....... Required parameter.\n"
                             "                                          HDF5 file with simulation results.\n"
                             "\n"
                             "  -m HDF5SimulationInputFilename ........ Optional parameter. HDF5 simulation input filename \n"
                             "                                          (with sensor_mask_index or sensor_mask_corners\n"
                             "                                          or p_source_input dataset).\n"
                             "\n"
                             "  -o HDF5ProcessingOutputFilename ....... Optional parameter. HDF5 processing output filename. \n"
                             "                                          Default is HDF5SimulationOutputFilename + \"_modified.h5\".\n"
                             "\n"
                             "  -d HDF5ProcessingInputFilename ........ Optional parameter. HDF5 processing input filename for \n"
                             "                                          reading decompressed datasets from a separate file.\n"
                             "\n"
                             "  -reshape .............................. Optional parameter. Performs processing sensor mask\n"
                             "                                          type datasets to group with 4D datasets and saves datasets\n"
                             "                                          to the output file. In HDF5SimulationOutputFilename or \n"
                             "                                          HDF5SimulationInputFilename must be sensor_mask_index or\n"
                             "                                          sensor_mask_corners dataset.\n"
                             "\n"
                             "  -changeChunks ......................... Optional parameter. Sets a new chunks size of\n"
                             "                                          datasets and saves datasets to the output file.\n"
                             "\n"
                             "  -dwnsmpl .............................. Optional parameter. Performs downsampling of datasets\n"
                             "                                          and saves them to the output file.\n"
                             "\n"
                             "  -compress ............................. Optional parameter. Performs compression of time series\n"
                             "                                          dataset data, creates new dataset with coefficients (e.g p_c).\n"
                             "\n"
                             "  -decompress ........................... Optional parameter. Performs decompression of time series\n"
                             "                                          dataset data, needs dataset with coefficients (e.g p_c).\n"
                             "\n"
                             "  -difference ........................... Optional parameter. Performs subtraction of time series\n"
                             "                                          dataset data, needs original dataset and decoded (name_d).\n"
                             "\n"
                             "  -s size ............................... Optional parameter. Max size for downsampling.\n"
                             "                                          Default size is 512.\n"
                             "\n"
                             "  -ch chunkSize;chunkSize;... ........... Optional parameter. The sizes for new chunks (x;y;z;t;) from\n"
                             "                                          1 to maximal appropriately value. Default sizes are 64;64;64;1.\n"
                             "\n"
                             "  -c blockSize .......................... Optional parameter. Sets number of data elements\n"
                             "                                          for block reading. Default value is based on available\n"
                             "                                          system physical memory.\n"
                             "\n"
                             "  -p period ............................. Optional parameter. Sets period of input signal for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -h harmonics........................... Optional parameter. Sets multiple of harmonic frequency for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -mos size ............................. Optional parameter. Sets multiple of overlap size for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -names name1;name2;... ................ Optional parameter. Names of selected datasets or groups\n"
                             "                                          to processing.\n"
                             "\n"
                             "  -info ................................. Prints the values of attributes of selected datasets.\n"
                             "\n"
                             "  -computePeriod ........................ Computes period from p_source_input.\n"
                             "\n"
                             "  -findMinMax ........................... Finds and stores minimal and maximal values and their\n"
                             "                                          indices of known datasets from input files.\n"
                             "\n"
                             "  -help ................................. Prints this help message.\n"
                             "\n");

    // Parse params from command line
    //Helper::printDebugMsg("");
    try {
        paramsDefinition.commandLineParse(argc, argv);
    } catch (std::exception &e) {
        Helper::printErrorMsg("  Wrong parameter " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }

    // Set flags according to params
    ParamsDefinition::Flags flags = paramsDefinition.getFlags();

    if (flags.at("help").getEnabled()) {
        Helper::printDebugMsg(paramsDefinition.getHelp());
        exit(EXIT_SUCCESS);
    }

    Helper::enableDebugMsgs = flags.at("log").getEnabled();
    setFlagLog(flags.at("log").getEnabled());

    setFlagReshape(flags.at("reshape").getEnabled());
    setFlagChangeChunks(flags.at("changeChunks").getEnabled());
    setFlagDwnsmpl(flags.at("dwnsmpl").getEnabled());
    setFlagCompress(flags.at("compress").getEnabled());
    setFlagDecompress(flags.at("decompress").getEnabled());
    setFlagDifference(flags.at("difference").getEnabled());
    setFlagInfo(flags.at("info").getEnabled());
    setFlagComputePeriod(flags.at("computePeriod").getEnabled());
    setFlagFindMinMax(flags.at("findMinMax").getEnabled());

    setFlagNames(flags.at("names").getEnabled());

    if (flags.at("names").getEnabled()) {
        ParamsDefinition::ListOfStrings names;
        flags.at("names").getParams().readParam(0, &names);
        setNames(names);
    }

    if (flags.at("f").getEnabled()) {
        std::string simulationOutputFilename;
        flags.at("f").getParams().readParam(0, &simulationOutputFilename);
        setSimulationOutputFilename(simulationOutputFilename);
    }

    if (flags.at("m").getEnabled()) {
        std::string simulationInputFilename;
        flags.at("m").getParams().readParam(0, &simulationInputFilename);
        setSimulationInputFilename(simulationInputFilename);
    }

    if (flags.at("o").getEnabled()) {
        std::string processingOutputFilename;
        flags.at("o").getParams().readParam(0, &processingOutputFilename);
        setProcessingOutputFilename(processingOutputFilename);
    }

    if (flags.at("d").getEnabled()) {
        std::string processingInputFilename;
        flags.at("d").getParams().readParam(0, &processingInputFilename);
        setProcessingInputFilename(processingInputFilename);
    }

    if (flags.at("s").getEnabled()) {
        unsigned long long maxSize;
        flags.at("s").getParams().readParam(0, &maxSize);
        setMaxSize(maxSize);
    }

    if (flags.at("ch").getEnabled()) {
        ParamsDefinition::VectorOfULongLongs sizes;
        flags.at("ch").getParams().readParam(0, &sizes);
        setMaxChunkSizes(sizes);
    }

    if (flags.at("c").getEnabled()) {
        unsigned long long blockSize;
        flags.at("c").getParams().readParam(0, &blockSize);
        setBlockSize(blockSize);
    }

    if (flags.at("p").getEnabled()) {
        unsigned long long period;
        flags.at("p").getParams().readParam(0, &period);
        setPeriod(period);
    }

    if (flags.at("h").getEnabled()) {
        unsigned long long harmonic;
        flags.at("h").getParams().readParam(0, &harmonic);
        setHarmonic(harmonic);
    }

    if (flags.at("mos").getEnabled()) {
        unsigned long long mOs;
        flags.at("mos").getParams().readParam(0, &mOs);
        setMOS(mOs);
    }
}

/**
 * @brief Returns simulation output filename
 * @return Simulation output filename
 */
std::string Settings::getSimulationOutputFilename() const
{
    return simulationOutputFilename;
}

/**
 * @brief Sets simulation output filename
 * @param[in] value Filename
 */
void Settings::setSimulationOutputFilename(const std::string &value)
{
    simulationOutputFilename = value;
    Helper::printDebugTwoColumns2S("Simulation output filename", simulationOutputFilename, 30);
}

/**
 * @brief Returns simulation input filename
 * @return Simulation input filename
 */
std::string Settings::getSimulationInputFilename() const
{
    return simulationInputFilename;
}

/**
 * @brief Sets simulation input filename
 * @param[in] value Filename
 */
void Settings::setSimulationInputFilename(const std::string &value)
{
    simulationInputFilename = value;
    Helper::printDebugTwoColumns2S("Simulation input filename", simulationInputFilename, 30);
}

/**
 * @brief Returns processing output filename
 * @return Processing output filename
 */
std::string Settings::getProcessingOutputFilename() const
{
    return processingOutputFilename;
}

/**
 * @brief Sets processing output filename
 * @param[in] value Filename
 */
void Settings::setProcessingOutputFilename(const std::string &value)
{
    processingOutputFilename = value;
    Helper::printDebugTwoColumns2S("Processing output filename", processingOutputFilename, 30);
}

/**
 * @brief Returns processing input filename
 * @return Processing input filename
 */
std::string Settings::getProcessingInputFilename() const
{
    return processingInputFilename;
}

/**
 * @brief Sets processing input filename
 * @param[in] value Filename
 */
void Settings::setProcessingInputFilename(const std::string &value)
{
    processingInputFilename = value;
    Helper::printDebugTwoColumns2S("Processing input filename", processingInputFilename, 30);
}

/**
 * @brief Returns max size for downsampling
 * @return Max size for downsampling
 */
unsigned long long Settings::getMaxSize() const
{
    return maxSize;
}

/**
 * @brief Sets max size
 * @param[in] value Max size
 */
void Settings::setMaxSize(const unsigned long long &value)
{
    maxSize = value;
    Helper::printDebugTwoColumns2S("Max size for downsampling", maxSize, 30);
}

/**
 * @brief Returns max chunk size
 * @return Max chunk size
 */
ParamsDefinition::VectorOfULongLongs Settings::getMaxChunkSizes() const
{
    return maxChunkSizes;
}

/**
 * @brief Sets max chunk size
 * @param[in] value Max chunk size
 */
void Settings::setMaxChunkSizes(const ParamsDefinition::VectorOfULongLongs &value)
{
    maxChunkSizes = value;
    for (size_t i = 0; i < value.size(); i++) {
        maxChunkSizes[i] = value[i];
    }
    std::string sizesString;
    for (ParamsDefinition::VectorOfULongLongs::const_iterator ci = maxChunkSizes.begin(); ci != maxChunkSizes.end(); ++ci) {
        sizesString +=  std::to_string(*ci);
        if (std::next(ci) != maxChunkSizes.end())
            sizesString += " x ";
    }
    Helper::printDebugTwoColumns2S("New chunk sizes", sizesString, 30);
}

/**
 * @brief Returns block size
 * @return Block size
 */
unsigned long long Settings::getBlockSize() const
{
    return blockSize;
}

/**
 * @brief Sets max size for block reading
 * @param[in] value Max size for block reading
 */
void Settings::setBlockSize(const unsigned long long &value)
{
    blockSize = value;
    Helper::printDebugTwoColumns2S("Max size for block reading", blockSize, 30);
}

/**
 * @brief Returns multiple of overlapping size
 * @return Multiple of overlapping size
 */
unsigned long long Settings::getMOS() const
{
    return mOS;
}

/**
 * @brief Sets multiple of overlap size
 * @param[in] value Multiple of overlap size
 */
void Settings::setMOS(const unsigned long long &value)
{
    mOS = value;
    Helper::printDebugTwoColumns2S("Multiple of overlap size", mOS, 30);
}

unsigned long long Settings::getMaxChunkSizeX() const
{
    return maxChunkSizes.at(0);
}

unsigned long long Settings::getMaxChunkSizeY() const
{
    return maxChunkSizes.at(1);
}

unsigned long long Settings::getMaxChunkSizeZ() const
{
    return maxChunkSizes.at(2);
}

unsigned long long Settings::getMaxChunkSizeW() const
{
    return maxChunkSizes.at(3);
}

unsigned long long Settings::getMaxChunkSizeT() const
{
    return maxChunkSizes.at(3);
}

/**
 * @brief Returns period
 * @return Period
 */
unsigned long long Settings::getPeriod() const
{
    return period;
}

/**
 * @brief Sets period for compression
 * @param[in] value Period for compression
 */
void Settings::setPeriod(const unsigned long long &value)
{
    period = value;
    Helper::printDebugTwoColumns2S("Period for compression", period, 30);
}

/**
 * @brief Returns multiple of harmonic frequency for compression
 * @return Harmonic
 */
unsigned long long Settings::getHarmonics() const
{
    return harmonic;
}

/**
 * @brief Sets multiple of harmonic frequency for compression
 * @param[in] value Multiple of harmonic frequency for compression
 */
void Settings::setHarmonic(const unsigned long long &value)
{
    harmonic = value;
    Helper::printDebugTwoColumns2S("Number of harmonics", harmonic, 30);
}

/**
 * @brief Returns selected datasets or groups names
 * @return Selected datasets or groups names
 */
ParamsDefinition::ListOfStrings Settings::getNames() const
{
    return names;
}

/**
 * @brief Settings::setNames
 * @param[in] value Selected datasets or groups names
 */
void Settings::setNames(const ParamsDefinition::ListOfStrings &value)
{
    names = value;
    std::string namesString;
    for (ParamsDefinition::ListOfStrings::const_iterator ci = value.begin(); ci != value.end(); ++ci) {
        namesString +=  *ci;
        namesString += ", ";
    }
    Helper::printDebugTwoColumns2S("Selected datasets or groups", namesString, 30);
}

/**
 * @brief Returns names flag
 * @return Names flag
 */
bool Settings::getFlagNames() const
{
    return flagNames;
}

/**
 * @brief Sets flag names
 * @param[in] value Flag names
 */
void Settings::setFlagNames(bool value)
{
    flagNames = value;
}

/**
 * @brief Returns reshape flag
 * @return Reshape flag
 */
bool Settings::getFlagReshape() const
{
    return flagReshape;
}

/**
 * @brief Sets reshape mode flag
 * @param[in] value Reshape mode flag
 */
void Settings::setFlagReshape(bool value)
{
    flagReshape = value;
    if (value)
        Helper::printDebugTwoColumns2S("Reshape mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Reshape mode", "OFF");
}

/**
 * @brief Returns change chunks mode flag
 * @return Change chunks mode flag
 */
bool Settings::getFlagChangeChunks() const
{
    return flagRechunk;
}

/**
 * @brief Sets change chunks mode flag
 * @param[in] value Change chunks mode flag
 */
void Settings::setFlagChangeChunks(bool value)
{
    flagRechunk = value;
    if (value)
        Helper::printDebugTwoColumns2S("Change chunks mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Change chunks mode", "OFF");
}

/**
 * @brief Returns downsampling mode flag
 * @return Downsampling mode flag
 */
bool Settings::getFlagDwnsmpl() const
{
    return flagDwnsmpl;
}

/**
 * @brief Sets downsampling mode flag
 * @param[in] value Downsampling mode flag
 */
void Settings::setFlagDwnsmpl(bool value)
{
    flagDwnsmpl = value;
    if (value)
        Helper::printDebugTwoColumns2S("Downsampling mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Downsampling mode", "OFF");
}

/**
 * @brief Returns compression mode flag
 * @return Compression mode flag
 */
bool Settings::getFlagCompress() const
{
    return flagCompress;
}

/**
 * @brief Sets compression mode flag
 * @param[in] value Compression mode flag
 */
void Settings::setFlagCompress(bool value)
{
    flagCompress = value;
    if (value)
        Helper::printDebugTwoColumns2S("Compression mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Compression mode", "OFF");
}

/**
 * @brief Returns decompression flag
 * @return Decompression mode flag
 */
bool Settings::getFlagDecompress() const
{
    return flagDecompress;
}

/**
 * @brief Sets decompression mode flag
 * @param[in] value Decompression mode flag
 */
void Settings::setFlagDecompress(bool value)
{
    flagDecompress = value;
    if (value)
        Helper::printDebugTwoColumns2S("Decompression mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Decompression mode", "OFF");
}

/**
 * @brief Returns difference mode flag
 * @return Difference mode flag
 */
bool Settings::getFlagDifference() const
{
    return flagDifference;
}

/**
 * @brief Sets difference mode flag
 * @param[in] value Difference mode flag
 */
void Settings::setFlagDifference(bool value)
{
    flagDifference = value;
    if (value)
        Helper::printDebugTwoColumns2S("Difference mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Difference mode", "OFF");
}

/**
 * @brief Returns info mode flag
 * @return Info mode flag
 */
bool Settings::getFlagInfo() const
{
    return flagInfo;
}

/**
 * @brief Sets info mode flag
 * @param[in] value Info mode flag
 */
void Settings::setFlagInfo(bool value)
{
    flagInfo = value;
    if (value)
        Helper::printDebugTwoColumns2S("Info mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Info mode", "OFF");
}

/**
 * @brief Returns compute period mode flag
 * @return Compute period mode flag
 */
bool Settings::getFlagComputePeriod() const
{
    return flagComputePeriod;
}

/**
 * @brief Sets compute period mode flag
 * @param[in] value Compute period mode flag
 */
void Settings::setFlagComputePeriod(bool value)
{
    flagComputePeriod = value;
    if (value)
        Helper::printDebugTwoColumns2S("Compute period mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Compute period mode", "OFF");
}

/**
 * @brief Returns find min/max mode flag
 * @return Find min/max mode flag
 */
bool Settings::getFlagFindMinMax() const
{
    return flagFindMinMax;
}

/**
 * @brief Sets find min/max mode flag
 * @param[in] value Find min/max mode flag
 */
void Settings::setFlagFindMinMax(bool value)
{
    flagFindMinMax = value;
    if (value)
        Helper::printDebugTwoColumns2S("Find min/max mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Find min/max mode", "OFF");
}

/**
 * @brief Returns log mode flag
 * @return Log mode flag
 */
bool Settings::getFlagLog() const
{
    return flagLog;
}

/**
 * @brief Sets log mode flag
 * @param[in] value Log mode flag
 */
void Settings::setFlagLog(bool value)
{
    flagLog = value;
    if (value)
        Helper::printDebugTwoColumns2S("Log mode", "ON");
    else
        Helper::printDebugTwoColumns2S("Log", "OFF");
}

/**
 * @brief Returns params definition
 * @return Params definition
 */
ParamsDefinition Settings::getParamsDefinition() const
{
    return paramsDefinition;
}
