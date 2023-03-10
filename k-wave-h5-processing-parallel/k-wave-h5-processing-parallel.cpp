/**
 * @file        k-wave-h5-processing-parallel.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       Old not updated nonfunctional! The implementation file containing k-Wave HDF5 processing application.
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

#ifdef __unix
#include <unistd.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Winsock2.h>
#endif

#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <map>
#include <math.h>
#include <time.h>
#include <list>

#include <hdf5helper-parallel.h>

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#ifdef HAVE_OPENCV_CONTRIB
#include <opencv2/contrib/contrib.hpp>
#endif

const std::string SENSOR_MASK_TYPE_DATASET("sensor_mask_type");
const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");
const std::string SENSOR_MASK_TYPE_CORNERS("sensor_mask_corners");
const std::string NT_DATASET("Nt");
const std::string NX_DATASET("Nx");
const std::string NY_DATASET("Ny");
const std::string NZ_DATASET("Nz");

#define MAX_SIZE 512
#define MAX_CHUNK_SIZE 64
#define MAX_NUMBER_OF_FRAMES 0 // TODO set 0 -> all steps (frames)

// Filenames
std::string simulationOutputFilename = "";
std::string simulationInputFilename  = "";
std::string outputFilename           = "";

// Size vars
hsize_t maxSize      = MAX_SIZE;
hsize_t maxChunkSize = MAX_CHUNK_SIZE;
hsize_t blockSize    = HDF5Helper::NUMBER_OF_ELEMENTS_TO_LOAD; // From HDF5File.h

// Filter by names
std::list<std::string> names;
bool flagNames = false;

// Params for visualization
std::string datasetName = "";
std::string cutType     = "YX";
hsize_t cutIndex        = 0;

// Application modes
bool flagTest    = false;
bool flagReshape = false;
bool flagRechunk = false;
bool flagView    = false;
bool flagDwnsmpl = false;

bool flagCollective = false;

int mPISize;
int mPIRank;
MPI_Comm comm = MPI_COMM_WORLD;
MPI_Info info = MPI_INFO_NULL;

/**
 * @brief help
 * @return help string
 */
std::string help()
{
    return "\n"
           "Usage: k-wave-h5-processing-parallel [options]\n"
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
           "  -view datasetName cutType cutIndex ... Optional parameter. Visualizes the selected slice of\n"
           "                                         dataset. Required is datasetName, cutType values YX,\n"
           "                                         ZX or ZY and cutIndex values from 0 to max size-1 of\n"
           "                                         the selected dimension.\n"
           "\n"
           "  -dwnsmpl ............................. Optional parameter. Performs downsampling of datasets\n"
           "                                         and saves them to new file.\n"
           "\n"
           "  -collective .......................... Optional parameter. Enable collective parallel reading\n"
           "\n"
           "  -s size .............................. Optional parameter. Max size for donwsampling.\n"
           "\n"
           "  -ch chunkSize ........................ Optional parameter. Size for new chunks from 1 to\n"
           "                                         maximal appropriately value.\n"
           "\n"
           "  -names name1;name2; .................. Optional parameter. Names of selected datasets or\n"
           "                                         groups for processing.\n"
           "\n"
           "  -test ................................ Optional parameter. Test mode. Reading test \n"
           "                                         of 3D type datasets is performed.\n"

           "\n"
           "  -help ................................ Prints this help message.\n"
           "\n";
}

/**
 * @brief printDebugTitle Print debug title
 * @param msg
 */
void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

/**
 * @brief getParams
 * @param argc
 * @param argv
 */
void getParams(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -f (simulation output filename)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationOutputFilename = argv[i];
            std::cout << "\n  Simulation output filename:\n    " << simulationOutputFilename << std::endl;

        } else if (strcmp("-help", argv[i]) == 0) {
            std::cout << help() << std::endl;
            exit(EXIT_SUCCESS);
            continue;
        } else if (strcmp("-test", argv[i]) == 0) {
            flagTest = true;
            std::cout << "\n  Test mode: ON\n" << std::endl;
            continue;
        } else if (strcmp("-reshape", argv[i]) == 0) {
            flagReshape = true;
            std::cout << "\n  Reshape mode: ON\n" << std::endl;
            continue;
        } else if (strcmp("-changeChunks", argv[i]) == 0) {
            flagRechunk = true;
            std::cout << "\n  Change chunks mode: ON\n" << std::endl;
            continue;
        } else if (strcmp("-view", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -view (missing name of dataset)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            datasetName = argv[i];
            std::cout << "\n  Name of dataset to visualize:\n    " << datasetName << std::endl;
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -view (missing type of cut)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            cutType = argv[i];
            std::cout << "\n  Cut type to visualize:\n    " << cutType << std::endl;
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -view (missing index)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            try {
                size_t s;
                cutIndex = std::stoi(argv[i], &s);
                if (strlen(argv[i]) != s)
                    throw std::invalid_argument(argv[i]);
                std::cout << "\n  Index to view:\n    " << cutIndex << std::endl;
            } catch (std::invalid_argument error) {
                std::cerr << "\n  Wrong parameter -view (wrong index)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            flagView = true;
            continue;
        } else if (strcmp("-dwnsmpl", argv[i]) == 0) {
            flagDwnsmpl = true;
            std::cout << "\n  Downsampling mode: ON\n" << std::endl;
            continue;
        } else if (strcmp("-collective", argv[i]) == 0) {
            flagCollective = true;
            std::cout << "\n  Collective reading: ON\n" << std::endl;
            continue;
        } else if (strcmp("-m", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -m (simulation intput filename)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationInputFilename = argv[i];
            std::cout << "\n  Simulation intput filename:\n    " << simulationInputFilename << std::endl;
            continue;
        } else if (strcmp("-o", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -o (output filename)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            outputFilename = argv[i];
            std::cout << "\n  Output filename:\n    " << outputFilename << std::endl;

        } else if (strcmp("-s", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -s (max size for downsampling)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            try {
                size_t s;
                maxSize = std::stoi(argv[i], &s);
                if (strlen(argv[i]) != s)
                    throw std::invalid_argument(argv[i]);
                std::cout << "\n  Max size for downsampling:\n    " << maxSize << std::endl;
            } catch (std::invalid_argument error) {
                std::cerr << "\n  Wrong parameter -s (max size for downsampling)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (strcmp("-ch", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -ch (chunk size)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            try {
                size_t s;
                maxChunkSize = std::stoi(argv[i], &s);
                if (strlen(argv[i]) != s)
                    throw std::invalid_argument(argv[i]);
                std::cout << "\n  Chunk size:\n    " << maxChunkSize << std::endl;
            } catch (std::invalid_argument) {
                std::cerr << "\n  Wrong parameter -ch (chunk size)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (strcmp("-names", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -names (dataset names)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cout << "\n  Selected datasets or groups names:    " << std::endl;
            std::string myText(argv[i]);
            std::istringstream iss(myText);
            std::string token;
            while (getline(iss, token, ';')) {
                names.insert(names.begin(), token);
                std::cout << "    " << token << std::endl;
                flagNames = true;
            }
        } else {
            std::cerr << "\n  Unknown options (" << argv[i] << ")" << std::endl;
            std::cout << help() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

hsize_t myRound(double number)
{
    return (hsize_t)floor(number + 0.5);
}

/**
 * @brief The DtsForPcs class Helper class for manipulation with datasets for processing
 */
class DtsForPcs
{
public:
    HDF5Helper::HDF5Dataset *sensorMaskIndexDataset;
    hsize_t sensorMaskSize;
    HDF5Helper::MapOfDatasets dtsMaskType;
    HDF5Helper::MapOfGroups dtsGroupType;
    HDF5Helper::MapOfGroups dtsGroupTypeDsp;
    HDF5Helper::MapOfDatasets dts3DType;
    HDF5Helper::MapOfDatasets dts3DTypeDsp;
};

/**
 * @brief loadSimulationFile Load simulation file by filename
 * @param simulationFilename path to HDF5 filename
 * @return file
 */
HDF5Helper::File *loadSimulationFile(std::string simulationFilename)
{
    HDF5Helper::File *hDF5SimulationFile = NULL;
    try {
        hDF5SimulationFile = new HDF5Helper::File(simulationFilename, HDF5Helper::File::OPEN, comm, info);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return hDF5SimulationFile;
}

/**
 * @brief findAndGetSensorMaskDataset Find and get sensor mask dataset
 * @param hDF5SimOutputFile
 * @param hDF5SimInputFile
 * @return sensor_mask_index dataset
 */
HDF5Helper::HDF5Dataset *findAndGetSensorMaskIndexDataset(HDF5Helper::File *hDF5SimOutputFile,
                                                          HDF5Helper::File *hDF5SimInputFile)
{
    HDF5Helper::HDF5Dataset *sensorMaskIndexDataset = NULL;
    std::cout << std::endl
              << std::endl
              << "---- Find and get sensor mask dataset ----" << std::endl
              << std::endl
              << std::endl;

    if (hDF5SimOutputFile->objExistsByName(SENSOR_MASK_INDEX_DATASET)) {
        // Try to load sensor mask from simulation output file
        try {
            sensorMaskIndexDataset = hDF5SimOutputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    } else if (hDF5SimInputFile != NULL && hDF5SimInputFile->objExistsByName(SENSOR_MASK_INDEX_DATASET)) {
        // Try to load sensor mask from simulation input file
        try {
            sensorMaskIndexDataset = hDF5SimInputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    } else {
        std::cout << "Sensor mask is not in simulation output or input file" << std::endl;
    }
    return sensorMaskIndexDataset;
}

/**
 * @brief createOrOpenOutputFile
 * @param outputFilename
 */
HDF5Helper::File *createOrOpenOutputFile(std::string outputFilename)
{
    std::string filename   = "";
    HDF5Helper::File *file = NULL;
    if (outputFilename.empty()) {
        // Create auto filename
        size_t lastindex    = simulationOutputFilename.find_last_of(".");
        std::string rawname = simulationOutputFilename.substr(0, lastindex);
        filename            = rawname + "_modified.h5";
    } else {
        filename = outputFilename;
    }

    try {
        // Try open file
        file = new HDF5Helper::File(filename, HDF5Helper::File::OPEN, comm, info);
    } catch (std::exception) {
        try {
            // Try create file
            file = new HDF5Helper::File(filename, HDF5Helper::File::CREATE, comm, info);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    return file;
}

/**
 * @brief findDatasetsForProcessing Find datasets for processing
 * @param hDF5SimOutputFile
 * @param dtsForPcs
 */
void findDatasetsForProcessing(HDF5Helper::File *hDF5SimOutputFile, DtsForPcs *dtsForPcs)
{
    for (hsize_t i = 0; i < hDF5SimOutputFile->getNumObjs(); i++) {
        H5G_obj_t type = hDF5SimOutputFile->getObjTypeByIdx(i);

        // Datasets
        if (type == H5G_DATASET) {
            HDF5Helper::HDF5Dataset *dataset = hDF5SimOutputFile->openDataset(i);

            // Filter by selected names
            if (flagNames && std::find(names.begin(), names.end(), dataset->getName()) == names.end()) {
                hDF5SimOutputFile->closeDataset(dataset->getName());
                continue;
            }

            // 3D type
            if (dataset->getDims().getLength() == 3) {
                HDF5Helper::HDF5Vector3D size = (HDF5Helper::HDF5Vector3D)dataset->getDims();
                if (dataset->getDataTypeClass() == H5T_FLOAT && dataset->getRank() == 3
                    && size.z() == hDF5SimOutputFile->getNZ() && size.y() == hDF5SimOutputFile->getNY()
                    && size.x() == hDF5SimOutputFile->getNX()) {
                    dtsForPcs->dts3DType.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Dataset *>(dataset->getName(), dataset));
                    std::cout << "----> 3D type dataset: " << dataset->getName() << "; size: " << size.z() << " x "
                              << size.y() << " x " << size.x() << std::endl
                              << std::endl;
                }
                // Downsampled 3D type
                else if (dataset->hasAttribute("dwnsmpl") && dataset->getDataTypeClass() == H5T_FLOAT
                         && dataset->getRank() == 3 && size.z() < hDF5SimOutputFile->getNZ()
                         && size.y() < hDF5SimOutputFile->getNY() && size.x() < hDF5SimOutputFile->getNX()) {
                    dtsForPcs->dts3DTypeDsp.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Dataset *>(dataset->getName(), dataset));
                    std::cout << "----> 3D type downsampled dataset: " << dataset->getName() << "; size: " << size.z()
                              << " x " << size.y() << " x " << size.x() << std::endl
                              << std::endl;
                }
                // Sensor mask type
                else if (dataset->getDataTypeClass() == H5T_FLOAT && dtsForPcs->sensorMaskIndexDataset != NULL
                         && dataset->getRank() == 3 && size.z() == 1 && size.y() <= hDF5SimOutputFile->getNT()
                         && size.x() == dtsForPcs->sensorMaskSize) {
                    dtsForPcs->dtsMaskType.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Dataset *>(dataset->getName(), dataset));
                    std::cout << "----> Mask type dataset: " << dataset->getName() << "; size: " << size.z() << " x "
                              << size.y() << " x " << size.x() << std::endl
                              << std::endl;
                }
            }
            // Unknown type
            else {
                hDF5SimOutputFile->closeDataset(dataset->getName());
            }
        }
        // Groups
        else if (type == H5G_GROUP) {
            // Reshaped mask type to group
            try {
                HDF5Helper::HDF5Group *group = hDF5SimOutputFile->openGroup(i);

                // Filter by selected names
                if (flagNames && std::find(names.begin(), names.end(), group->getName()) == names.end()) {
                    hDF5SimOutputFile->closeGroup(group->getName());
                    continue;
                }

                // Reshaped mask type group downsampled
                if (group->hasAttribute("src_group_id")) {
                    hsize_t count = group->readAttributeI("count");
                    hsize_t posX  = group->readAttributeI("positionX");
                    hsize_t posY  = group->readAttributeI("positionY");
                    hsize_t posZ  = group->readAttributeI("positionZ");
                    dtsForPcs->dtsGroupTypeDsp.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Group *>(group->getName(), group));
                    std::cout << "----> Reshaped mask type group downsampled: " << group->getName()
                              << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ
                              << std::endl
                              << std::endl;
                }
                // Reshaped mask type group
                else if (group->hasAttribute("count")) {
                    hsize_t count = group->readAttributeI("count");
                    hsize_t posX  = group->readAttributeI("positionX");
                    hsize_t posY  = group->readAttributeI("positionY");
                    hsize_t posZ  = group->readAttributeI("positionZ");
                    dtsForPcs->dtsGroupType.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Group *>(group->getName(), group));
                    std::cout << "----> Reshaped mask type group: " << group->getName() << "; count: " << count
                              << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl
                              << std::endl;
                }
                // Unknown type
                else {
                    hDF5SimOutputFile->closeGroup(group->getName());
                }

            } catch (std::exception) {
                std::cout << "Object " << hDF5SimOutputFile->getObjNameByIdx(i) << " is not reshaped group"
                          << std::endl;
            }
        }
    }
}

void testOfReading(DtsForPcs *dtsForPcs)
{
    // Check number of datasets
    if (dtsForPcs->dts3DType.empty() /* && dtsForPcs->dtsGroupType.empty()*/) {
        std::cout << "No dataset for test in simulation output file" << std::endl;
    } else {
        // For every 3D type dataset
        for (HDF5Helper::MapOfDatasets::iterator it = dtsForPcs->dts3DType.begin(); it != dtsForPcs->dts3DType.end();
             ++it) {
            try {
                HDF5Helper::HDF5Dataset *dataset = it->second;

                MPI_Barrier(comm);

                std::cout << "Dataset: " << dataset->getName() << std::endl;
                std::cout << std::endl;

                float minValue = 0;
                float maxValue = 0;
                float *data    = 0;
                HDF5Helper::HDF5Vector3D offset;
                HDF5Helper::HDF5Vector3D count;

                if (mPISize > 1 && flagCollective)
                    dataset->setMPIOAccess(H5FD_MPIO_COLLECTIVE);

                HDF5Helper::HDF5Vector3D size = dataset->getDims();
                std::cout << "Dataset size:       " << size.z() << " x " << size.y() << " x " << size.x() << std::endl;
                HDF5Helper::HDF5Vector3D chunkSize = dataset->getChunkDims();
                std::cout << "Dataset chunk size: " << chunkSize.z() << " x " << chunkSize.y() << " x " << chunkSize.x()
                          << std::endl;

                dataset->setNumberOfElmsToLoad(dataset->getSize() / mPISize);

                std::cout << "Block reading test..." << std::endl;
                std::cout << "   reading block size (number of elements): " << dataset->getNumberOfElmsToLoad()
                          << std::endl;
                std::cout << std::endl;

                MPI_Barrier(comm);

                double ts = HDF5Helper::getTime();

                hsize_t steps = dataset->getNumberOfBlocks() / mPISize;
                for (hsize_t i = 0; i < steps; i++) {
                    double ts1 = HDF5Helper::getTime();
                    dataset->read3DBlock(mPISize * i + mPIRank, offset, count, data, minValue, maxValue);
                    double tf1 = HDF5Helper::getTime();
                    std::cout << "p: " << mPIRank << " readBlock time: " << (tf1 - ts1) << " ms; \t" << std::endl;
                    delete[] data; // !!
                }
                if (dataset->getNumberOfBlocks() % mPISize > 0) {
                    if (mPIRank < (int)dataset->getNumberOfBlocks() % mPISize) {
                        double ts3 = HDF5Helper::getTime();
                        dataset->read3DBlock(steps * mPISize + mPIRank, offset, count, data, minValue, maxValue);
                        double tf3 = HDF5Helper::getTime();
                        std::cout << "p: " << mPIRank << "readBlock time: " << (tf3 - ts3) << " ms; \t" << std::endl;
                        delete[] data; // !!
                    } else {
                        dataset->readEmptyBlock();
                    }
                }

                MPI_Barrier(comm);

                double tf = HDF5Helper::getTime();

                std::cout << std::endl;
                std::cout << "Time of the block reading test: " << (tf - ts) << " ms; \t" << std::endl;
                std::cout << std::endl;

                /*std::cout << "Slices reading test..." << std::endl;

                std::cout << std::endl;

                std::cout << "   XY" << std::endl;

                // XY
                dataset->read3DDataset(0, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(1, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(2, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() / 2 - 1, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() / 2, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() / 2 + 1, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() - 3, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() - 2, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                dataset->read3DDataset(size.z() - 1, 0, 0, 1, size.y(), size.x(), data, minValue, maxValue);
                //height = size.y();
                //width = size.x();
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete[] data;

                std::cout << "   XZ" << std::endl;

                // XZ
                dataset->read3DDataset(0, 0, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, 1, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, 2, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() / 2 - 1, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() / 2, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() / 2 + 1, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() - 3, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() - 2, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                dataset->read3DDataset(0, size.y() - 1, 0, size.z(), 1, size.x(), data, minValue, maxValue);
                //height = size.z();
                //width = size.x();
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete[] data;

                std::cout << "   YZ" << std::endl;

                // YZ
                dataset->read3DDataset(0, 0, 0, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, 1, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, 2, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() / 2 - 1, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() / 2, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() / 2 + 1, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() - 3, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() - 2, size.z(), size.y(), 1, data, minValue, maxValue);
                dataset->read3DDataset(0, 0, size.x() - 1, size.z(), size.y(), 1, data, minValue, maxValue);
                //height = size.z();
                //width = size.y();
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete[] data;


                std::cout << std::endl;*/

            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * @brief reshape Performs reshaping of sensor mask type datasets
 * @param hDF5SimOutputFile
 * @param hDF5OutputFile
 * @param dtsForPcs
 */
void reshape(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5OutputFile, DtsForPcs *dtsForPcs)
{
    // Check number of datasets
    if (dtsForPcs->dtsMaskType.empty()) {
        std::cout << "No dataset for reshape in simuation output file" << std::endl;
    } else {
        try {
            hsize_t minX = hDF5SimOutputFile->getNX();
            hsize_t minY = hDF5SimOutputFile->getNY();
            hsize_t minZ = hDF5SimOutputFile->getNZ();
            hsize_t maxX = 0, maxY = 0, maxZ = 0;

            // Find min and max position from linear saved values
            hsize_t index = 0;
            hsize_t *data;
            hsize_t minV, maxV;
            HDF5Helper::HDF5Vector3D offset;
            HDF5Helper::HDF5Vector3D count;
            HDF5Helper::HDF5Vector3D dstPos;
            for (hsize_t i = 0; i < dtsForPcs->sensorMaskIndexDataset->getNumberOfBlocks(); i++) {
                dtsForPcs->sensorMaskIndexDataset->read3DBlock(i, offset, count, data, minV, maxV);
                for (hsize_t z = 0; z < count.z(); z++)
                    for (hsize_t y = 0; y < count.y(); y++)
                        for (hsize_t x = 0; x < count.x(); x++) {
                            // [x + y * count.x() + z * count.x() * count.y()] is from 0, but value of index is from 1
                            index = data[x + y * count.x() + z * count.x() * count.y()];
                            HDF5Helper::convertlinearTo3D(index, dstPos, hDF5SimOutputFile->getNdims());
                            if (dstPos.x() < minX)
                                minX = dstPos.x();
                            if (dstPos.y() < minY)
                                minY = dstPos.y();
                            if (dstPos.z() < minZ)
                                minZ = dstPos.z();
                            if (dstPos.x() > maxX)
                                maxX = dstPos.x();
                            if (dstPos.y() > maxY)
                                maxY = dstPos.y();
                            if (dstPos.z() > maxZ)
                                maxZ = dstPos.z();
                        }
                delete[] data; // !!
            }
            std::cout << "   min point:\t"
                      << " z: " << minZ << "\ty: " << minY << "\tx: " << minX << std::endl;
            std::cout << "   max point:\t"
                      << " z: " << maxZ << "\ty: " << maxY << "\tx: " << maxX << std::endl;

            // TODO --> unused
            // Find first and last point
            /*hsize_t *size = dtsForPcs->sensorMaskIndexDataset->getDims();
            hsize_t firstX = 0, firstY = 0, firstZ = 0, lastX = 0, lastY = 0, lastZ = 0;
            // Get data for first point
            dtsForPcs->sensorMaskIndexDataset->read3DDataset(0, 0, 0, 1, 1, 1, data, minV, maxV);
            hDF5SimOutputFile->convertlinearTo3D(data[0], firstZ, firstY, firstX);
            delete[] data; // !!
            // Get data for last point
            dtsForPcs->sensorMaskIndexDataset->read3DDataset(size.z() - 1, size.y() - 1, size.x() - 1, 1, 1, 1, data,
            minV, maxV); hDF5SimOutputFile->convertlinearTo3D(data[0], lastZ, lastY, lastX); delete[] data; // !!
            std::cout << "   first point:\t" << " z: " << firstZ << "\ty: " << firstY << "\tx: " << firstX << std::endl;
            std::cout << "   last point:\t" << " z: " << lastZ << "\ty: " << lastY << "\tx: " << lastX << std::endl;*/
            // unused

            // Compute chunk size according to min/max position
            HDF5Helper::HDF5Vector3D chunkSize;
            chunkSize.z(std::min(maxChunkSize, maxZ - minZ + 1));
            chunkSize.y(std::min(maxChunkSize, maxY - minY + 1));
            chunkSize.x(std::min(maxChunkSize, maxX - minX + 1));
            // and dataset size
            HDF5Helper::HDF5Vector3D datasetSize(maxZ - minZ + 1, maxY - minY + 1, maxX - minX + 1);
            std::cout << "   new dataset size:\t" << datasetSize.z() << " x " << datasetSize.y() << " x "
                      << datasetSize.x() << std::endl;
            std::cout << "   new chunk size:\t" << chunkSize.z() << " x " << chunkSize.y() << " x " << chunkSize.x()
                      << std::endl;

            // For every mask type dataset
            for (HDF5Helper::MapOfDatasets::iterator it = dtsForPcs->dtsMaskType.begin();
                 it != dtsForPcs->dtsMaskType.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;

                hsize_t *sensorMaskData;
                float *datasetData = 0;
                hsize_t minVI, maxVI;
                float minVF = std::numeric_limits<float>::infinity(), maxVF = -std::numeric_limits<float>::infinity();
                float minVFL = std::numeric_limits<float>::infinity(), maxVFL = -std::numeric_limits<float>::infinity();
                float minVFG = std::numeric_limits<float>::infinity(), maxVFG = -std::numeric_limits<float>::infinity();
                HDF5Helper::HDF5Vector3D offset; // Offset
                HDF5Helper::HDF5Vector3D count;  // Count

                // Set same block size as sensorMaskIndexDataset
                dataset->setNumberOfElmsToLoad(dtsForPcs->sensorMaskIndexDataset->getRealNumberOfElmsToLoad());

                // Save attributes to new group
                hDF5OutputFile->createGroup(dataset->getName(), true);
                HDF5Helper::HDF5Group *group = hDF5OutputFile->openGroup(dataset->getName());
                group->setAttribute("positionZ", (hsize_t)minZ);
                group->setAttribute("positionY", (hsize_t)minY);
                group->setAttribute("positionX", (hsize_t)minX);
                group->setAttribute("sizeZ", (hsize_t)datasetSize.z());
                group->setAttribute("sizeY", (hsize_t)datasetSize.y());
                group->setAttribute("sizeX", (hsize_t)datasetSize.x());

                hsize_t number_of_frames = MAX_NUMBER_OF_FRAMES < ((HDF5Helper::HDF5Vector3D)dataset->getDims()).y()
                                               ? MAX_NUMBER_OF_FRAMES
                                               : ((HDF5Helper::HDF5Vector3D)dataset->getDims()).y();
                if (MAX_NUMBER_OF_FRAMES == 0)
                    number_of_frames = ((HDF5Helper::HDF5Vector3D)dataset->getDims()).y();
                std::cout << "number_of_frames: " << number_of_frames << std::endl;

                // First datasets in group
                if (number_of_frames < (hsize_t)mPISize) {
                    for (int i = 0; i < (int)number_of_frames; i++)
                        hDF5OutputFile->createDatasetF(dataset->getName() + "/" + std::to_string(i), datasetSize,
                                                       chunkSize, true);
                } else {
                    for (int i = 0; i < mPISize; i++)
                        hDF5OutputFile->createDatasetF(dataset->getName() + "/" + std::to_string(i), datasetSize,
                                                       chunkSize, true);
                }

                MPI_Barrier(comm);

                hsize_t index = 0;
                HDF5Helper::HDF5Vector3D dstPos;
                float data[1];
                hsize_t frame                          = 0;
                HDF5Helper::HDF5Dataset *actualDataset = NULL;

                // Set as actual
                if (number_of_frames >= (hsize_t)mPIRank + 1) {
                    actualDataset = hDF5OutputFile->openDataset(dataset->getName() + "/" + std::to_string(mPIRank));
                }

                MPI_Barrier(comm);

                for (hsize_t i = 0; i < dtsForPcs->sensorMaskIndexDataset->getNumberOfBlocks() * number_of_frames;
                     i++) {
                    // Load data
                    if (number_of_frames >= frame + mPIRank + 1) {
                        dtsForPcs->sensorMaskIndexDataset->read3DBlock(
                            i % dtsForPcs->sensorMaskIndexDataset->getNumberOfBlocks(), offset, count, sensorMaskData,
                            minVI, maxVI);
                        dataset->read3DDataset(HDF5Helper::HDF5Vector3D(0, frame + mPIRank, offset.x()), count,
                                               datasetData, minVF, maxVF);

                        if (minVFL > minVF)
                            minVFL = minVF;
                        if (maxVFL < maxVF)
                            maxVFL = maxVF;

                        double t4 = HDF5Helper::getTime();
                        // For the entire block write "voxels"
                        for (hsize_t z = 0; z < count.z(); z++)
                            for (hsize_t y = 0; y < count.y(); y++)
                                for (hsize_t x = 0; x < count.x(); x++) {
                                    hsize_t srcIndex = x + y * count.x() + z * count.x() * count.y();
                                    index            = sensorMaskData[srcIndex];
                                    HDF5Helper::convertlinearTo3D(index, dstPos, hDF5SimOutputFile->getNdims());
                                    data[0] = datasetData[srcIndex];
                                    actualDataset->write3DDataset(HDF5Helper::HDF5Vector3D(dstPos.z() - minZ,
                                                                                           dstPos.y() - minY,
                                                                                           dstPos.x() - minX),
                                                                  HDF5Helper::HDF5Vector3D(1, 1, 1), data, false);
                                }

                        double t5 = HDF5Helper::getTime();

                        std::cout << actualDataset->getName() << " write time: " << (t5 - t4) << " ms; \t" << std::endl;

                        delete[] sensorMaskData;
                        delete[] datasetData;
                    }

                    // Next time step for group of datasets (yDO)
                    if (i % dtsForPcs->sensorMaskIndexDataset->getNumberOfBlocks() + 1
                        == dtsForPcs->sensorMaskIndexDataset->getNumberOfBlocks()) {

                        if (number_of_frames >= frame + mPIRank + 1) {
                            // setAttribute attribute must be collective!! PROBLEM !!!!
                            // actualDataset->setAttribute("min", minVFL);
                            // actualDataset->setAttribute("max", maxVFL);

                            if (minVFG > minVFL)
                                minVFG = minVFL;
                            if (maxVFG < maxVFL)
                                maxVFG = maxVFL;

                            // Shift to next dataset -> step
                            hDF5OutputFile->closeDataset(actualDataset->getName());
                        }

                        frame += mPISize;

                        if (number_of_frames < frame + mPISize) {
                            if (number_of_frames > frame)
                                for (hsize_t i = 0; i < number_of_frames - frame; i++)
                                    hDF5OutputFile->createDatasetF(dataset->getName() + "/" + std::to_string(frame + i),
                                                                   datasetSize, chunkSize, true);
                        } else {
                            for (int i = 0; i < mPISize; i++)
                                hDF5OutputFile->createDatasetF(dataset->getName() + "/" + std::to_string(frame + i),
                                                               datasetSize, chunkSize, true);
                        }

                        if (number_of_frames < frame + mPIRank + 1)
                            break;

                        actualDataset
                            = hDF5OutputFile->openDataset(dataset->getName() + "/" + std::to_string(frame + mPIRank));
                    }
                }

                MPI_Barrier(comm);

                float minVFGG = std::numeric_limits<float>::infinity(),
                      maxVFGG = -std::numeric_limits<float>::infinity();

                MPI_Allreduce(&minVFG, &minVFGG, 1, MPI_FLOAT, MPI_MIN, comm);
                MPI_Allreduce(&maxVFG, &maxVFGG, 1, MPI_FLOAT, MPI_MAX, comm);

                // Save some attributes
                group->setAttribute("min", minVFGG);
                group->setAttribute("max", maxVFGG);
                group->setAttribute("count", (hsize_t)number_of_frames);

                MPI_Barrier(comm);

                // Filter or add to map for next processing
                if (!flagNames || std::find(names.begin(), names.end(), group->getName()) != names.end())
                    dtsForPcs->dtsGroupType.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Group *>(group->getName(), group));
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

void computeDstDims(HDF5Helper::HDF5Vector3D nDims, HDF5Helper::HDF5Vector3D &nDimsDst, double &ratio,
                    HDF5Helper::HDF5Vector3D &chunkSize)
{
    // Get max dimension
    hsize_t nMax = std::max(std::max(nDims.z(), nDims.y()), nDims.x());
    ratio        = (double)maxSize / nMax;
    nDimsDst.z(myRound(nDims.z() * ratio));
    nDimsDst.y(myRound(nDims.y() * ratio));
    nDimsDst.x(myRound(nDims.x() * ratio));
    // Check < 1
    if (nDimsDst.z() < 1)
        nDimsDst.z(1);
    if (nDimsDst.y() < 1)
        nDimsDst.y(1);
    if (nDimsDst.x() < 1)
        nDimsDst.x(1);
    // Chunk size
    chunkSize.z(maxChunkSize);
    chunkSize.y(maxChunkSize);
    chunkSize.x(maxChunkSize);
    if (chunkSize.z() > nDimsDst.z())
        chunkSize.z(nDimsDst.z());
    if (chunkSize.y() > nDimsDst.y())
        chunkSize.y(nDimsDst.y());
    if (chunkSize.x() > nDimsDst.x())
        chunkSize.x(nDimsDst.x());
    std::cout << "   new size:\t" << nDimsDst.z() << " x " << nDimsDst.y() << " x " << nDimsDst.x() << std::endl;
}

void resamplingOfDataset(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector3D nDims,
                         HDF5Helper::HDF5Vector3D nDimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal, float &minVG,
                         float &maxVG)
{
    // Interpolation method:
    //  INTER_NEAREST   - a nearest-neighbor interpolation
    //  INTER_LINEAR    - a bilinear interpolation (used by default)
    //  INTER_AREA      - resampling using pixel area relation.
    //                    It may be a preferred method for image decimation, as it gives moire’-free results.
    //                    But when the image is zoomed, it is similar to the INTER_NEAREST method.
    //  INTER_CUBIC     - a bicubic interpolation over 4x4 pixel neighborhood
    //  INTER_LANCZOS4  - a Lanczos interpolation over 8x8 pixel neighborhood
    int interpolation = cv::INTER_NEAREST;

    double t0 = HDF5Helper::getTime();

    hsize_t nCount = 1; // Number of slabs along Z axis to get from neighbours

    if (interpolation == cv::INTER_CUBIC)
        nCount = 2;
    if (interpolation == cv::INTER_LANCZOS4)
        nCount = 4;
    if (interpolation == cv::INTER_AREA)
        nCount = 2;

    if (mPISize == 1)
        nCount = 0;

    std::cout << "nCount: " << nCount << std::endl;

    bool smallZCountFlag = false;

    // Divide dataset to every process
    if (nDims.z() < mPISize * nCount * 2) {
        srcDataset->setNumberOfElmsToLoad(nDims.z() * nDims.y() * nDims.x());
        smallZCountFlag = true;
    } else {
        srcDataset->setNumberOfElmsToLoad((hsize_t)ceil(double(nDims.z()) / mPISize) * nDims.y() * nDims.x());
    }
    // throw std::runtime_error("Too many processes for resampling");

    float *srcData = 0;
    HDF5Helper::HDF5Vector3D offset;
    HDF5Helper::HDF5Vector3D count;
    float minV = std::numeric_limits<float>::infinity(), maxV = -std::numeric_limits<float>::infinity();

    if (!smallZCountFlag || mPIRank == 0) {
        // Check and set collective flag for reading
        if (mPISize > 1 && flagCollective && !smallZCountFlag)
            srcDataset->setMPIOAccess(H5FD_MPIO_COLLECTIVE);

        // Every process reads other block
        srcDataset->read3DBlock(mPIRank, offset, count, srcData, minV, maxV);

        // Memory for first step
        // "+ nCount * 2" because of borders
        float *dataDst1 = new float[(count.z() + nCount * 2) * nDimsDst.y() * nDimsDst.x()]();

        double tRXY0 = HDF5Helper::getTime();

        // First resize 2D slices in XY plane
        for (hsize_t z = 0; z < count.z(); z++) {
            cv::Mat image    = cv::Mat((int)nDims.y(), (int)nDims.x(), CV_32FC1,
                                       &srcData[nDims.x() * nDims.y() * z]); // rows, cols (height, width)
            cv::Mat imageDst = cv::Mat((int)nDimsDst.y(), (int)nDimsDst.x(), CV_32FC1,
                                       &dataDst1[nDimsDst.x() * nDimsDst.y() * (z + nCount)]);
            cv::resize(image, imageDst, cv::Size((int)nDimsDst.x(), (int)nDimsDst.y()), 0, 0, interpolation);
            image.release();
        }

        double tRXY1 = HDF5Helper::getTime();

        // Delete srcData
        delete[] srcData;

        if (!smallZCountFlag)
            MPI_Barrier(comm);
        double tSN0 = HDF5Helper::getTime();

        // If there is more than one process -> send and receive data of neighbours
        if (mPISize > 1 && !smallZCountFlag) {
            MPI_Status statuses[4];
            MPI_Request requests[4];
            int dataSize          = (int)(nCount * nDimsDst.y() * nDimsDst.x());
            hsize_t slabSizeXYDst = nDimsDst.y() * nDimsDst.x();
            // Get neighbours' data
            // From left to right and back
            if (mPIRank == 0) {
                // to right
                MPI_Isend(&dataDst1[count.z() * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank + 1, mPIRank, comm,
                          &requests[0]);
                // MPI_Barrier(comm);

                // from right
                MPI_Irecv(&dataDst1[(count.z() + nCount) * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank + 1,
                          mPIRank + 1, comm, &requests[1]);
                // MPI_Barrier(comm);
                MPI_Waitall(2, requests, statuses);
                memcpy(&dataDst1[0], &dataDst1[nCount * slabSizeXYDst], sizeof(float) * dataSize);

            } else if (mPIRank == mPISize - 1) {
                // from left
                MPI_Irecv(&dataDst1[0], dataSize, MPI_FLOAT, mPIRank - 1, mPIRank - 1, comm, &requests[0]);
                // MPI_Barrier(comm);

                // to left
                MPI_Isend(&dataDst1[nCount * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank - 1, mPIRank, comm,
                          &requests[1]);
                // MPI_Barrier(comm);
                MPI_Waitall(2, requests, statuses);
                memcpy(&dataDst1[(count.z() + nCount) * slabSizeXYDst], &dataDst1[count.z() * slabSizeXYDst],
                       sizeof(float) * dataSize);

            } else {
                // from left
                MPI_Irecv(&dataDst1[0], dataSize, MPI_FLOAT, mPIRank - 1, mPIRank - 1, comm, &requests[0]);
                // to right
                MPI_Isend(&dataDst1[count.z() * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank + 1, mPIRank, comm,
                          &requests[1]);
                // MPI_Barrier(comm);

                // form right
                MPI_Irecv(&dataDst1[(count.z() + nCount) * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank + 1,
                          mPIRank + 1, comm, &requests[2]);
                // to left
                MPI_Isend(&dataDst1[nCount * slabSizeXYDst], dataSize, MPI_FLOAT, mPIRank - 1, mPIRank, comm,
                          &requests[3]);
                // MPI_Barrier(comm);
                MPI_Waitall(4, requests, statuses);
            }
        }

        if (!smallZCountFlag)
            MPI_Barrier(comm);
        double tSN1 = HDF5Helper::getTime();

        double tTYZ0 = HDF5Helper::getTime();

        // Transpose y<->z
        float *dataDst1T = new float[(count.z() + nCount * 2) * nDimsDst.y() * nDimsDst.x()]();
        for (hsize_t z = 0; z < (count.z() + nCount * 2); z++) {
            for (hsize_t y = 0; y < nDimsDst.y(); y++) {
                memcpy(&dataDst1T[nDimsDst.x() * z + y * ((count.z() + nCount * 2) * nDimsDst.x())],
                       &dataDst1[nDimsDst.x() * y + z * (nDimsDst.y() * nDimsDst.x())], sizeof(float) * nDimsDst.x());
            }
        }

        double tTYZ1 = HDF5Helper::getTime();

        // Delete dataDst1
        delete[] dataDst1;

        double countZDst   = smallZCountFlag ? double(nDimsDst.z()) : (double(nDimsDst.z()) / mPISize);
        hsize_t countZDstR = myRound(double(mPIRank + 1) * countZDst) - myRound(double(mPIRank) * countZDst);
        hsize_t nCountDst  = myRound((double)nCount * countZDstR / count.z());

        float *dataDst2 = new float[(countZDstR + nCountDst * 2) * nDimsDst.y() * nDimsDst.x()]();

        double tRXZ0 = HDF5Helper::getTime();

        // And resize 2D slices in XZ plane
        for (hsize_t y = 0; y < nDimsDst.y(); y++) {
            cv::Mat image
                = cv::Mat((int)(count.z() + nCount * 2), (int)nDimsDst.x(), CV_32FC1,
                          &dataDst1T[(count.z() + nCount * 2) * y * nDimsDst.x()]); // rows, cols (height, width)
            // cv::Mat image = cv::Mat((int) nDims.z(), (int) nDimsDst.x(), CV_32FC1, &dataDst1TRecv2[nDims.z() * y *
            // nDimsDst.x()]); // rows, cols (height, width)
            cv::Mat imageDst = cv::Mat((int)(countZDstR + nCountDst * 2), (int)nDimsDst.x(), CV_32FC1,
                                       &dataDst2[(countZDstR + nCountDst * 2) * y * nDimsDst.x()]);
            // cv::Mat imageDst = cv::Mat((int) nDimsDst.z(), (int) nDimsDst.x(), CV_32FC1, &dataDst2[nDimsDst.z() * y *
            // nDimsDst.x()]);
            cv::resize(image, imageDst, cv::Size((int)nDimsDst.x(), (int)(countZDstR + nCountDst * 2)), 0, 0,
                       interpolation);
            // cv::resize(image, imageDst, cv::Size((int) nDimsDst.x(), (int) nDimsDst.z()), 0, 0, interpolation);
            image.release();
        }

        double tRXZ1 = HDF5Helper::getTime();

        // Delete dataDst1TRecv2
        // delete [] dataDst1TRecv2;

        double tTZY0 = HDF5Helper::getTime();

        // Transpose back
        float *dataDst2T = new float[countZDstR * nDimsDst.y() * nDimsDst.x()]();
        // float *dataDst2T = new float[nDimsDst.z() * blockDepth * nDimsDst.x()]();
        for (hsize_t z = 0; z < countZDstR; z++) {
            for (hsize_t y = 0; y < nDimsDst.y(); y++) {
                memcpy(&dataDst2T[nDimsDst.x() * y + z * (nDimsDst.y() * nDimsDst.x())],
                       &dataDst2[nDimsDst.x() * (z + nCountDst) + y * ((countZDstR + nCountDst * 2) * nDimsDst.x())],
                       sizeof(float) * nDimsDst.x());
                // memcpy(&dataDst2T[nDimsDst.x() * y + z * (blockDepth * nDimsDst.x())], &dataDst2[nDimsDst.x() * z + y
                // * (nDimsDst.z() * nDimsDst.x())], sizeof(float) * nDimsDst.x());
            }
        }

        double tTZY1 = HDF5Helper::getTime();

        // Delete dataDst2
        delete[] dataDst2;

        if (mPISize > 1 && !smallZCountFlag)
            dstDatasetFinal->setMPIOAccess(H5FD_MPIO_COLLECTIVE);
        dstDatasetFinal->write3DDataset(HDF5Helper::HDF5Vector3D(myRound(double(mPIRank) * countZDst), 0, 0),
                                        HDF5Helper::HDF5Vector3D(countZDstR, nDimsDst.y(), nDimsDst.x()), dataDst2T,
                                        true);
        // dstDatasetFinal->write3DDataset(HDF5HDF5Vector3D(0, mPIRank * blockDepthG, 0), HDF5HDF5Vector3D(nDimsDst.z(),
        // blockDepth, nDimsDst.x()), dataDst2T, true);

        // Get min/max values
        dstDatasetFinal->getMinAndMaxValue(dataDst2T, countZDstR * nDimsDst.y() * nDimsDst.x(), minV, maxV);
        // dstDatasetFinal->getMinAndMaxValue(dataDst2T, nDimsDst.z() * blockDepth * nDimsDst.x(), minV, maxV);

        // Delete dataDst2T
        delete[] dataDst2T;

        std::cout << "Times: ";
        std::cout << "resample XY " << (tRXY1 - tRXY0) << ", ";
        std::cout << "transpoze YZ " << (tTYZ1 - tTYZ0) << ", ";
        std::cout << "send and receive (neighbours data) " << (tSN1 - tSN0) << ", ";
        std::cout << "resample XZ " << (tRXZ1 - tRXZ0) << ", ";
        std::cout << "transpoze ZY " << (tTZY1 - tTZY0) << " ";
        std::cout << "ms; \t" << std::endl;
    }

    MPI_Barrier(comm);

    MPI_Allreduce(&minV, &minVG, 1, MPI_FLOAT, MPI_MIN, comm);
    MPI_Allreduce(&maxV, &maxVG, 1, MPI_FLOAT, MPI_MAX, comm);

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of resampling of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

void resamplingOfDatasetAlltoallv(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector3D nDims,
                                  HDF5Helper::HDF5Vector3D nDimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal,
                                  float &minVG, float &maxVG)
{
    double t0 = HDF5Helper::getTime();

    if ((int)nDims.z() < mPISize)
        throw std::runtime_error("Too many processes for resampling");
    // TODO add check for small otuput sizes

    // Divide dataset to every process
    srcDataset->setNumberOfElmsToLoad((hsize_t)ceil(double(nDims.z()) / mPISize) * nDims.y() * nDims.x());

    // Interpolation method:
    //  INTER_NEAREST - a nearest-neighbor interpolation
    //  INTER_LINEAR - a bilinear interpolation (used by default)
    //  INTER_AREA - resampling using pixel area relation. It may be a preferred method for image decimation, as it
    //  gives moire’-free results. But when the image is zoomed, it is similar to the INTER_NEAREST method. INTER_CUBIC
    //  - a bicubic interpolation over 4x4 pixel neighborhood INTER_LANCZOS4 - a Lanczos interpolation over 8x8 pixel
    //  neighborhood
    int interpolation = cv::INTER_NEAREST;

    float *srcData = 0;
    HDF5Helper::HDF5Vector3D offset;
    HDF5Helper::HDF5Vector3D count;
    float minV = 0, maxV = 0;

    // Every process reads other block
    srcDataset->read3DBlock(mPIRank, offset, count, srcData, minV, maxV);

    // Memory for first step
    float *dataDst1 = new float[count.z() * nDimsDst.y() * nDimsDst.x()]();

    double tRXY0 = HDF5Helper::getTime();

    // First resize 2D slices in XY plane
    for (hsize_t z = 0; z < count.z(); z++) {
        cv::Mat image = cv::Mat((int)nDims.y(), (int)nDims.x(), CV_32FC1,
                                &srcData[nDims.x() * nDims.y() * z]); // rows, cols (height, width)
        cv::Mat imageDst
            = cv::Mat((int)nDimsDst.y(), (int)nDimsDst.x(), CV_32FC1, &dataDst1[nDimsDst.x() * nDimsDst.y() * z]);
        cv::resize(image, imageDst, cv::Size((int)nDimsDst.x(), (int)nDimsDst.y()), 0, 0, interpolation);
        image.release();
    }

    double tRXY1 = HDF5Helper::getTime();

    // Delete srcData
    delete[] srcData;

    double tTYZ0 = HDF5Helper::getTime();

    // Transpose y<->z
    float *dataDst1T = new float[count.z() * nDimsDst.y() * nDimsDst.x()]();
    for (hsize_t z = 0; z < count.z(); z++) {
        for (hsize_t y = 0; y < nDimsDst.y(); y++) {
            memcpy(&dataDst1T[nDimsDst.x() * z + y * (count.z() * nDimsDst.x())],
                   &dataDst1[nDimsDst.x() * y + z * (nDimsDst.y() * nDimsDst.x())], sizeof(float) * nDimsDst.x());
        }
    }

    double tTYZ1 = HDF5Helper::getTime();

    // Delete dataDst1
    delete[] dataDst1;

    // Sending by ZX slice

    // Number of floats in ZX slab in non-last block
    hsize_t dataCountG = srcDataset->getGeneralBlockDims()[0] /*.z()*/ * nDimsDst.x();
    // Number of floats in ZX slab in last block
    hsize_t dataCountL = nDims.z() * nDimsDst.x() - (mPISize - 1) * dataCountG;
    // Number of floats in ZX slab in current block
    hsize_t dataCount = count.z() * nDimsDst.x();

    // Number of floats in stitched ZX slice
    // hsize_t dstZXSlabOffset = nDims.z() * nDimsDst.x();

    // Number of slabs for every non-last process
    hsize_t blockDepthG = (hsize_t)ceil(double(nDimsDst.y()) / mPISize);
    // Last process can have different (smaller) block size
    hsize_t blockDepthL = nDimsDst.y() - blockDepthG * (mPISize - 1);
    // Number of slabs for every actual process
    hsize_t blockDepth = (mPIRank == mPISize - 1) ? blockDepthL : blockDepthG;

    // Memory for received data
    float *dataDst1TRecv = new float[nDims.z() * blockDepthG * nDimsDst.x()]();

    int *sendDispls = new int[mPISize];
    int *sendCounts = new int[mPISize];
    int *recvDispls = new int[mPISize];
    int *recvCounts = new int[mPISize];

    // Settings for MPI_Gatherv
    /*for (int i = 0; i < mPISize; i++) {
        recvDispls[i] = i * dataCountG; // offset
        if (i == mPISize - 1) // Last data count
            recvCounts[i] = dataCountL;
        else
            recvCounts[i] = dataCountG;
    }*/

    // For every process
    /*for (int r = 0; r < mPISize; r++) {
        if (r == mPISize - 1) { // Last process can have different number of slices
            for (hsize_t c = 0; c < blockDepthL; c++) {
                MPI_Gatherv(&dataDst1T[dataCount * (c + r * blockDepthG)], dataCount, MPI_FLOAT,
    &dataDst1TRecv[dstZXSlabOffset * c], recvCounts, recvDispls, MPI_FLOAT, r, comm);
            }
        } else {
            for (hsize_t c = 0; c < blockDepthG; c++) {
                MPI_Gatherv(&dataDst1T[dataCount * (c + r * blockDepthG)], dataCount, MPI_FLOAT,
    &dataDst1TRecv[dstZXSlabOffset * c], recvCounts, recvDispls, MPI_FLOAT, r, comm);
            }
        }
    }*/

    int rSize = 0;
    int sSize = 0;

    for (int i = 0; i < mPISize; i++) {
        recvDispls[i] = (int)(i * dataCountG * blockDepth); // offset
        sendDispls[i] = (int)(i * dataCount * blockDepthG); // offset
        if (i == mPISize - 1) {                             // Last data count
            recvCounts[i] = (int)(dataCountL * blockDepth);
            sendCounts[i] = (int)(dataCount * blockDepthL);
        } else {
            recvCounts[i] = (int)(dataCountG * blockDepth);
            sendCounts[i] = (int)(dataCount * blockDepthG);
        }
        rSize += recvCounts[i];
        sSize += sendCounts[i];
    }

    MPI_Barrier(comm);
    double tSZX0 = HDF5Helper::getTime();

    MPI_Alltoallv(dataDst1T, sendCounts, sendDispls, MPI_FLOAT, dataDst1TRecv, recvCounts, recvDispls, MPI_FLOAT, comm);

    MPI_Barrier(comm);
    double tSZX1 = HDF5Helper::getTime();

    float *dataDst1TRecv2 = new float[nDims.z() * blockDepthG * nDimsDst.x()]();
    // memcpy(dataDst1TRecv2, dataDst1TRecv, sizeof(float) * nDims.z() * blockDepth * nDimsDst.x());

    int dstOffset = 0;
    for (hsize_t y = 0; y < blockDepth; y++) {
        for (int i = 0; i < mPISize; i++) {
            if (i == mPISize - 1) { // Last block can have different size of slices
                memcpy(&dataDst1TRecv2[dstOffset], &dataDst1TRecv[i * dataCountG * blockDepth + y * dataCountL],
                       sizeof(float) * dataCountL);
                dstOffset += (int)dataCountL;
            } else {
                memcpy(&dataDst1TRecv2[dstOffset], &dataDst1TRecv[i * dataCountG * blockDepth + y * dataCountG],
                       sizeof(float) * dataCountG);
                dstOffset += (int)dataCountG;
            }
        }
    }

    delete[] recvDispls;
    delete[] recvCounts;
    delete[] sendDispls;
    delete[] sendCounts;

    // Delete dataDst1T
    delete[] dataDst1T;

    // Memory for resized data
    float *dataDst2 = new float[nDimsDst.z() * blockDepth * nDimsDst.x()]();

    double tRXZ0 = HDF5Helper::getTime();

    // After resize 2D slices in XZ plane
    for (hsize_t y = 0; y < blockDepth; y++) {
        cv::Mat image = cv::Mat((int)nDims.z(), (int)nDimsDst.x(), CV_32FC1,
                                &dataDst1TRecv2[nDims.z() * y * nDimsDst.x()]); // rows, cols (height, width)
        cv::Mat imageDst
            = cv::Mat((int)nDimsDst.z(), (int)nDimsDst.x(), CV_32FC1, &dataDst2[nDimsDst.z() * y * nDimsDst.x()]);
        cv::resize(image, imageDst, cv::Size((int)nDimsDst.x(), (int)nDimsDst.z()), 0, 0, interpolation);
        image.release();
    }

    double tRXZ1 = HDF5Helper::getTime();

    // Delete dataDst1TRecv
    delete[] dataDst1TRecv;
    // Delete dataDst1TRecv2
    delete[] dataDst1TRecv2;

    double tTZY0 = HDF5Helper::getTime();

    // Transpose back
    float *dataDst2T = new float[nDimsDst.z() * blockDepth * nDimsDst.x()]();
    for (hsize_t z = 0; z < nDimsDst.z(); z++) {
        for (hsize_t y = 0; y < blockDepth; y++) {
            memcpy(&dataDst2T[nDimsDst.x() * y + z * (blockDepth * nDimsDst.x())],
                   &dataDst2[nDimsDst.x() * z + y * (nDimsDst.z() * nDimsDst.x())], sizeof(float) * nDimsDst.x());
        }
    }

    double tTZY1 = HDF5Helper::getTime();

    // Delete dataDst2
    delete[] dataDst2;

    dstDatasetFinal->write3DDataset(HDF5Helper::HDF5Vector3D(0, mPIRank * blockDepthG, 0),
                                    HDF5Helper::HDF5Vector3D(nDimsDst.z(), blockDepth, nDimsDst.x()), dataDst2T, true);

    // Get min/max values
    dstDatasetFinal->getMinAndMaxValue(dataDst2T, nDimsDst.z() * blockDepth * nDimsDst.x(), minV, maxV);
    MPI_Allreduce(&minV, &minVG, 1, MPI_FLOAT, MPI_MIN, comm);
    MPI_Allreduce(&maxV, &maxVG, 1, MPI_FLOAT, MPI_MAX, comm);

    // Delete dataDst2T
    delete[] dataDst2T;

    double t1 = HDF5Helper::getTime();
    std::cout << "Times: ";
    std::cout << "resample XY " << (tRXY1 - tRXY0) << ", ";
    std::cout << "transpoze YZ " << (tTYZ1 - tTYZ0) << ", ";
    std::cout << "send ZX " << (tSZX1 - tSZX0) << ", ";
    std::cout << "resample XZ " << (tRXZ1 - tRXZ0) << ", ";
    std::cout << "transpoze ZY " << (tTZY1 - tTZY0) << " ";
    std::cout << "ms; \t" << std::endl;
    std::cout << "Time of resampling of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

/**
 * @brief downsampling Performs downsampling of datasets
 * @param hDF5SimOutputFile
 * @param hDF5OutputFile
 * @param dtsForPcs
 */
void downsampling(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5OutputFile, DtsForPcs *dtsForPcs)
{
    try {
        // Check number of datasets for downsamling
        if (dtsForPcs->dts3DType.empty() && dtsForPcs->dtsGroupType.empty()) {
            std::cout << "No dataset for downsampling in simulation output file" << std::endl;
            // Check current size -> is greater?
        } else if (std::max(std::max(hDF5SimOutputFile->getNZ(), hDF5SimOutputFile->getNY()),
                            hDF5SimOutputFile->getNX())
                   <= maxSize) {
            std::cout << "No dataset for downsampling - max(nZ, nY, nX) == "
                             + std::to_string(std::max(std::max(hDF5SimOutputFile->getNZ(), hDF5SimOutputFile->getNY()),
                                                       hDF5SimOutputFile->getNX()))
                             + " <= " + std::to_string(maxSize)
                      << std::endl;
        } else {
            HDF5Helper::HDF5Vector3D nDims = hDF5SimOutputFile->getNdims();
            HDF5Helper::HDF5Vector3D chunkSize;
            HDF5Helper::HDF5Vector3D nDimsDst;
            double ratio;
            float minV = 0, maxV = 0;
            // Compute new size
            computeDstDims(nDims, nDimsDst, ratio, chunkSize);

            // For every 3D type dataset
            for (HDF5Helper::MapOfDatasets::iterator it = dtsForPcs->dts3DType.begin();
                 it != dtsForPcs->dts3DType.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;

                hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(maxSize), nDimsDst,
                                               chunkSize, true);
                HDF5Helper::HDF5Dataset *dstDatasetFinal
                    = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(maxSize));

                // Downsampling
                resamplingOfDataset(srcDataset, nDims, nDimsDst, dstDatasetFinal, minV, maxV);

                // Save attributes
                dstDatasetFinal->setAttribute("min", minV);
                dstDatasetFinal->setAttribute("max", maxV);
                dstDatasetFinal->setAttribute("dwnsmpl", maxSize);
                dstDatasetFinal->setAttribute("src_dataset_name", srcDataset->getName());
                dstDatasetFinal->setAttribute("src_dataset_id", srcDataset->getId());

                // Filter or add to map
                if (!flagNames || std::find(names.begin(), names.end(), dstDatasetFinal->getName()) != names.end())
                    dtsForPcs->dts3DTypeDsp.insert(std::pair<const std::string, HDF5Helper::HDF5Dataset *>(
                        dstDatasetFinal->getName(), dstDatasetFinal));
            }

            // For every reshaped mask type
            for (HDF5Helper::MapOfGroups::iterator it = dtsForPcs->dtsGroupType.begin();
                 it != dtsForPcs->dtsGroupType.end(); ++it) {
                HDF5Helper::HDF5Group *srcGroup = it->second;

                // Create group (dstGroupFinal) in hDF5OutputFile
                hDF5OutputFile->createGroup(srcGroup->getName() + "_" + std::to_string(maxSize), true);
                HDF5Helper::HDF5Group *dstGroupFinal
                    = hDF5OutputFile->openGroup(srcGroup->getName() + "_" + std::to_string(maxSize));

                // Get datasets count
                hsize_t count = srcGroup->getNumObjs();

                // Get dimensions and min/max values
                float minVG                    = srcGroup->readAttributeF("min");
                float maxVG                    = srcGroup->readAttributeF("max");
                float minV                     = 0;
                maxV                           = 0;
                HDF5Helper::HDF5Dataset *tmp   = srcGroup->getFile()->openDataset(srcGroup->getName() + "/0");
                HDF5Helper::HDF5Vector3D nDims = tmp->getDims();
                srcGroup->getFile()->closeDataset(tmp->getName());

                // Check current size
                if (std::max(std::max(nDims.z(), nDims.y()), nDims.x()) <= maxSize) {
                    std::cout << "No dataset for downsampling - "
                                     + std::to_string(std::max(std::max(nDims.z(), nDims.y()), nDims.x()))
                                     + " <= " + std::to_string(maxSize)
                              << std::endl;
                    break;
                }

                HDF5Helper::HDF5Vector3D chunkSize;
                HDF5Helper::HDF5Vector3D nDimsDst;
                double ratio;
                // Compute new size
                computeDstDims(nDims, nDimsDst, ratio, chunkSize);

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5Helper::HDF5Dataset *srcDataset
                        = srcGroup->getFile()->openDataset(srcGroup->getName() + "/" + std::to_string(i));

                    hDF5OutputFile->createDatasetF(srcGroup->getName() + "_" + std::to_string(maxSize) + "/"
                                                       + std::to_string(i),
                                                   nDimsDst, chunkSize, true);
                    HDF5Helper::HDF5Dataset *dstDatasetFinal = hDF5OutputFile->openDataset(
                        srcGroup->getName() + "_" + std::to_string(maxSize) + "/" + std::to_string(i));

                    // Downsampling
                    resamplingOfDataset(srcDataset, nDims, nDimsDst, dstDatasetFinal, minV, maxV);

                    if (minVG > minV)
                        minVG = minV;
                    if (maxVG < maxV)
                        maxVG = maxV;

                    hDF5OutputFile->closeDataset(srcGroup->getName() + "_" + std::to_string(maxSize) + "/"
                                                 + std::to_string(i));
                    srcGroup->getFile()->closeDataset(srcDataset->getName());
                }

                // Save attributes
                hsize_t positionZ    = srcGroup->readAttributeI("positionZ");
                hsize_t positionY    = srcGroup->readAttributeI("positionY");
                hsize_t positionX    = srcGroup->readAttributeI("positionX");
                hsize_t newPositionZ = 0;
                hsize_t newPositionY = 0;
                hsize_t newPositionX = 0;
                if (positionZ > 0)
                    newPositionZ = myRound((double)(positionZ + 1) * ratio) - 1;
                if (positionY > 0)
                    newPositionY = myRound((double)(positionY + 1) * ratio) - 1;
                if (positionX > 0)
                    newPositionX = myRound((double)(positionX + 1) * ratio) - 1;
                hsize_t newSizeZ = myRound((double)(srcGroup->readAttributeI("sizeZ") + 1) * ratio) - 1;
                hsize_t newSizeY = myRound((double)(srcGroup->readAttributeI("sizeY") + 1) * ratio) - 1;
                hsize_t newSizeX = myRound((double)(srcGroup->readAttributeI("sizeX") + 1) * ratio) - 1;
                if (newSizeZ < 1)
                    newSizeZ = 1;
                if (newSizeY < 1)
                    newSizeY = 1;
                if (newSizeX < 1)
                    newSizeX = 1;
                dstGroupFinal->setAttribute("positionZ", (hsize_t)newPositionZ);
                dstGroupFinal->setAttribute("positionY", (hsize_t)newPositionY);
                dstGroupFinal->setAttribute("positionX", (hsize_t)newPositionX);
                dstGroupFinal->setAttribute("sizeZ", (hsize_t)newSizeZ);
                dstGroupFinal->setAttribute("sizeY", (hsize_t)newSizeY);
                dstGroupFinal->setAttribute("sizeX", (hsize_t)newSizeX);
                dstGroupFinal->setAttribute("count", (hsize_t)count);
                dstGroupFinal->setAttribute("dwnsmpl", (hsize_t)maxSize);
                dstGroupFinal->setAttribute("min", minVG);
                dstGroupFinal->setAttribute("max", maxVG);
                dstGroupFinal->setAttribute("src_group_name", srcGroup->getName());
                dstGroupFinal->setAttribute("src_group_id", (hsize_t)srcGroup->getId());

                // Filter or add to map
                if (!flagNames || std::find(names.begin(), names.end(), dstGroupFinal->getName()) != names.end())
                    dtsForPcs->dtsGroupTypeDsp.insert(
                        std::pair<const std::string, HDF5Helper::HDF5Group *>(dstGroupFinal->getName(), dstGroupFinal));
                else
                    hDF5OutputFile->closeGroup(dstGroupFinal->getName());
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief rechunkDataset Copy srcDataset to hDF5OutputFile with new chunk size
 * @param srcDataset source dataset
 * @param hDF5OutputFile output file
 * @param maxChunkSize chunk size
 */
void changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::File *hDF5OutputFile, hsize_t maxChunkSize)
{
    HDF5Helper::HDF5Vector3D dims = srcDataset->getDims();

    // Chunk size
    HDF5Helper::HDF5Vector3D chunkSize(maxChunkSize, maxChunkSize, maxChunkSize);
    if (chunkSize.z() > dims.z())
        chunkSize.z(dims.z());
    if (chunkSize.y() > dims.y())
        chunkSize.y(dims.y());
    if (chunkSize.x() > dims.x())
        chunkSize.x(dims.x());
    hDF5OutputFile->createDatasetF(srcDataset->getName(), dims, chunkSize, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName());

    MPI_Barrier(comm);
    double t0 = HDF5Helper::getTime();

    float *data = 0;
    float minV, maxV;
    float minVG = 0, maxVG = 0;

    HDF5Helper::HDF5Vector3D offset;
    HDF5Helper::HDF5Vector3D count;

    // Divide dataset to every process
    srcDataset->setNumberOfElmsToLoad((hsize_t)ceil(double(dims.z()) / mPISize) * dims.y() * dims.x());

    if (mPISize > 1 && flagCollective)
        srcDataset->setMPIOAccess(H5FD_MPIO_COLLECTIVE);

    // Read and write every block by one process
    srcDataset->read3DBlock(mPIRank, offset, count, data, minV, maxV);

    if (mPISize > 1)
        dstDataset->setMPIOAccess(H5FD_MPIO_COLLECTIVE);

    dstDataset->write3DDataset(offset, count, data, true);
    delete[] data; // !!

    // Copy attributes
    for (unsigned int i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr);
        delete attr;
    }

    MPI_Allreduce(&minV, &minVG, 1, MPI_FLOAT, MPI_MIN, comm);
    MPI_Allreduce(&maxV, &maxVG, 1, MPI_FLOAT, MPI_MAX, comm);

    dstDataset->setAttribute("min", minVG);
    dstDataset->setAttribute("max", maxVG);

    MPI_Barrier(comm);
    double t1 = HDF5Helper::getTime();

    std::cout << "Time of changing chunks of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDataset->getName());
}

/**
 * @brief rechunk Change chunk size of datasets
 * @param hDF5SimOutputFile
 * @param hDF5OutputFile
 * @param dtsForPcs
 */
void changeChunks(HDF5Helper::File *hDF5OutputFile, DtsForPcs *dtsForPcs)
{
    try {
        if (dtsForPcs->dts3DType.empty() && dtsForPcs->dts3DTypeDsp.empty() && dtsForPcs->dtsGroupType.empty()
            && dtsForPcs->dtsGroupTypeDsp.empty()) {
            std::cout << "No dataset for change chunks in simulation output file" << std::endl;
        } else {
            // For every 3D type dataset
            for (HDF5Helper::MapOfDatasets::iterator it = dtsForPcs->dts3DType.begin();
                 it != dtsForPcs->dts3DType.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;
                changeChunksOfDataset(srcDataset, hDF5OutputFile, maxChunkSize);
            }

            // For every 3D type downsampled dataset
            for (HDF5Helper::MapOfDatasets::iterator it = dtsForPcs->dts3DTypeDsp.begin();
                 it != dtsForPcs->dts3DTypeDsp.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;
                changeChunksOfDataset(srcDataset, hDF5OutputFile, maxChunkSize);
            }

            // For every reshaped mask type
            for (HDF5Helper::MapOfGroups::iterator it = dtsForPcs->dtsGroupType.begin();
                 it != dtsForPcs->dtsGroupType.end(); ++it) {
                HDF5Helper::HDF5Group *srcGroup = it->second;
                hsize_t count                   = srcGroup->getNumObjs();
                // Try create group, if it already exists, then do not rewrite it
                try {
                    hDF5OutputFile->createGroup(srcGroup->getName());
                } catch (std::exception) {
                }
                HDF5Helper::HDF5Group *dstGroup = hDF5OutputFile->openGroup(srcGroup->getName());

                // Copy attributes
                for (unsigned int i = 0; i < srcGroup->getNumAttrs(); i++) {
                    HDF5Helper::HDF5Attribute *attr = srcGroup->getAttribute(i);
                    dstGroup->setAttribute(attr, false);
                    delete attr;
                }

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5Helper::HDF5Dataset *srcDataset
                        = srcGroup->getFile()->openDataset(srcGroup->getName() + "/" + std::to_string(i));
                    changeChunksOfDataset(srcDataset, hDF5OutputFile, maxChunkSize);
                    srcGroup->getFile()->closeDataset(srcGroup->getName() + "/" + std::to_string(i));
                }
            }

            // For every reshaped mask type downsampled
            for (HDF5Helper::MapOfGroups::iterator it = dtsForPcs->dtsGroupTypeDsp.begin();
                 it != dtsForPcs->dtsGroupTypeDsp.end(); ++it) {
                HDF5Helper::HDF5Group *srcGroup = it->second;
                hsize_t count                   = srcGroup->getNumObjs();
                // Try create group, if it already exists, then do not rewrite it
                try {
                    hDF5OutputFile->createGroup(srcGroup->getName());
                } catch (std::exception) {
                }
                HDF5Helper::HDF5Group *dstGroup = hDF5OutputFile->openGroup(srcGroup->getName());

                // Copy attributes
                for (unsigned int i = 0; i < srcGroup->getNumAttrs(); i++) {
                    HDF5Helper::HDF5Attribute *attr = srcGroup->getAttribute(i);
                    dstGroup->setAttribute(attr, false);
                    delete attr;
                }

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5Helper::HDF5Dataset *srcDataset
                        = srcGroup->getFile()->openDataset(srcGroup->getName() + "/" + std::to_string(i));
                    changeChunksOfDataset(srcDataset, hDF5OutputFile, maxChunkSize);
                    srcGroup->getFile()->closeDataset(srcGroup->getName() + "/" + std::to_string(i));
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief visualize Simple slice visualization
 * @param hDF5ViewFile
 */
void visualization(HDF5Helper::File *hDF5ViewFile)
{
    try {
        HDF5Helper::HDF5Dataset *dataset;
        HDF5Helper::HDF5Group *group;
        bool flagGroup   = false;
        bool flagDataset = false;

        try {
            group     = hDF5ViewFile->openGroup(datasetName);
            flagGroup = true;
        } catch (std::exception) {
            // std::cerr << e.what() << std::endl;
            std::cout << "Dataset is not a time series" << std::endl;
            dataset                       = hDF5ViewFile->openDataset(datasetName);
            HDF5Helper::HDF5Vector3D size = dataset->getDims();
            if (size.z() > hDF5ViewFile->getNZ() || size.y() > hDF5ViewFile->getNY()
                || size.y() > hDF5ViewFile->getNX())
                std::cout << "Dataset is too large" << std::endl;
            else
                flagDataset = true;
        }

        if (flagGroup) {
            float minValueSeriesGlobal = group->readAttributeF("min");
            float maxValueSeriesGlobal = group->readAttributeF("max");
            hsize_t count              = group->readAttributeI("count");

            cv::namedWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex));
            cv::namedWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex));
            cv::namedWindow(datasetName + " series global " + cutType + " " + std::to_string(cutIndex));

            for (hsize_t i = 0; i < count; i++) {
                HDF5Helper::HDF5Dataset *dataset = hDF5ViewFile->openDataset(datasetName + "/" + std::to_string(i));

                float minValueGlobal = dataset->getGlobalMinValueF();
                float maxValueGlobal = dataset->getGlobalMaxValueF();

                float minValue = 0;
                float maxValue = 0;

                HDF5Helper::HDF5Vector3D size = dataset->getDims();
                std::cout << "Dataset size:       " << size.z() << " x " << size.y() << " x " << size.x() << std::endl;
                HDF5Helper::HDF5Vector3D chunkSize = dataset->getChunkDims();
                std::cout << "Dataset chunk size: " << chunkSize.z() << " x " << chunkSize.y() << " x " << chunkSize.x()
                          << std::endl;

                float *data    = 0;
                hsize_t height = 0;
                hsize_t width  = 0;

                if (cutType == "YX") {
                    if (cutIndex >= size.z())
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex)
                                                 + " >= " + std::to_string(size.z()));
                    dataset->read3DDataset(HDF5Helper::HDF5Vector3D(cutIndex, 0, 0),
                                           HDF5Helper::HDF5Vector3D(1, size.y(), size.x()), data, minValue,
                                           maxValue); // YX
                    height = size.y();
                    width  = size.x();
                } else if (cutType == "ZX") {
                    if (cutIndex >= size.y())
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex)
                                                 + " >= " + std::to_string(size.y()));
                    dataset->read3DDataset(HDF5Helper::HDF5Vector3D(0, cutIndex, 0),
                                           HDF5Helper::HDF5Vector3D(size.z(), 1, size.x()), data, minValue,
                                           maxValue); // ZX
                    height = size.z();
                    width  = size.x();
                } else if (cutType == "ZY") {
                    if (cutIndex >= size.x())
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex)
                                                 + " >= " + std::to_string(size.x()));
                    dataset->read3DDataset(HDF5Helper::HDF5Vector3D(0, 0, cutIndex),
                                           HDF5Helper::HDF5Vector3D(size.z(), size.y(), 1), data, minValue,
                                           maxValue); // ZY
                    height = size.z();
                    width  = size.y();
                } else
                    throw std::runtime_error("Wrong cutType (cutType must be YX, ZX or ZY)");

                std::cout << "   minValueSeriesGlobal: " << minValueSeriesGlobal
                          << "\tmaxValueSeriesGlobal: " << maxValueSeriesGlobal << std::endl;
                std::cout << "   minValueGlobal:       " << minValueGlobal
                          << "\tmaxValueGlobal:      " << maxValueGlobal << std::endl;
                std::cout << "   minValue:             " << minValue << "\tmaxValue:            " << maxValue
                          << std::endl;
                std::cout << "   width:                " << width << "\theight:              " << height << std::endl;

                cv::Mat imageL = cv::Mat((int)height, (int)width, CV_32FC1, data); // rows, cols (height, width)
                cv::Mat imageG;
                cv::Mat imageGG;

                imageL.convertTo(imageG, CV_8UC1, 255.0 / (maxValueGlobal - minValueGlobal),
                                 -minValueGlobal * 255.0 / (maxValueGlobal - minValueGlobal));
                imageL.convertTo(imageGG, CV_8UC1, 255.0 / (maxValueSeriesGlobal - minValueSeriesGlobal),
                                 -minValueSeriesGlobal * 255.0 / (maxValueSeriesGlobal - minValueSeriesGlobal));
                imageL.convertTo(imageL, CV_8UC1, 255.0 / (maxValue - minValue),
                                 -minValue * 255.0 / (maxValue - minValue));

                if (width > height) {
                    int dstWidth = 300;
                    cv::Size size(dstWidth, (int)ceil((double)imageL.size().height * dstWidth / imageL.size().width));
                    // Set original size
                    size = imageL.size();
                    cv::resize(imageL, imageL, size);
                    cv::resize(imageG, imageG, size);
                    cv::resize(imageGG, imageGG, size);
                } else {
                    int dstHeight = 300;
                    cv::Size size((int)ceil((double)imageL.size().width * dstHeight / imageL.size().height), dstHeight);
                    // Set original size
                    size = imageL.size();
                    cv::resize(imageL, imageL, size);
                    cv::resize(imageG, imageG, size);
                    cv::resize(imageGG, imageGG, size);
                }

                // cv::moveWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), 100, 50);
                // cv::moveWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex),
                // imageL.size().width + 50 + 100, 50); cv::moveWindow(datasetName + " series global " + cutType + " " +
                // std::to_string(cutIndex), 2 * imageL.size().width  + 100 + 100, 50);

                cv::applyColorMap(imageL, imageL, cv::COLORMAP_JET);
                cv::applyColorMap(imageG, imageG, cv::COLORMAP_JET);
                cv::applyColorMap(imageGG, imageGG, cv::COLORMAP_JET);

                cv::imshow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), imageL);
                cv::imshow(datasetName + " global " + cutType + " " + std::to_string(cutIndex), imageG);
                cv::imshow(datasetName + " series global " + cutType + " " + std::to_string(cutIndex), imageGG);

                cv::waitKey(0);
                imageL.release();
                imageGG.release();
                imageG.release();
                delete[] data;
            }
        } else if (flagDataset) {
            float minValueGlobal = dataset->getGlobalMinValueF();
            float maxValueGlobal = dataset->getGlobalMaxValueF();

            float minValue                = 0;
            float maxValue                = 0;
            HDF5Helper::HDF5Vector3D size = dataset->getDims();
            std::cout << "Dataset size:       " << size.z() << " x " << size.y() << " x " << size.x() << std::endl;
            HDF5Helper::HDF5Vector3D chunkSize = dataset->getChunkDims();
            std::cout << "Dataset chunk size: " << chunkSize.z() << " x " << chunkSize.y() << " x " << chunkSize.x()
                      << std::endl;

            float *data    = 0;
            hsize_t height = 0;
            hsize_t width  = 0;

            if (cutType == "YX") {
                if (cutIndex >= size.z())
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(HDF5Helper::HDF5Vector3D(cutIndex, 0, 0),
                                       HDF5Helper::HDF5Vector3D(1, size.y(), size.x()), data, minValue, maxValue); // YX
                height = size.y();
                width  = size.x();
            } else if (cutType == "ZX") {
                if (cutIndex >= size.y())
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(HDF5Helper::HDF5Vector3D(0, cutIndex, 0),
                                       HDF5Helper::HDF5Vector3D(size.z(), 1, size.x()), data, minValue, maxValue); // ZX
                height = size.z();
                width  = size.x();
            } else if (cutType == "ZY") {
                if (cutIndex >= size.x())
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(HDF5Helper::HDF5Vector3D(0, 0, cutIndex),
                                       HDF5Helper::HDF5Vector3D(size.z(), size.y(), 1), data, minValue, maxValue); // ZY
                height = size.z();
                width  = size.y();
            } else
                throw std::runtime_error("Wrong cutType (cutType must be YX, ZX or ZY)");

            std::cout << "   minValueGlobal: " << minValueGlobal << "\tmaxValueGlobal: " << maxValueGlobal << std::endl;
            std::cout << "   minValue:       " << minValue << "\tmaxValue:       " << maxValue << std::endl;
            std::cout << "   width:          " << width << "\theight:         " << height << std::endl;

            cv::Mat imageL = cv::Mat((int)height, (int)width, CV_32FC1, data); // rows, cols (height, width)
            cv::Mat imageG;
            // cv::Size(width, height)
            imageL.convertTo(imageG, CV_8UC1, 255.0 / (maxValueGlobal - minValueGlobal),
                             -minValueGlobal * 255.0 / (maxValueGlobal - minValueGlobal));
            imageL.convertTo(imageL, CV_8UC1, 255.0 / (maxValue - minValue), -minValue * 255.0 / (maxValue - minValue));

            cv::Mat imageLUP;
            cv::Mat imageGUP;

            if (width > height) {
                int dstWidth = 300;
                cv::Size size(dstWidth, (int)ceil((double)imageL.size().height * dstWidth / imageL.size().width));
                // Set original size
                size = imageL.size();
                cv::resize(imageL, imageLUP, size);
                cv::resize(imageG, imageGUP, size);
            } else {
                int dstHeight = 300;
                cv::Size size((int)ceil((double)imageL.size().width * dstHeight / imageL.size().height), dstHeight);
                // Set original size
                size = imageL.size();
                cv::resize(imageL, imageLUP, size);
                cv::resize(imageG, imageGUP, size);
            }

            cv::applyColorMap(imageL, imageL, cv::COLORMAP_JET);
            cv::applyColorMap(imageG, imageG, cv::COLORMAP_JET);

            cv::applyColorMap(imageLUP, imageLUP, cv::COLORMAP_JET);
            cv::applyColorMap(imageGUP, imageGUP, cv::COLORMAP_JET);

            cv::imshow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), imageLUP);
            cv::imshow(datasetName + " global " + cutType + " " + std::to_string(cutIndex), imageGUP);

            // cv::moveWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), 100, 50);
            // cv::moveWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex), imageL.size().width +
            // 50 + 100, 50);

            // cv::imwrite(datasetName + " local " + cutType + " " + std::to_string(cutIndex) + ".png", imageL);
            // cv::imwrite(datasetName + " global " + cutType + " " + std::to_string(cutIndex) + ".png", imageG);

            imageLUP.release();
            imageGUP.release();
            imageG.release();
            imageL.release();
            delete[] data;
        } else {
            std::cout << "Dataset is not displayable" << std::endl;
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    cv::waitKey(0);
    cv::destroyAllWindows();
}

/**
 * @brief copyDimensionsAndAttributes Copy dimensions and attributes
 * @param hDF5SimOutputFile
 * @param hDF5OutputFile
 */
void copyDimensionsAndAttributes(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5OutputFile)
{
    // Copy nT, nX, nY, nZ
    try {
        HDF5Helper::HDF5Vector3D size(1, 1, 1);
        hsize_t data[1];
        data[0] = hDF5SimOutputFile->getNT();
        hDF5OutputFile->createDatasetI(NT_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        hDF5OutputFile->openDataset(NT_DATASET)
            ->write3DDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), data);
        data[0] = hDF5SimOutputFile->getNX();
        hDF5OutputFile->createDatasetI(NX_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        hDF5OutputFile->openDataset(NX_DATASET)
            ->write3DDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), data);
        data[0] = hDF5SimOutputFile->getNY();
        hDF5OutputFile->createDatasetI(NY_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        hDF5OutputFile->openDataset(NY_DATASET)
            ->write3DDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), data);
        data[0] = hDF5SimOutputFile->getNZ();
        hDF5OutputFile->createDatasetI(NZ_DATASET, size, HDF5Helper::HDF5Vector3D(), true);
        hDF5OutputFile->openDataset(NZ_DATASET)
            ->write3DDataset(HDF5Helper::HDF5Vector3D(), HDF5Helper::HDF5Vector3D(1, 1, 1), data);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Copy root (info) attributes to destination h5 file
    try {
        HDF5Helper::HDF5Group *srcGroup = hDF5SimOutputFile->openGroup("/");
        HDF5Helper::HDF5Group *dstGroup = hDF5OutputFile->openGroup("/");
        for (unsigned int i = 0; i < srcGroup->getNumAttrs(); i++) {
            HDF5Helper::HDF5Attribute *attr = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attr, false);
            delete attr;
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief main Main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
    const int buflen = 512;
    char hostname[buflen];
    gethostname(hostname, buflen);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mPISize);
    MPI_Comm_rank(comm, &mPIRank);

    MPI_Info_create(&info);
    MPI_Info_set(info, ((char *)"ibm_largeblock_io"), ((char *)"true"));

    std::cout << "process rank " << mPIRank << " of " << mPISize << " on host " << hostname << std::endl;

    /*if (mPIRank != 0) {
        std::ofstream   fout("/dev/null");
        std::cout.rdbuf(fout.rdbuf()); // redirect 'cout' to a 'fout'
        std::cerr.rdbuf(fout.rdbuf()); // redirect 'cerr' to a 'fout'
        std::clog.rdbuf(fout.rdbuf()); // redirect 'clog' to a 'fout'
    }*/

    double t0 = HDF5Helper::getTime();

    HDF5Helper::File *hDF5SimOutputFile = NULL;
    HDF5Helper::File *hDF5SimInputFile  = NULL;
    HDF5Helper::File *hDF5OutputFile    = NULL;
    HDF5Helper::File *hDF5ViewFile      = NULL;

    // Create helper class for datasets of various types
    DtsForPcs *dtsForPcs              = new DtsForPcs();
    dtsForPcs->sensorMaskIndexDataset = NULL;
    dtsForPcs->sensorMaskSize         = 0;

    // Load parameters
    getParams(argc, argv);

    // Load simulation output file
    if (simulationOutputFilename.empty()) {
        std::cerr << "Missing parameter -f (simulation output filename)" << std::endl;
        std::cout << help() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        printDebugTitle("Load simulation output file");
        hDF5SimOutputFile = loadSimulationFile(simulationOutputFilename);
    }

    // Load simulation input file
    if (!simulationInputFilename.empty()) {
        printDebugTitle("Load simulation input file");
        hDF5SimInputFile = loadSimulationFile(simulationInputFilename);
    }

    // Find and get sensor mask dataset
    dtsForPcs->sensorMaskIndexDataset = findAndGetSensorMaskIndexDataset(hDF5SimOutputFile, hDF5SimInputFile);
    if (dtsForPcs->sensorMaskIndexDataset != NULL) {
        // Get sensor mask size
        HDF5Helper::HDF5Vector3D size = dtsForPcs->sensorMaskIndexDataset->getDims();
        if (dtsForPcs->sensorMaskIndexDataset->getRank() == 3 && size.z() == 1 && size.y() == 1) {
            dtsForPcs->sensorMaskSize = size.x();
        } else {
            std::cerr << "Wrong sensor mask" << std::endl;
            if (flagReshape)
                exit(EXIT_FAILURE);
        }
    } else {
        // std::cerr << "Sensor mask is not in simulation output or input file" << std::endl;
        if (flagReshape)
            exit(EXIT_FAILURE);
    }

    // Find datasets for visualization to edit
    printDebugTitle("Find datasets for visualization for processing");
    findDatasetsForProcessing(hDF5SimOutputFile, dtsForPcs);

    // Test dataset reading
    if (flagTest) {
        printDebugTitle("Testing");
        testOfReading(dtsForPcs);
    }

    if (flagReshape || flagRechunk || flagDwnsmpl) {
        // Create new file
        printDebugTitle("Create or open output file");
        hDF5OutputFile = createOrOpenOutputFile(outputFilename);
    }

    // Processing of sensor mask
    if (flagReshape) {
        printDebugTitle("Reshaping");
        reshape(hDF5SimOutputFile, hDF5OutputFile, dtsForPcs);
    }

    // Downsampling
    if (flagDwnsmpl) {
        printDebugTitle("Downsampling");
        downsampling(hDF5SimOutputFile, hDF5OutputFile, dtsForPcs);
    }

    // Copy 3D datasets a set new chunking
    if (flagRechunk) {
        printDebugTitle("Change chunks");
        changeChunks(hDF5OutputFile, dtsForPcs);
    }

    if (flagReshape || flagRechunk || flagDwnsmpl) {
        // Copy dimensions and attributes
        printDebugTitle("Copy dimensions and attributes");
        copyDimensionsAndAttributes(hDF5SimOutputFile, hDF5OutputFile);
    }

    // Select file to view
    if (!outputFilename.empty() && (flagRechunk || flagReshape || flagDwnsmpl))
        hDF5ViewFile = hDF5OutputFile;
    else
        hDF5ViewFile = hDF5SimOutputFile;

    // Simple slice visualization
    if (flagView) {
        printDebugTitle("Visualization");
        visualization(hDF5ViewFile);
    }

    printDebugTitle("Closing files");

    MPI_Barrier(comm);

    // Close files
    delete hDF5SimOutputFile;
    delete hDF5SimInputFile;
    delete hDF5OutputFile;

    MPI_Barrier(comm);

    double t2 = HDF5Helper::getTime();

    std::cout << std::endl
              << std::endl
              << "Time of the entire process: " << (t2 - t0) << " ms; \t" << std::endl
              << std::endl
              << std::endl;

    MPI_Finalize();

    std::exit(EXIT_SUCCESS);
}
