/**
 * @file        settings.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        8  September 2016 (created)
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


Settings::Settings()
{
    init();
}

void Settings::loadParams(int argc, char **argv)
{
    // Define expected params
    paramsDefinition.defineParamsFlag("help");
    paramsDefinition.defineParamsFlag("reshape");
    paramsDefinition.defineParamsFlag("changeChunks");
    paramsDefinition.defineParamsFlag("dwnsmpl");

    ParamsDefinition::Flag::Params paramsS;
    paramsS.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("s", paramsS);

    ParamsDefinition::Flag::Params paramsCh;
    paramsCh.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("ch", paramsCh);

    ParamsDefinition::Flag::Params paramsC;
    paramsC.defineParam(ParamsDefinition::ULONGLONG);
    paramsDefinition.defineParamsFlag("c", paramsC);

    ParamsDefinition::Flag::Params paramsNames;
    paramsNames.defineParam(ParamsDefinition::STRINGS_SEPARATED);
    paramsDefinition.defineParamsFlag("names", paramsNames);

    ParamsDefinition::Flag::Params paramsF;
    paramsF.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("f", paramsF);

    ParamsDefinition::Flag::Params paramsM;
    paramsM.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("m", paramsM);

    ParamsDefinition::Flag::Params paramsO;
    paramsO.defineParam(ParamsDefinition::STRING);
    paramsDefinition.defineParamsFlag("o", paramsO);

    paramsDefinition.setHelp("\n"
    "Usage: k-wave-h5-processing [options]\n"
    "where options include:\n\n"
    "  -f HDF5SimulationOutputFilename ...... Required parameter.\n"
    "                                         HDF5 file with simulation results.\n"
    "\n"
    "  -m HDF5SimulationInputFilename ....... Optional parameter. Simulation HDF5 input file\n"
    "                                         with sensor_mask_index dataset.\n"
    "\n"
    "  -o HDF5OutputFilename ................ Optional parameter - output filename. Default value is\n"
    "                                         HDF5SimulationOutputFilename + \"_modified.h5\".\n"
    "\n"
    "  -reshape ............................. Optional parameter. Performs processing sensor mask\n"
    "                                         type datasets to group with series of 3D datasets\n"
    "                                         and saves datasets to a new file.\n"
    "                                         In hDF5SimOutputFile or hDF5SimInputFile\n"
    "                                         must be sensor_mask_index dataset.\n"
    "\n"
    "  -changeChunks ........................ Optional parameter. Sets a new size chunks of 3D type\n"
    "                                         datasets and saves datasets to a new file.\n"
    "\n"
    "  -dwnsmpl ............................. Optional parameter. Performs downsampling of datasets\n"
    "                                         and saves them to new file.\n"
    "\n"
    "  -s size .............................. Optional parameter. Max size for donwsampling.\n"
    "\n"
    "  -ch chunkSize ........................ Optional parameter. Size for new chunks from 1 to\n"
    "                                         maximal appropriately value.\n"
    "                                         groups for processing.\n"
    "\n"
    "  -c blockSize ......................... Optional parameter. Set number of data elements\n"
    "                                         for block reading.\n"
    "\n"
    "  -names name1;name2;... ............... Optional parameter. Names of selected datasets or\n"
    "\n"
    "  -help ................................ Prints this help message.\n"
    "\n");

    paramsDefinition.commandLineParse(argc, argv);

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
        setOutputFilename(outputFilename);
    }

    if (flags.at("s").getEnabled()) {
        hsize_t maxSize;
        flags.at("s").getParams().readParam(0, &maxSize);
        setMaxSize(maxSize);
    }

    if (flags.at("ch").getEnabled()) {
        hsize_t maxChunkSize;
        flags.at("ch").getParams().readParam(0, &maxChunkSize);
        setMaxChunkSize(maxChunkSize);
    }

    if (flags.at("c").getEnabled()) {
        hsize_t blockSize;
        flags.at("c").getParams().readParam(0, &blockSize);
        setBlockSize(blockSize);
    }
}

void Settings::init()
{
    // Filenames
    simulationOutputFilename = "";
    simulationInputFilename = "";
    outputFilename = "";

    // Size vars
    maxSize = 512;
    maxChunkSize = 64;
    blockSize = HDF5Helper::NUMBER_OF_ELEMENTS_TO_LOAD; // From HDF5File.h

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

std::string Settings::getOutputFilename()
{
    return outputFilename;
}

void Settings::setOutputFilename(const std::string &value)
{
    outputFilename = value;
    std::cout << "\n  Output filename:\n    " << outputFilename << std::endl;
}

hsize_t Settings::getMaxSize()
{
    return maxSize;
}

void Settings::setMaxSize(const hsize_t &value)
{
    maxSize = value;
    std::cout << "\n  Max size for downsampling:\n    " << maxSize << std::endl;
}

hsize_t Settings::getMaxChunkSize()
{
    return maxChunkSize;
}

void Settings::setMaxChunkSize(const hsize_t &value)
{
    maxChunkSize = value;
    std::cout << "\n  Chunk size:\n    " << maxChunkSize << std::endl;
}

hsize_t Settings::getBlockSize()
{
    return blockSize;
}

void Settings::setBlockSize(const hsize_t &value)
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
        std::cout << *ci << " ";
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
