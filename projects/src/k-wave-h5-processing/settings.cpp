/**
 * @file        settings.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing settings.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "settings.h"

Settings::Settings()
{

}

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

    // Size
    ParamsDefinition::Flag::Params paramsS;
    paramsS.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("s", paramsS);

    // Chunk size
    ParamsDefinition::Flag::Params paramsCh;
    paramsCh.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("ch", paramsCh);

    // Block size
    ParamsDefinition::Flag::Params paramsC;
    paramsC.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("c", paramsC);

    // Period
    ParamsDefinition::Flag::Params paramsP;
    paramsP.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("p", paramsP);

    // Harmonic
    ParamsDefinition::Flag::Params paramsH;
    paramsH.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("h", paramsH);

    // Multiple of overlap size
    ParamsDefinition::Flag::Params paramsMOS;
    paramsMOS.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("mos", paramsMOS);

    // Names
    ParamsDefinition::Flag::Params paramsNames;
    paramsNames.defineParam(ParamsDefinition::STRINGS_SEPARATED);
    paramsDefinition.defineParamsFlag("names", paramsNames);

    // HDF5 simulation output filename
    ParamsDefinition::Flag::Params paramsF;
    paramsF.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("f", paramsF);

    // HDF5 simulation input filename
    ParamsDefinition::Flag::Params paramsM;
    paramsM.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("m", paramsM);

    // HDF5 processing output filename
    ParamsDefinition::Flag::Params paramsO;
    paramsO.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("o", paramsO);

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
                             "                                          dataset data, creates 2 new datasets for fi and k. \n"
                             "\n"
                             "  -decompress ........................... Optional parameter. Performs decompression of time series\n"
                             "                                          dataset data, needs datasets fi and k.\n"
                             "\n"
                             "  -difference ........................... Optional parameter. Performs substraction of time series\n"
                             "                                          dataset data, needs original dataset and decoded (name_d).\n"
                             "\n"
                             "  -s size ............................... Optional parameter. Max size for donwsampling.\n"
                             "                                          Default size is 512.\n"
                             "\n"
                             "  -ch chunkSize ......................... Optional parameter. The size for new chunks from 1 to\n"
                             "                                          maximal appropriately value. Default size is 64 (64^3).\n"
                             "\n"
                             "  -c blockSize .......................... Optional parameter. Sets number of data elements\n"
                             "                                          for block reading. Default value is based on available\n"
                             "                                          system physical memory.\n"
                             "\n"
                             "  -p period ............................. Optional parameter. Sets period of input signal for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -h harmonic ........................... Optional parameter. Sets multiple of harmonic frequency for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -mos size ............................. Optional parameter. Sets multiple of overlap size for\n"
                             "                                          compression of time series HIFU data.\n"
                             "\n"
                             "  -names name1;name2;... ................ Optional parameter. Names of selected datasets or groups\n"
                             "                                          to processing.\n"
                             "\n"
                             "  -help ................................. Prints this help message.\n"
                             "\n");

    // Parse params from command line
    try {
        paramsDefinition.commandLineParse(argc, argv);
    } catch (std::exception &e) {
        std::cerr << "\n  Wrong parameter " << e.what() << std::endl << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Set flags according to params
    ParamsDefinition::Flags flags = paramsDefinition.getFlags();

    if (flags.at("help").getEnabled()) {
        std::cout << paramsDefinition.getHelp() << std::endl;
        exit(EXIT_SUCCESS);
    }

    setFlagReshape(flags.at("reshape").getEnabled());
    setFlagChangeChunks(flags.at("changeChunks").getEnabled());
    setFlagDwnsmpl(flags.at("dwnsmpl").getEnabled());
    setFlagCompress(flags.at("compress").getEnabled());
    setFlagDecompress(flags.at("decompress").getEnabled());
    setFlagDifference(flags.at("difference").getEnabled());

    setFlagNames(flags.at("names").getEnabled());

    if (flagNames) {
        std::list<std::string> names;
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
        std::string outputFilename;
        flags.at("o").getParams().readParam(0, &outputFilename);
        setProcessingOutputFilename(outputFilename);
    }

    if (flags.at("s").getEnabled()) {
        unsigned long long maxSize;
        flags.at("s").getParams().readParam(0, &maxSize);
        setMaxSize(maxSize);
    }

    if (flags.at("ch").getEnabled()) {
        unsigned long long maxChunkSize;
        flags.at("ch").getParams().readParam(0, &maxChunkSize);
        setMaxChunkSize(maxChunkSize);
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

std::string Settings::getSimulationOutputFilename() const
{
    return simulationOutputFilename;
}

void Settings::setSimulationOutputFilename(const std::string &value)
{
    simulationOutputFilename = value;
    std::cout << "\n  Simulation output filename:\n    " << simulationOutputFilename << std::endl;
}

std::string Settings::getSimulationInputFilename() const
{
    return simulationInputFilename;
}

void Settings::setSimulationInputFilename(const std::string &value)
{
    simulationInputFilename = value;
    std::cout << "\n  Simulation input filename:\n    " << simulationInputFilename << std::endl;
}

std::string Settings::getProcessingOutputFilename() const
{
    return processingOutputFilename;
}

void Settings::setProcessingOutputFilename(const std::string &value)
{
    processingOutputFilename = value;
    std::cout << "\n  Processing output filename:\n    " << processingOutputFilename << std::endl;
}

unsigned long long Settings::getMaxSize() const
{
    return maxSize;
}

void Settings::setMaxSize(const unsigned long long &value)
{
    maxSize = value;
    std::cout << "\n  Max size for downsampling:\n    " << maxSize << std::endl;
}

unsigned long long Settings::getMaxChunkSize() const
{
    return maxChunkSize;
}

void Settings::setMaxChunkSize(const unsigned long long &value)
{
    maxChunkSize = value;
    std::cout << "\n  Chunk size:\n    " << maxChunkSize << std::endl;
}

unsigned long long Settings::getBlockSize() const
{
    return blockSize;
}

void Settings::setBlockSize(const unsigned long long &value)
{
    blockSize = value;
    std::cout << "\n  Max size for block reading:\n    " << blockSize << std::endl;
}

unsigned long long Settings::getMOS() const
{
    return mOS;
}

void Settings::setMOS(const unsigned long long &value)
{
    mOS = value;
    std::cout << "\n  Multiple of overlap size:\n    " << mOS << std::endl;
}

unsigned long long Settings::getPeriod() const
{
    return period;
}

void Settings::setPeriod(const unsigned long long &value)
{
    period = value;
    std::cout << "\n  Period for compression:\n    " << period << std::endl;
}

unsigned long long Settings::getHarmonic() const
{
    return harmonic;
}

void Settings::setHarmonic(const unsigned long long &value)
{
    harmonic = value;
    std::cout << "\n  Multiple of harmonic frequency for compression:\n    " << harmonic << std::endl;
}


std::list<std::string> Settings::getNames() const
{
    return names;
}

void Settings::setNames(const std::list<std::string> &value)
{
    names = value;
    std::cout << "\n  Selected datasets or groups names:\n    ";
    for (std::list<std::string>::const_iterator ci = value.begin(); ci != value.end(); ++ci) {
        std::cout << *ci << ", ";
    }
    std::cout << std::endl;
}

bool Settings::getFlagNames() const
{
    return flagNames;
}

void Settings::setFlagNames(bool value)
{
    flagNames = value;
}

bool Settings::getFlagReshape() const
{
    return flagReshape;
}

void Settings::setFlagReshape(bool value)
{
    flagReshape = value;
    if (value)
        std::cout << "\n  Reshape mode: ON\n" << std::endl;
    else
        std::cout << "\n  Reshape mode: OFF\n" << std::endl;
}

bool Settings::getFlagChangeChunks() const
{
    return flagRechunk;
}

void Settings::setFlagChangeChunks(bool value)
{
    flagRechunk = value;
    if (value)
        std::cout << "\n  Change chunks mode: ON\n" << std::endl;
    else
        std::cout << "\n  Change chunks mode: OFF\n" << std::endl;
}

bool Settings::getFlagDwnsmpl() const
{
    return flagDwnsmpl;
}

void Settings::setFlagDwnsmpl(bool value)
{
    flagDwnsmpl = value;
    if (value)
        std::cout << "\n  Downsampling mode: ON\n" << std::endl;
    else
        std::cout << "\n  Downsampling mode: OFF\n" << std::endl;
}

bool Settings::getFlagCompress() const
{
    return flagCompress;
}

void Settings::setFlagCompress(bool value)
{
    flagCompress = value;
    if (value)
        std::cout << "\n  Compression mode: ON\n" << std::endl;
    else
        std::cout << "\n  Compression mode: OFF\n" << std::endl;
}

bool Settings::getFlagDecompress() const
{
    return flagDecompress;
}

void Settings::setFlagDecompress(bool value)
{
    flagDecompress = value;
    if (value)
        std::cout << "\n  Decompression mode: ON\n" << std::endl;
    else
        std::cout << "\n  Decompression mode: OFF\n" << std::endl;
}

bool Settings::getFlagDifference() const
{
    return flagDifference;
}

void Settings::setFlagDifference(bool value)
{
    flagDifference = value;
    if (value)
        std::cout << "\n  Difference mode: ON\n" << std::endl;
    else
        std::cout << "\n  Difference mode: OFF\n" << std::endl;
}

ParamsDefinition Settings::getParamsDefinition() const
{
    return paramsDefinition;
}
