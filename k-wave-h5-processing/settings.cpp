/**
 * @file        settings.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Settings class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "settings.h"

/**
 * @brief Creates Settings object
 */
Settings::Settings()
{
}

/**
 * @brief Creates Settings object with arguments
 * @param[in] argc Arguments count
 * @param[in] argv Arguments string
 */
Settings::Settings(int argc, const char **argv)
{
    loadParams(argc, argv);
}

/**
 * @brief Loads params
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 */
void Settings::loadParams(int argc, const char **argv)
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
    paramsDefinition.defineParamsFlag("shift");
    paramsDefinition.defineParamsFlag("c40bit");
    paramsDefinition.defineParamsFlag("noOverlap");

    // Size
    paramsDefinition.defineParamsFlag("s", ParamsDefinition::ULONGLONG);

    // Chunk sizes
    paramsDefinition.defineParamsFlag("ch", ParamsDefinition::ULONGLONG_SEPARATED);

    // Block size
    paramsDefinition.defineParamsFlag("c", ParamsDefinition::ULONGLONG);

    // Period
    paramsDefinition.defineParamsFlag("p", ParamsDefinition::FLOAT);

    // Frequency
    paramsDefinition.defineParamsFlag("fq", ParamsDefinition::FLOAT);

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
    paramsDefinition.setHelp(
        "+---------------------------------------------------------------+\n"
        "|   Usage: k-wave-h5-processing [parameters]                    |\n"
        "+---------------------------------------------------------------+\n"
        "|   Mandatory parameters:                                       |\n"
        "+-------------------------------+-------------------------------+\n"
        "| -f <file_name>                | HDF5 output or input file     |\n"
        "+-------------------------------+-------------------------------+\n"
        "|   Optional parameters:                                        |\n"
        "+-------------------------------+-------------------------------+\n"
        "| -m <file_name>                | HDF5 input file with          |\n"
        "|                               |   sensor_mask_index or        |\n"
        "|                               |   sensor_mask_corners or      |\n"
        "|                               |   p_source_input dataset.     |\n"
        "| -o <file_name>                | HDF5 processing output file   |\n"
        "|                               |   Default is HDF5 output or   |\n"
        "|                               |   input file name + .         |\n"
        "|                               |   \"_modified.h5\"              |\n"
        "| -d <file_name>                | HDF5 processing input file    |\n"
        "|                               |   for reading other datasets  |\n"
        "|                               |   from a separate file.       |\n"
        "| -reshape                      | Performs processing sensor    |\n"
        "|                               |   mask type datasets to group |\n"
        "|                               |   with 4D datasets and saves  |\n"
        "|                               |   datasets to the output      |\n"
        "|                               |   file. The sensor_mask_index |\n"
        "|                               |   or sensor_mask_corners      |\n"
        "|                               |   dataset must be in          |\n"
        "|                               |   simulation output or        |\n"
        "|                               |   simulation input file.      |\n"
        "| -changeChunks                 | Sets a new chunks size of     |\n"
        "|                               |   float datasets and saves    |\n"
        "|                               |   datasets to the output      |\n"
        "|                               |   file.                       |\n"
        "| -dwnsmpl                      | Performs downsampling of      |\n"
        "|                               |   datasets and saves them to  |\n"
        "|                               |   the output file.            |\n"
        "| -compress                     | Performs compression of time  |\n"
        "|                               |   series dataset data,        |\n"
        "|                               |   creates new dataset in the  |\n"
        "|                               |   output file with            |\n"
        "|                               |   coefficients, e.g p_c.      |\n"
        "| -decompress                   | Performs decompression of     |\n"
        "|                               |   time series dataset data,   |\n"
        "|                               |   needs dataset with          |\n"
        "|                               |   coefficients, e.g p_c.      |\n"
        "| -difference                   | Performs subtraction of       |\n"
        "|                               |   datasets data. Needs        |\n"
        "|                               |   original dataset end        |\n"
        "|                               |   decoded (name_d) dataset    |\n"
        "|                               |   with src_dataset_name       |\n"
        "|                               |   attribute or two dataset    |\n"
        "|                               |   names - the first name from |\n"
        "|                               |   HDF5 output or input file   |\n"
        "|                               |   and the second name from    |\n"
        "|                               |   HDF5 processing input file. |\n"
        "| -s <size>                     | Max size for downsampling.    |\n"
        "|                               |   Default size is 512.        |\n"
        "| -ch <chunkSize;chunkSize;>    | The sizes for new chunks      |\n"
        "|                               |   (x;y;z;t;) from 1 to        |\n"
        "|                               |   maximal appropriately       |\n"
        "|                               |   value. Default sizes are    |\n"
        "|                               |   64;64;64;1.                 |\n"
        "| -c <blockSize>                | Sets number of data elements  |\n"
        "|                               |   for block reading. Default  |\n"
        "|                               |   value is based on available |\n"
        "|                               |   system physical memory.     |\n"
        "| -p <period>                   | Sets period of input signal   |\n"
        "|                               |   for compression of time     |\n"
        "|                               |   series HIFU data (float).   |\n"
        "| -fq <frequency>               | Sets frequency of input       |\n"
        "|                               |   signal for compression      |\n"
        "|                               |   of time series HIFU data    |\n"
        "|                               |   (float).                    |\n"
        "| -h <harmonics>                | Sets multiple of harmonic     |\n"
        "|                               |   frequency for compression   |\n"
        "|                               |   of time series HIFU data.   |\n"
        "| -mos <size>                   | Sets multiple of overlap size |\n"
        "|                               |   for compression of time     |\n"
        "|                               |   series HIFU data.           |\n"
        "| -shift                        | Enables time shift for        |\n"
        "|                               |   compression of time series  |\n"
        "|                               |   HIFU data.                  |\n"
        "| -c40bit                       | Enables 40-bit for            |\n"
        "|                               |   compression of time series  |\n"
        "|                               |   HIFU data.                  |\n"
        "| -noOverlap                    | Enables no overlap for        |\n"
        "|                               |   compression of time series  |\n"
        "|                               |   HIFU data.                  |\n"
        "| -names <name1;name2;>         | Names of selected datasets or |\n"
        "|                               |   groups to processing.       |\n"
        "| -info                         | Prints the values of          |\n"
        "|                               |   attributes of selected      |\n"
        "|                               |   datasets.                   |\n"
        "| -computePeriod                | Computes period from          |\n"
        "|                               |   p_source_input. Store it to |\n"
        "|                               |   the simulation input file   |\n"
        "|                               |   if the file is set.         |\n"
        "| -findMinMax                   | Finds and stores minimal and  |\n"
        "|                               |   maximal values and their    |\n"
        "|                               |   indices of known datasets   |\n"
        "|                               |   from simulation output file |\n"
        "|                               |   and processing input file.  |\n"
        "| -help                         | Prints this help message.     |\n"
        "+-------------------------------+-------------------------------+\n");

    // Parse params from command line
    //Helper::printDebugMsg("");
    try {
        paramsDefinition.commandLineParse(argc, argv);
    } catch (std::exception &e) {
        Helper::printMsg(paramsDefinition.getHelp());
        Helper::printErrorMsg("Wrong parameter " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }

    // Set flags according to params
    ParamsDefinition::Flags flags = paramsDefinition.getFlags();

    if (flags.at("help").getEnabled()) {
        Helper::printMsg(paramsDefinition.getHelp());
        exit(EXIT_SUCCESS);
    }

    Helper::enableDebugMsgs = flags.at("log").getEnabled();
    Helper::printDebugTitle("k-wave-h5-processing");
    Helper::printDebugTitle("Settings");

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

    if (flags.at("compress").getEnabled() || flags.at("decompress").getEnabled()) {
        setFlagShift(flags.at("shift").getEnabled());
        setFlagC40bit(flags.at("c40bit").getEnabled());
        setFlagNoOverlap(flags.at("noOverlap").getEnabled());
    }

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
        float period;
        flags.at("p").getParams().readParam(0, &period);
        setPeriod(period);
    }

    if (flags.at("fq").getEnabled()) {
        float frequency;
        flags.at("fq").getParams().readParam(0, &frequency);
        setFrequency(frequency);
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
    Helper::printDebugTwoColumnsS("Simulation output filename", simulationOutputFilename, 0, 30);
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
    Helper::printDebugTwoColumnsS("Simulation input filename", simulationInputFilename, 0, 30);
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
    Helper::printDebugTwoColumnsS("Processing output filename", processingOutputFilename, 0, 30);
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
    Helper::printDebugTwoColumnsS("Processing input filename", processingInputFilename, 0, 30);
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
    Helper::printDebugTwoColumnsS("Max size for downsampling", maxSize, 0, 30);
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
    //maxChunkSizes = value;
    for (size_t i = 0; i < value.size(); i++) {
        maxChunkSizes[i] = value[i];
    }
    std::string sizesString;
    ParamsDefinition::VectorOfULongLongs maxChunkSizesR = maxChunkSizes;
    std::reverse(maxChunkSizesR.begin(), maxChunkSizesR.end());
    for (ParamsDefinition::VectorOfULongLongs::const_iterator ci = maxChunkSizesR.begin(); ci != maxChunkSizesR.end(); ++ci) {
        sizesString +=  std::to_string(*ci);
        if (std::next(ci) != maxChunkSizesR.end())
            sizesString += " x ";
    }
    Helper::printDebugTwoColumnsS("New chunk sizes", sizesString, 0, 30);
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
    Helper::printDebugTwoColumnsS("Max size for block reading", blockSize, 0, 30);
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
    Helper::printDebugTwoColumnsS("Multiple of overlap size for compression", mOS, 0, 40);
}

/**
 * @brief Returns maximal chunk size at X dimension
 * @return Maximal chunk size at X dimension
 */
unsigned long long Settings::getMaxChunkSizeX() const
{
    return maxChunkSizes.at(0);
}

/**
 * @brief Returns maximal chunk size at Y dimension
 * @return Maximal chunk size at Y dimension
 */
unsigned long long Settings::getMaxChunkSizeY() const
{
    return maxChunkSizes.at(1);
}

/**
 * @brief Returns maximal chunk size at Z dimension
 * @return Maximal chunk size at Z dimension
 */
unsigned long long Settings::getMaxChunkSizeZ() const
{
    return maxChunkSizes.at(2);
}

/**
 * @brief Returns maximal chunk size at W dimension
 * @return Maximal chunk size at W dimension
 */
unsigned long long Settings::getMaxChunkSizeW() const
{
    return maxChunkSizes.at(3);
}

/**
 * @brief Returns maximal chunk size at T dimension
 * @return Maximal chunk size at T dimension
 */
unsigned long long Settings::getMaxChunkSizeT() const
{
    return maxChunkSizes.at(3);
}

/**
 * @brief Returns period
 * @return Period
 */
float Settings::getPeriod() const
{
    return period;
}

/**
 * @brief Sets period for compression
 * @param[in] value Period for compression
 */
void Settings::setPeriod(const float &value)
{
    period = value;
    Helper::printDebugTwoColumnsS("Period for compression", period, 0, 40);
}

/**
 * @brief Returns frequency
 * @return Frequency
 */
float Settings::getFrequency() const
{
    return frequency;
}

/**
 * @brief Sets frequency for compression
 * @param[in] value Frequency for compression
 */
void Settings::setFrequency(const float &value)
{
    frequency = value;
    Helper::printDebugTwoColumnsS("Frequency for compression", frequency, 0, 40);
}

/**
 * @brief Returns multiple of harmonic frequency for compression
 * @return Number of harmonics
 */
unsigned long long Settings::getHarmonics() const
{
    return harmonics;
}

/**
 * @brief Sets multiple of harmonic frequency for compression
 * @param[in] value Multiple of harmonic frequency for compression
 */
void Settings::setHarmonic(const unsigned long long &value)
{
    harmonics = value;
    Helper::printDebugTwoColumnsS("Number of harmonics for compression", harmonics, 0, 40);
}

/**
 * @brief Returns time shift flag for compression
 * @return Shift flag
 */
bool Settings::getFlagShift() const
{
    return shift;
}

/**
 * @brief Sets time shift flag for compression
 * @param[in] value Time shift flag for compression
 */
void Settings::setFlagShift(bool value)
{
    shift = value;
    if (shift) {
        Helper::printDebugTwoColumnsS("Time shift flag for compression", "TRUE", 0, 40);
    } else {
        Helper::printDebugTwoColumnsS("Time shift flag for compression", "FALSE", 0, 40);
    }
}

/**
 * @brief Returns 40-bit flag for compression
 * @return 40-bit flag
 */
bool Settings::getFlagC40bit() const
{
    return c40bit;
}

/**
 * @brief Sets 40-bit flag for compression
 * @param[in] value 40-bit flag for compression
 */
void Settings::setFlagC40bit(bool value)
{
    c40bit = value;
    if (c40bit) {
        Helper::printDebugTwoColumnsS("40-bit flag for compression", "TRUE", 0, 40);
    } else {
        Helper::printDebugTwoColumnsS("40-bit flag for compression", "FALSE", 0, 40);
    }
}

/**
 * @brief Returns no overlap flag for compression
 * @return No overlap flag
 */
bool Settings::getFlagNoOverlap() const
{
    return noOverlap;
}

/**
 * @brief Sets no overlap flag for compression
 * @param[in] value No overlap flag for compression
 */
void Settings::setFlagNoOverlap(bool value)
{
    noOverlap = value;
    if (noOverlap) {
        Helper::printDebugTwoColumnsS("No overlap flag for compression", "TRUE", 0, 40);
    } else {
        Helper::printDebugTwoColumnsS("No overlap flag for compression", "FALSE", 0, 40);
    }
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
 * @brief Sets selected datasets or groups
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
    Helper::printDebugTwoColumnsS("Selected datasets or groups", namesString, 0, 30);
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
        Helper::printDebugTwoColumnsS("Reshape mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Reshape mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Change chunks mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Change chunks mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Downsampling mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Downsampling mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Compression mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Compression mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Decompression mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Decompression mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Difference mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Difference mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Info mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Info mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Compute period mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Compute period mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Find min/max mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Find min/max mode", "OFF");
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
        Helper::printDebugTwoColumnsS("Log mode", "ON");
    else
        Helper::printDebugTwoColumnsS("Log", "OFF");
}

/**
 * @brief Returns params definition
 * @return Params definition
 */
ParamsDefinition Settings::getParamsDefinition() const
{
    return paramsDefinition;
}
