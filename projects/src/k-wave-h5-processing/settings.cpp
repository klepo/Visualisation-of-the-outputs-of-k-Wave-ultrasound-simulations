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

// Dataset names
const std::string Settings::SENSOR_MASK_TYPE_DATASET("sensor_mask_type");
const std::string Settings::SENSOR_MASK_INDEX_DATASET("sensor_mask_index");
const std::string Settings::SENSOR_MASK_CORNERS_DATASET("sensor_mask_corners");
const std::string Settings::NT_DATASET("Nt");
const std::string Settings::NX_DATASET("Nx");
const std::string Settings::NY_DATASET("Ny");
const std::string Settings::NZ_DATASET("Nz");
const std::string Settings::P_SOURCE_INPUT_DATASET("p_source_input");


Settings::Settings()
{
    init();
}

void Settings::loadParams(int argc, char **argv)
{
    // Define expected params
    // Modes
    paramsDefinition.defineParamsFlag("help");
    paramsDefinition.defineParamsFlag("reshape");
    paramsDefinition.defineParamsFlag("changeChunks");
    paramsDefinition.defineParamsFlag("dwnsmpl");

    // Size
    ParamsDefinition::Flag::Params paramsS;
    paramsS.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("s", paramsS);

    // Chnunk size
    ParamsDefinition::Flag::Params paramsCh;
    paramsCh.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("ch", paramsCh);

    // Block size
    ParamsDefinition::Flag::Params paramsC;
    paramsC.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("c", paramsC);

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
    "  -f HDF5SimulationOutputFilename ...... Required parameter.\n"
    "                                         HDF5 file with simulation results.\n"
    "\n"
    "  -m HDF5SimulationInputFilename ....... Optional parameter. HDF5 simulation input filename \n"
    "                                         (with sensor_mask_index or sensor_mask_corners dataset).\n"
    "\n"
    "  -o HDF5ProcessingOutputFilename ...... Optional parameter. HDF5 processing output filename. \n"
    "                                         Default is HDF5SimulationOutputFilename + \"_modified.h5\".\n"
    "\n"
    "  -reshape ............................. Optional parameter. Performs processing sensor mask\n"
    "                                         type datasets to group with 4D datasets and saves datasets\n"
    "                                         to the output file. In HDF5SimulationOutputFilename or \n"
    "                                         HDF5SimulationInputFilename must be sensor_mask_index or\n"
    "                                         sensor_mask_corners dataset.\n"
    "\n"
    "  -changeChunks ........................ Optional parameter. Sets a new chunks size of\n"
    "                                         datasets and saves datasets to the output file.\n"
    "\n"
    "  -dwnsmpl ............................. Optional parameter. Performs downsampling of datasets\n"
    "                                         and saves them to the output file.\n"
    "\n"
    "  -s size .............................. Optional parameter. Max size for donwsampling.\n"
    "                                         Default size is 512.\n"
    "\n"
    "  -ch chunkSize ........................ Optional parameter. The size for new chunks from 1 to\n"
    "                                         maximal appropriately value. Default size is 64 (64^3).\n"
    "\n"
    "  -c blockSize ......................... Optional parameter. Set number of data elements\n"
    "                                         for block reading. Default value is based on available\n"
    "                                         system physical memory.\n"
    "\n"
    "  -names name1;name2;... ............... Optional parameter. Names of selected datasets or groups\n"
    "                                         to processing.\n"
    "\n"
    "  -help ................................ Prints this help message.\n"
    "\n");

    // Parse params from command line.
    paramsDefinition.commandLineParse(argc, argv);

    // Set flags according to params
    ParamsDefinition::Flags flags = paramsDefinition.getFlags();

    if (flags.at("help").getEnabled()) {
        std::cout << paramsDefinition.getHelp() << std::endl;
        exit(EXIT_SUCCESS);
    }

    setFlagReshape(flags.at("reshape").getEnabled());
    setFlagChangeChunks(flags.at("changeChunks").getEnabled());
    setFlagDwnsmpl(flags.at("dwnsmpl").getEnabled());

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
}

void Settings::init()
{
    // Filenames
    simulationOutputFilename = "";
    simulationInputFilename = "";
    processingOutputFilename = "";

    // Size vars
    maxSize = 512;
    maxChunkSize = 64;
    blockSize = 0;

    // Filter by names
    names.clear();
    flagNames = false;

    // Application modes
    flagReshape = false;
    flagRechunk = false;
    flagDwnsmpl = false;

    paramsDefinition = ParamsDefinition();
}

std::string Settings::getSimulationOutputFilename()
{
    return simulationOutputFilename;
}

void Settings::setSimulationOutputFilename(const std::string &value)
{
    simulationOutputFilename = value;
    std::cout << "\n  Simulation output filename:\n    " << simulationOutputFilename << std::endl;
}

std::string Settings::getSimulationInputFilename()
{
    return simulationInputFilename;
}

void Settings::setSimulationInputFilename(const std::string &value)
{
    simulationInputFilename = value;
    std::cout << "\n  Simulation input filename:\n    " << simulationInputFilename << std::endl;
}

std::string Settings::getProcessingOutputFilename()
{
    return processingOutputFilename;
}

void Settings::setProcessingOutputFilename(const std::string &value)
{
    processingOutputFilename = value;
    std::cout << "\n  Output filename:\n    " << processingOutputFilename << std::endl;
}

unsigned long long Settings::getMaxSize()
{
    return maxSize;
}

void Settings::setMaxSize(const unsigned long long &value)
{
    maxSize = value;
    std::cout << "\n  Max size for downsampling:\n    " << maxSize << std::endl;
}

unsigned long long Settings::getMaxChunkSize()
{
    return maxChunkSize;
}

void Settings::setMaxChunkSize(const unsigned long long &value)
{
    maxChunkSize = value;
    std::cout << "\n  Chunk size:\n    " << maxChunkSize << std::endl;
}

unsigned long long Settings::getBlockSize()
{
    return blockSize;
}

void Settings::setBlockSize(const unsigned long long &value)
{
    blockSize = value;
    std::cout << "\n  Max size for block reading:\n    " << blockSize << std::endl;
}

std::list<std::string> Settings::getNames()
{
    return names;
}

void Settings::setNames(const std::list<std::string> &value)
{
    names = value;
    std::cout << "\n  Selected datasets or groups names:    " << std::endl;
    for (std::list<std::string>::const_iterator ci = value.begin(); ci != value.end(); ++ci) {
        std::cout << *ci << ", ";
    }
}

bool Settings::getFlagNames()
{
    return flagNames;
}

void Settings::setFlagNames(bool value)
{
    flagNames = value;
}

bool Settings::getFlagReshape()
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

bool Settings::getFlagChangeChunks()
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

bool Settings::getFlagDwnsmpl()
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

ParamsDefinition Settings::getParamsDefinition() const
{
    return paramsDefinition;
}
