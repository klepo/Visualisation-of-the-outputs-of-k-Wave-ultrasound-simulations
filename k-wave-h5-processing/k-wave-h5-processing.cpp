/*
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing k-Wave HDF5 processing application.
 *
 * @section     Licence
 * This application is for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave h5 processing is free software.
 */

#include <HDF5File.h>
#include <HDF5Dataset.h>
#include <HDF5Group.h>
#include <HDF5Attribute.h>

#include <iostream>
#include <string>
#include <map>
#include <math.h>
#include <time.h>
#include <list>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");
const std::string NT_DATASET("Nt");
const std::string NX_DATASET("Nx");
const std::string NY_DATASET("Ny");
const std::string NZ_DATASET("Nz");

#define MAX_SIZE 512
#define MAX_CHUNK_SIZE 64
#define MAX_NUMBER_OF_FRAMES 0 // TODO

// Filenames
std::string simulationOutputFilename = "";
std::string simulationInputFilename = "";
std::string outputFilename = "";

// Size vars
hsize_t maxSize = MAX_SIZE;
hsize_t maxChunkSize = MAX_CHUNK_SIZE;

// Filter by names
std::list<std::string> names;
bool flagNames = false;

// Params for visualize
std::string datasetName = "";
std::string cutType = "YX";
hsize_t cutIndex = 0;

// Application modes
bool flagTest = false;
bool flagReshape = false;
bool flagRechunk = false;
bool flagView = false;
bool flagDwnsmpl = false;

double t0 = HDF5File::getTime();

/**
 * @brief help
 * @return help string
 */
std::string help()
{
    return "\n"
    "Usage: k-wave-h5-processing [options]\n"
    "where options include:\n\n"
    "  -f HDF5SimulationOutputFilename ...... Required parameter.\n"
    "                                         HDF5 file with simulation results.\n"
    "\n"
    "  -m HDF5SimulationInputFilename ....... Optional parameter. Simulation HDF5 input file\n"
    "                                         with sensor_mask_index dataset.\n"
    "\n"
    "  -o HDF5OutputFilename ................ Optional parameter - output filename. Default value is\n"
    "                                         HDF5SimulationOutputFilename_modified.\n"
    "\n"
    "  -reshape ............................. Optional parameter. Performs processing sensor mask\n"
    "                                         type datasets to group with series of 3D datasets\n"
    "                                         and saves datasets to a new file.\n"
    "                                         In hDF5SimulationOutputFile or hDF5SimulationInputFile\n"
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
    "  -s size .............................. Optional parameter. Max size for donwsampling.\n"
    "\n"
    "  -ch chunkSize ........................ Optional parameter. Size for new chunks from 1 to\n"
    "                                         maximal appropriately value.\n"
    "\n"
    "  -names name1;name2; .................. Optional parameter. Names of selected datasets or\n"
    "                                         groups for processing.\n"
    "\n"
    "  -test ................................ Test mode. Reading test of 3D type dtatasets is performed.\n"
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
        } else if (strcmp("-m", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -m (simulation iutput filename)" << std::endl;
                std::cout << help() << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationInputFilename = argv[i];
            std::cout << "\n  Simulation iutput filename:\n    " << simulationInputFilename << std::endl;
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
            } catch (std::invalid_argument error) {
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
            while(getline(iss, token, ';')) {
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
    return (hsize_t) floor(number + 0.5);
}

/**
 * @brief The DatasetsForProcessing class Helper class for manipulation with datasets for processing
 */
class DatasetsForProcessing
{
public:
    HDF5File::HDF5Dataset *sensorMaskIndexDataset;
    uint64_t sensorMaskSize;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasetsMaskType;
    std::map<const H5std_string, HDF5File::HDF5Group *> datasetsGroupType;
    std::map<const H5std_string, HDF5File::HDF5Group *> datasetsGroupTypeDwnsmpl;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasets3DType;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasets3DTypeDwnsmpl;
};

/**
 * @brief loadSimulationFile Load simulation file by filename
 * @param simulationFilename path to HDF5 filename
 * @return file
 */
HDF5File *loadSimulationFile(std::string simulationFilename)
{
    HDF5File *hDF5SimulationFile = NULL;
    try {
        hDF5SimulationFile = new HDF5File(simulationFilename, HDF5File::OPEN);
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return hDF5SimulationFile;
}

/**
 * @brief findAndGetSensorMaskDataset Find and get sensor mask dataset
 * @param hDF5SimulationOutputFile
 * @param hDF5SimulationInputFile
 * @return sensor_mask_index dataset
 */
HDF5File::HDF5Dataset *findAndGetSensorMaskDataset(HDF5File *hDF5SimulationOutputFile, HDF5File *hDF5SimulationInputFile)
{
    HDF5File::HDF5Dataset *sensorMaskIndexDataset = NULL;
    std::cout << std::endl << std::endl << "---- Find and get sensor mask dataset ----" << std::endl << std::endl << std::endl;
    try {
        // Try to load sensor mask from simulation output file
        sensorMaskIndexDataset = hDF5SimulationOutputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
    } catch(std::exception &) {
        std::cout << "Sensor mask is not in simulation output file" << std::endl;
        // Try to load sensor mask from simulation input file
        try {
            sensorMaskIndexDataset = hDF5SimulationInputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
        } catch(std::exception &) {
            std::cout << "Sensor mask is not in simulation input file" << std::endl;
        }
    }
    return sensorMaskIndexDataset;
}

/**
 * @brief createOrOpenOutputFile
 * @param outputFilename
 */
HDF5File *createOrOpenOutputFile(std::string outputFilename) {
    std::string filename = "";
    HDF5File *file = NULL;
    if (outputFilename.empty()) {
        // Create auto filename
        size_t lastindex = simulationOutputFilename.find_last_of(".");
        std::string rawname = simulationOutputFilename.substr(0, lastindex);
        filename = rawname + "_modified.h5";
    } else {
        filename = outputFilename;
    }

    try {
        // Try open file
        file = new HDF5File(filename, HDF5File::OPEN);
    } catch(std::exception &) {
        try {
            // Try create file
            file = new HDF5File(filename, HDF5File::CREATE);
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    return file;
}

/**
 * @brief findDatasetsForProcessing Find datasets for processing
 * @param hDF5SimulationOutputFile
 * @param datasetsForProcessing
 */
void findDatasetsForProcessing(HDF5File *hDF5SimulationOutputFile, DatasetsForProcessing *datasetsForProcessing)
{
    for (hsize_t i = 0; i < hDF5SimulationOutputFile->getNumObjs(); i++) {
        H5G_obj_t type = hDF5SimulationOutputFile->getObjTypeById(i);

        // Datasets
        if (type == H5G_DATASET) {
            HDF5File::HDF5Dataset *dataset = hDF5SimulationOutputFile->openDataset(i);
            hsize_t *size = dataset->getDims();

            // Filter by selected names
            if (flagNames && std::find(names.begin(), names.end(), dataset->getName()) == names.end()) {
                hDF5SimulationOutputFile->closeDataset(dataset->getName());
                continue;
            }

            // 3D type
            if (dataset->getDataType() == H5T_FLOAT
                    && dataset->getRank() == 3
                    && size[0] == hDF5SimulationOutputFile->getNZ()
                    && size[1] == hDF5SimulationOutputFile->getNY()
                    && size[2] == hDF5SimulationOutputFile->getNX()) {
                datasetsForProcessing->datasets3DType.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dataset->getName(), dataset));
                std::cout << "----> 3D type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl << std::endl;
            }
            // Downsampled 3D type
            else if (dataset->hasAttribute("dwnsmpl")
                     && dataset->getDataType() == H5T_FLOAT
                     && dataset->getRank() == 3
                     && size[0] < hDF5SimulationOutputFile->getNZ()
                     && size[1] < hDF5SimulationOutputFile->getNY()
                     && size[2] < hDF5SimulationOutputFile->getNX()) {
                datasetsForProcessing->datasets3DTypeDwnsmpl.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dataset->getName(), dataset));
                std::cout << "----> 3D type downsampled dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl << std::endl;
            }
            // Sensor mask type
            else if (dataset->getDataType() == H5T_FLOAT
                     && datasetsForProcessing->sensorMaskIndexDataset != NULL
                     && dataset->getRank() == 3
                     && size[0] == 1
                     && size[1] <= hDF5SimulationOutputFile->getNT()
                     && size[2] == datasetsForProcessing->sensorMaskSize) {
                datasetsForProcessing->datasetsMaskType.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset *>(dataset->getName(), dataset));
                std::cout << "----> Mask type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl << std::endl;
            }
            // Unknown type
            else {
                hDF5SimulationOutputFile->closeDataset(dataset->getName());
            }
        }
        // Groups
        else if (type == H5G_GROUP) {
            // Reshaped mask type to group
            try {
                HDF5File::HDF5Group *group = hDF5SimulationOutputFile->openGroup(i);

                // Filter by selected names
                if (flagNames && std::find(names.begin(), names.end(), group->getName()) == names.end()) {
                    hDF5SimulationOutputFile->closeGroup(group->getName());
                    continue;
                }

                // Reshaped mask type group downsampled
                if (group->hasAttribute("src_group_id")) {
                    uint64_t count = group->readAttributeI("count");
                    uint64_t posX = group->readAttributeI("positionX");
                    uint64_t posY = group->readAttributeI("positionY");
                    uint64_t posZ = group->readAttributeI("positionZ");
                    datasetsForProcessing->datasetsGroupTypeDwnsmpl.insert(std::pair<const H5std_string, HDF5File::HDF5Group *>(group->getName(), group));
                    std::cout << "----> Reshaped mask type group downsampled: "<< group->getName() << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl << std::endl;
                }
                // Reshaped mask type group
                else if (group->hasAttribute("count")) {
                    uint64_t count = group->readAttributeI("count");
                    uint64_t posX = group->readAttributeI("positionX");
                    uint64_t posY = group->readAttributeI("positionY");
                    uint64_t posZ = group->readAttributeI("positionZ");
                    datasetsForProcessing->datasetsGroupType.insert(std::pair<const H5std_string, HDF5File::HDF5Group *>(group->getName(), group));
                    std::cout << "----> Reshaped mask type group: "<< group->getName() << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl << std::endl;
                }
                // Unknown type
                else {
                    hDF5SimulationOutputFile->closeGroup(group->getName());
                }

            } catch(std::exception &) {
                std::cout << "Object " << hDF5SimulationOutputFile->getObjNameById(i) << " is not reshaped group" << std::endl;
            }
        }
    }
}

void testOfReading(DatasetsForProcessing *datasetsForProcessing)
{
    // Check number of datasets
    if (datasetsForProcessing->datasets3DType.empty()/* && datasetsForProcessing->datasetsGroupType.empty()*/) {
        std::cout << "No dataset for test in simulation output file" << std::endl;
    } else {
        // For every 3D type dataset
        for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsForProcessing->datasets3DType.begin(); it != datasetsForProcessing->datasets3DType.end(); ++it) {
            try {
                HDF5File::HDF5Dataset *dataset = it->second;

                std::cout << "Dataset: " << dataset->getName() << std::endl << std::endl;

                float minValue = 0;
                float maxValue = 0;
                hsize_t *size = dataset->getDims();
                std::cout << "Dataset size:       " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                hsize_t *chunkSize = dataset->getChunkDims();
                std::cout << "Dataset chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;

                std::cout << "Getting and setting global min and max values..." << std::endl;
                dataset->findAndSetGlobalMinAndMaxValue();
                float minValueGlobal = dataset->getGlobalMinValueF();
                float maxValueGlobal = dataset->getGlobalMaxValueF();

                std::cout << "   minValueGlobal: " << minValueGlobal << "\tmaxValueGlobal: " << maxValueGlobal << std::endl;

                std::cout << std::endl;

                float *data = NULL;
                uint64_t height = 0;
                uint64_t width = 0;

                // XY
                dataset->read3DDataset(0, 0, 0, 1, size[1], size[2], data, minValue, maxValue);
                height = size[1];
                width = size[2];
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete [] data;

                // XZ
                dataset->read3DDataset(0, 0, 0, size[0], 1, size[2], data, minValue, maxValue);
                height = size[0];
                width = size[2];
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete [] data;

                // YZ
                dataset->read3DDataset(0, 0, 0, size[0], size[1], 1, data, minValue, maxValue);
                height = size[0];
                width = size[1];
                //std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
                //std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;
                delete [] data;


                std::cout << std::endl;

            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * @brief reshape Performs reshaping of sensor mask type datasets
 * @param hDF5SimulationOutputFile
 * @param hDF5OutputFile
 * @param datasetsForProcessing
 */
void reshape(HDF5File *hDF5SimulationOutputFile, HDF5File * hDF5OutputFile, DatasetsForProcessing *datasetsForProcessing)
{
    // Check number of datasets
    if (datasetsForProcessing->datasetsMaskType.empty()) {
        std::cout << "No dataset for reshape in simuation output file" << std::endl;
    } else {
        try {
            uint64_t *data;
            uint64_t minV, maxV;
            hsize_t xO, yO, zO, xC, yC, zC;
            hsize_t minX = hDF5SimulationOutputFile->getNX();
            hsize_t minY = hDF5SimulationOutputFile->getNY();
            hsize_t minZ = hDF5SimulationOutputFile->getNZ();
            hsize_t maxX = 0, maxY = 0, maxZ = 0;
            datasetsForProcessing->sensorMaskIndexDataset->initBlockReading();

            // Find min and max position from linear saved values
            hsize_t index = 0, zM = 0, yM = 0, xM = 0;
            do {
                datasetsForProcessing->sensorMaskIndexDataset->readBlock(zO, yO, xO, zC, yC, xC, data, minV, maxV);
                for (hsize_t z = 0; z < zC; z++)
                    for (hsize_t y = 0; y < yC; y++)
                        for (hsize_t x = 0; x < xC; x++) {
                            // [x + y * xC + z * xC * yC] is from 0, but value of index is from 1
                            index = data[x + y * xC + z * xC * yC];
                            hDF5SimulationOutputFile->convertlinearTo3D(index, zM, yM, xM);
                            if (xM < minX) minX = xM;
                            if (yM < minY) minY = yM;
                            if (zM < minZ) minZ = zM;
                            if (xM > maxX) maxX = xM;
                            if (yM > maxY) maxY = yM;
                            if (zM > maxZ) maxZ = zM;
                        }
                delete [] data; // !!
            } while (!datasetsForProcessing->sensorMaskIndexDataset->isLastBlock());
            std::cout << "   min point:\t" << " z: " << minZ << "\ty: " << minY << "\tx: " << minX << std::endl;
            std::cout << "   max point:\t" << " z: " << maxZ << "\ty: " << maxY << "\tx: " << maxX << std::endl;

            // TODO --> unused
            // Find first and last point
            /*hsize_t *size = datasetsForProcessing->sensorMaskIndexDataset->getDims();
            hsize_t firstX = 0, firstY = 0, firstZ = 0, lastX = 0, lastY = 0, lastZ = 0;
            // Get data for first point
            datasetsForProcessing->sensorMaskIndexDataset->read3DDataset(0, 0, 0, 1, 1, 1, data, minV, maxV);
            hDF5SimulationOutputFile->convertlinearTo3D(data[0], firstZ, firstY, firstX);
            delete [] data; // !!
            // Get data for last point
            datasetsForProcessing->sensorMaskIndexDataset->read3DDataset(size[0]-1, size[1]-1, size[2]-1, 1, 1, 1, data, minV, maxV);
            hDF5SimulationOutputFile->convertlinearTo3D(data[0], lastZ, lastY, lastX);
            delete [] data; // !!
            std::cout << "   first point:\t" << " z: " << firstZ << "\ty: " << firstY << "\tx: " << firstX << std::endl;
            std::cout << "   last point:\t" << " z: " << lastZ << "\ty: " << lastY << "\tx: " << lastX << std::endl;*/
            // unused

            // Compute chunk size according to min/max position
            hsize_t chunkSize[3];
            chunkSize[0] = std::min(maxChunkSize, maxZ - minZ + 1);
            chunkSize[1] = std::min(maxChunkSize, maxY - minY + 1);
            chunkSize[2] = std::min(maxChunkSize, maxX - minX + 1);
            // and dataset size
            hsize_t datasetSize[3] = {maxZ - minZ + 1, maxY - minY + 1, maxX - minX + 1};

            std::cout << "   new chunk size:\t" << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;

            // For every mask type dataset
            for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsForProcessing->datasetsMaskType.begin(); it != datasetsForProcessing->datasetsMaskType.end(); ++it) {
                HDF5File::HDF5Dataset *dataset = it->second;

                uint64_t *sensorMaskData;
                float *datasetData;
                uint64_t minVI, maxVI;
                float minVF = 0, maxVF = 0;
                float minVFG = 0, maxVFG = 0;
                bool first = true;
                hsize_t xO, yMO, yDO, zO;
                hsize_t xC, yC, zC;

                datasetsForProcessing->sensorMaskIndexDataset->initBlockReading();
                // Set same block size as sensorMaskIndexDataset
                dataset->initBlockReading(datasetsForProcessing->sensorMaskIndexDataset->getBlockSize());

                // Save attributes to new group
                hDF5OutputFile->createGroup(dataset->getName(), true);
                HDF5File::HDF5Group *group = hDF5OutputFile->openGroup(dataset->getName());
                group->setAttribute("positionZ", (uint64_t) minZ);
                group->setAttribute("positionY", (uint64_t) minY);
                group->setAttribute("positionX", (uint64_t) minX);
                group->setAttribute("sizeZ", (uint64_t) datasetSize[0]);
                group->setAttribute("sizeY", (uint64_t) datasetSize[1]);
                group->setAttribute("sizeX", (uint64_t) datasetSize[2]);

                // First dataset in group
                hDF5OutputFile->createDatasetF(dataset->getName()  + "/" + std::to_string(0), 3, datasetSize, chunkSize, true);
                // Set as actual
                HDF5File::HDF5Dataset *actualDataset = hDF5OutputFile->openDataset(dataset->getName()  + "/" + std::to_string(0));

                hsize_t index = 0, zM = 0, yM = 0, xM = 0;
                float data[1];

                do {
                    // Next time step (yDO)
                    if (datasetsForProcessing->sensorMaskIndexDataset->isLastBlock()) {
                        /*if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                            if (yDO + 1 >= MAX_NUMBER_OF_FRAMES)
                                break;*/

                        datasetsForProcessing->sensorMaskIndexDataset->initBlockReading();
                        actualDataset->findAndSetGlobalMinAndMaxValue();

                        if (first){
                            minVFG = actualDataset->getGlobalMinValueF();
                            maxVFG = actualDataset->getGlobalMaxValueF();
                        }
                        first = false;

                        if (minVFG > actualDataset->getGlobalMinValueF()) minVFG = actualDataset->getGlobalMinValueF();
                        if (maxVFG < actualDataset->getGlobalMaxValueF()) maxVFG = actualDataset->getGlobalMaxValueF();

                        // Shift to next dataset -> step
                        hDF5OutputFile->closeDataset(dataset->getName()  + "/" + std::to_string(yDO));
                        hDF5OutputFile->createDatasetF(dataset->getName()  + "/" + std::to_string(yDO+1), 3, datasetSize, chunkSize, true);
                        actualDataset = hDF5OutputFile->openDataset(dataset->getName()  + "/" + std::to_string(yDO+1));
                    }

                    // Offset is unused (zO, yMO, xO)
                    datasetsForProcessing->sensorMaskIndexDataset->readBlock(zO, yMO, xO, zC, yC, xC, sensorMaskData, minVI, maxVI);
                    dataset->readBlock(zO, yDO, xO, zC, yC, xC, datasetData, minVF, maxVF);

                    double t4 = HDF5File::getTime();
                    // For the entire block write "voxels"
                    for (hsize_t z = 0; z < zC; z++)
                        for (hsize_t y = 0; y < yC; y++)
                            for (hsize_t x = 0; x < xC; x++) {
                                index = sensorMaskData[x + y * xC + z * xC * yC];
                                hDF5SimulationOutputFile->convertlinearTo3D(index, zM, yM, xM);
                                data[0] = datasetData[x + y * xC + z * xC * yC];
                                // Save from position (0,0,0)
                                actualDataset->write3DDataset(zM - minZ, yM - minY, xM - minX, 1, 1, 1, data, false);
                            }
                    double t5 = HDF5File::getTime();

                    std::cout << "write time: " << (t5-t4) << " ms; \t" << std::endl;

                    delete [] sensorMaskData;
                    delete [] datasetData;

                } while (!dataset->isLastBlock());

                actualDataset->findAndSetGlobalMinAndMaxValue();

                if (first){
                    minVFG = actualDataset->getGlobalMinValueF();
                    maxVFG = actualDataset->getGlobalMaxValueF();
                }
                first = false;

                // min/max values
                if (minVFG > actualDataset->getGlobalMinValueF()) minVFG = actualDataset->getGlobalMinValueF();
                if (maxVFG < actualDataset->getGlobalMaxValueF()) maxVFG = actualDataset->getGlobalMaxValueF();

                // Save some attributes
                group->setAttribute("min", minVFG);
                group->setAttribute("max", maxVFG);
                group->setAttribute("count", (uint64_t) yDO + 1);

                hDF5OutputFile->closeDataset(actualDataset->getName());

                // Filter or add to map for next processing
                if (!flagNames || std::find(names.begin(), names.end(), group->getName()) != names.end())
                    datasetsForProcessing->datasetsGroupType.insert(std::pair<const H5std_string, HDF5File::HDF5Group *>(group->getName(), group));

            }
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief downsampling Performs downsampling of datasets
 * @param hDF5SimulationOutputFile
 * @param hDF5OutputFile
 * @param datasetsForProcessing
 */
void downsampling(HDF5File *hDF5SimulationOutputFile, HDF5File *hDF5OutputFile, DatasetsForProcessing *datasetsForProcessing)
{
    try {
        // Check number of datasets for downsamling
        if (datasetsForProcessing->datasets3DType.empty() && datasetsForProcessing->datasetsGroupType.empty()) {
            std::cout << "No dataset for downsampling in simulation output file" << std::endl;
        }
        // Check current size -> is greater?
        else if (std::max(std::max(hDF5SimulationOutputFile->getNZ(), hDF5SimulationOutputFile->getNY()), hDF5SimulationOutputFile->getNX()) <= maxSize) {
            std::cout << "No dataset for downsampling - max(nZ, nY, nX) == " + std::to_string(std::max(std::max(hDF5SimulationOutputFile->getNZ(), hDF5SimulationOutputFile->getNY()), hDF5SimulationOutputFile->getNX())) + " <= " + std::to_string(maxSize) << std::endl;
        }
        else {
            // Compute new size
            // Get max dimension
            hsize_t nMax = std::max(std::max(hDF5SimulationOutputFile->getNZ(), hDF5SimulationOutputFile->getNY()), hDF5SimulationOutputFile->getNX());
            double ratio = (double) maxSize / nMax;
            hsize_t nZd = myRound(hDF5SimulationOutputFile->getNZ() * ratio);
            hsize_t nYd = myRound(hDF5SimulationOutputFile->getNY() * ratio);
            hsize_t nXd = myRound(hDF5SimulationOutputFile->getNX() * ratio);
            // Check < 1
            if (nZd < 1) nZd = 1;
            if (nYd < 1) nYd = 1;
            if (nXd < 1) nXd = 1;
            hsize_t newDatasetSize[3] = {nZd, nYd, nXd};
            // Tmp has original Z dimension (hDF5SimulationOutputFile->getNZ())
            hsize_t newTmpDatasetSize[3] = {hDF5SimulationOutputFile->getNZ(), nYd, nXd};
            // Chunk size
            hsize_t chunkSize[3] = {maxChunkSize, maxChunkSize, maxChunkSize};
            // Check bigger chunk size
            if (chunkSize[0] > nZd) chunkSize[0] = nZd;
            if (chunkSize[1] > nYd) chunkSize[1] = nYd;
            if (chunkSize[2] > nXd) chunkSize[2] = nXd;
            std::cout << "   new size:\t" << newDatasetSize[0] << " x " << newDatasetSize[1] << " x " << newDatasetSize[2] << std::endl;

            // Create temp file
            HDF5File *tmpFile = new HDF5File("tmp.h5", HDF5File::CREATE);

            // For every 3D type dataset
            for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsForProcessing->datasets3DType.begin(); it != datasetsForProcessing->datasets3DType.end(); ++it) {
                HDF5File::HDF5Dataset *srcDataset = it->second;

                tmpFile->createDatasetF(srcDataset->getName(), srcDataset->getRank(), newTmpDatasetSize);
                HDF5File::HDF5Dataset *dstDataset = tmpFile->openDataset(srcDataset->getName());
                hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(maxSize), srcDataset->getRank(), newDatasetSize, chunkSize, true);
                HDF5File::HDF5Dataset *dstDatasetFinal = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(maxSize));

                //float *srcData;
                float minV = 0, maxV = 0;
                float minVG = 0, maxVG = 0;

                // First 2D slices in XY plane
                for (unsigned int z = 0; z < hDF5SimulationOutputFile->getNZ(); z++) {
                    float *srcData;
                    srcDataset->read3DDataset(z, 0, 0, 1, hDF5SimulationOutputFile->getNY(), hDF5SimulationOutputFile->getNX(), srcData, minV, maxV);
                    cv::Mat image = cv::Mat((int) hDF5SimulationOutputFile->getNY(), (int) hDF5SimulationOutputFile->getNX(), CV_32FC1, srcData); // rows, cols (height, width)
                    cv::resize(image, image, cv::Size((int) nXd, (int) nYd), 0, 0, cv::INTER_NEAREST);
                    dstDataset->write3DDataset(z, 0, 0, 1, nYd, nXd, (float *) image.data);
                    image.release();
                    delete [] srcData;
                    srcData =  NULL;
                }

                // and after 2d slices XZ plane
                for (unsigned int y = 0; y < nYd; y++) {
                    float *srcData;
                    dstDataset->read3DDataset(0, y, 0, hDF5SimulationOutputFile->getNZ(), 1, nXd, srcData, minV, maxV);
                    cv::Mat image = cv::Mat((int) hDF5SimulationOutputFile->getNZ(), (int) nXd, CV_32FC1, srcData); // rows, cols (height, width)
                    cv::resize(image, image, cv::Size((int) nXd, (int) nZd), 0, 0, cv::INTER_NEAREST);
                    dstDatasetFinal->write3DDataset(0, y, 0, nZd, 1, nXd, (float *) image.data, true);

                    dstDatasetFinal->getMinAndMaxValue((float *) image.data, nZd * nXd, minV, maxV);
                    if (y == 0) {
                        minVG = minV;
                        maxVG = maxV;
                    } else {
                        if (minV < minVG) minVG = minV;
                        if (maxV > maxVG) maxVG = maxV;
                    }

                    image.release();
                    delete [] srcData;
                    srcData =  NULL;
                }

                // Save attributes
                dstDatasetFinal->setAttribute("min", minVG);
                dstDatasetFinal->setAttribute("max", maxVG);
                dstDatasetFinal->setAttribute("dwnsmpl", (uint64_t) maxSize);
                dstDatasetFinal->setAttribute("src_dataset_name", srcDataset->getName());
                dstDatasetFinal->setAttribute("src_dataset_id", (uint64_t) srcDataset->getId());

                // Filter or add to map
                if (!flagNames || std::find(names.begin(), names.end(), dstDatasetFinal->getName()) != names.end())
                    datasetsForProcessing->datasets3DTypeDwnsmpl.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dstDatasetFinal->getName(), dstDatasetFinal));
            }

            // For every reshaped mask type
            for (std::map<const H5std_string, HDF5File::HDF5Group *>::iterator it = datasetsForProcessing->datasetsGroupType.begin(); it != datasetsForProcessing->datasetsGroupType.end(); ++it) {
                HDF5File::HDF5Group *srcGroup = it->second;

                // Create group in tmp file
                tmpFile->createGroup(srcGroup->getName());
                // Create group (dstGroupFinal) in hDF5OutputFile
                hDF5OutputFile->createGroup(srcGroup->getName() + "_" + std::to_string(maxSize), true);
                HDF5File::HDF5Group *dstGroupFinal = hDF5OutputFile->openGroup(srcGroup->getName() + "_" + std::to_string(maxSize));

                // Get datasets count
                hsize_t count = srcGroup->getNumObjs();

                float minVG, maxVG;

                // Get dimensions and min/max values
                HDF5File::HDF5Dataset *tmp = hDF5OutputFile->openDataset(srcGroup->getName() + "/0");
                minVG = tmp->readAttributeF("min");
                maxVG = tmp->readAttributeF("max");
                hsize_t *dims = tmp->getDims();
                hsize_t nX = dims[2];
                hsize_t nY = dims[1];
                hsize_t nZ = dims[0];
                hDF5OutputFile->closeDataset(srcGroup->getName() + "/0");

                // Check current size
                if (std::max(std::max(dims[0], dims[1]), dims[2]) <= maxSize) {
                    std::cout << "No dataset for downsampling - " + std::to_string(std::max(std::max(dims[0], dims[1]), dims[2])) + " <= " + std::to_string(maxSize) << std::endl;
                    break;
                }

                // Compute new size
                // Get max dimension
                hsize_t nMax = std::max(std::max(nZ, nY), nX);
                double ratio = (double) maxSize / nMax;
                hsize_t nZd = myRound(nZ * ratio);
                hsize_t nYd = myRound(nY * ratio);
                hsize_t nXd = myRound(nX * ratio);
                // Check < 1
                if (nZd < 1) nZd = 1;
                if (nYd < 1) nYd = 1;
                if (nXd < 1) nXd = 1;
                hsize_t newDatasetSize[3] = {nZd, nYd, nXd};
                // Tmp has original Z dimension (nZ)
                hsize_t newTmpDatasetSize[3] = {nZ, nYd, nXd};
                // Chunk size
                hsize_t chunkSize[3] = {maxChunkSize, maxChunkSize, maxChunkSize};
                if (chunkSize[0] > nZd) chunkSize[0] = nZd;
                if (chunkSize[1] > nYd) chunkSize[1] = nYd;
                if (chunkSize[2] > nXd) chunkSize[2] = nXd;
                std::cout << "   new size:\t" << newDatasetSize[0] << " x " << newDatasetSize[1] << " x " << newDatasetSize[2] << std::endl;

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5File::HDF5Dataset *srcDataset = hDF5OutputFile->openDataset(srcGroup->getName() + "/" + std::to_string(i));

                    tmpFile->createDatasetF(srcDataset->getName(), srcDataset->getRank(), newTmpDatasetSize);
                    HDF5File::HDF5Dataset *dstDataset = tmpFile->openDataset(srcDataset->getName());
                    hDF5OutputFile->createDatasetF(srcGroup->getName() + "_" + std::to_string(maxSize) + "/" + std::to_string(i), srcDataset->getRank(), newDatasetSize, chunkSize, true);
                    HDF5File::HDF5Dataset *dstDatasetFinal = hDF5OutputFile->openDataset(srcGroup->getName() + "_" + std::to_string(maxSize) + "/" + std::to_string(i));

                    float *srcData;
                    float *dstData;
                    float minV, maxV;

                    // First 2D slices in XY plane
                    for (unsigned int z = 0; z < nZ; z++) {
                        srcDataset->read3DDataset(z, 0, 0, 1, nY, nX, srcData, minV, maxV);
                        cv::Mat image = cv::Mat((int) nY, (int) nX, CV_32FC1, srcData); // rows, cols (height, width)
                        cv::resize(image, image, cv::Size((int) nXd, (int) nYd));
                        dstData = (float *) image.data;
                        dstDataset->write3DDataset(z, 0, 0, 1, nYd, nXd, dstData);
                        image.release();
                        delete [] srcData;
                        //delete [] dstData;
                    }

                    // and after 2d slices XZ plane
                    for (unsigned int y = 0; y < nYd; y++) {
                        dstDataset->read3DDataset(0, y, 0, nZ, 1, nXd, srcData, minV, maxV);
                        cv::Mat image = cv::Mat((int) nZ, (int) nXd, CV_32FC1, srcData); // rows, cols (height, width)
                        cv::resize(image, image, cv::Size((int) nXd, (int) nZd));
                        dstData = (float *) image.data;
                        dstDatasetFinal->write3DDataset(0, y, 0, nZd, 1, nXd, dstData, true);
                        image.release();
                        delete [] srcData;
                        //delete [] dstData;
                    }

                    dstDatasetFinal->findAndSetGlobalMinAndMaxValue();
                    if (minVG > dstDatasetFinal->getGlobalMinValueF()) minVG = dstDatasetFinal->getGlobalMinValueF();
                    if (maxVG < dstDatasetFinal->getGlobalMaxValueF()) maxVG = dstDatasetFinal->getGlobalMaxValueF();
                    hDF5OutputFile->closeDataset(srcGroup->getName() + "_" + std::to_string(maxSize) + "/" + std::to_string(i));
                    hDF5OutputFile->closeDataset(srcDataset->getName());
                }

                // Save attributes
                hsize_t newPositionZ = myRound((double) (srcGroup->readAttributeI("positionZ") + 1) * ratio) - 1;
                hsize_t newPositionY = myRound((double) (srcGroup->readAttributeI("positionY") + 1) * ratio) - 1;
                hsize_t newPositionX = myRound((double) (srcGroup->readAttributeI("positionX") + 1) * ratio) - 1;
                hsize_t newSizeZ = myRound((double) (srcGroup->readAttributeI("sizeZ") + 1) * ratio) - 1;
                hsize_t newSizeY = myRound((double) (srcGroup->readAttributeI("sizeY") + 1) * ratio) - 1;
                hsize_t newSizeX = myRound((double) (srcGroup->readAttributeI("sizeX") + 1) * ratio) - 1;
                //if (newPositionZ < 0) newPositionZ = 0;
                //if (newPositionY < 0) newPositionY = 0;
                //if (newPositionX < 0) newPositionX = 0;
                if (newSizeZ < 1) newSizeZ = 1;
                if (newSizeY < 1) newSizeY = 1;
                if (newSizeX < 1) newSizeX = 1;
                dstGroupFinal->setAttribute("positionZ", (uint64_t) newPositionZ);
                dstGroupFinal->setAttribute("positionY", (uint64_t) newPositionY);
                dstGroupFinal->setAttribute("positionX", (uint64_t) newPositionX);
                dstGroupFinal->setAttribute("sizeZ", (uint64_t) newSizeZ);
                dstGroupFinal->setAttribute("sizeY", (uint64_t) newSizeY);
                dstGroupFinal->setAttribute("sizeX", (uint64_t) newSizeX);
                dstGroupFinal->setAttribute("count", (uint64_t) count);
                dstGroupFinal->setAttribute("dwnsmpl", (uint64_t) maxSize);
                dstGroupFinal->setAttribute("min", minVG);
                dstGroupFinal->setAttribute("max", maxVG);
                dstGroupFinal->setAttribute("src_group_name", srcGroup->getName());
                dstGroupFinal->setAttribute("src_group_id", (uint64_t) srcGroup->getId());

                // Filter or add to map
                if (!flagNames || std::find(names.begin(), names.end(), dstGroupFinal->getName()) != names.end())
                    datasetsForProcessing->datasetsGroupTypeDwnsmpl.insert(std::pair<const H5std_string, HDF5File::HDF5Group*>(dstGroupFinal->getName(), dstGroupFinal));

                //hDF5OutputFile->closeGroup(dstGroupFinal->getName());
            }
            delete tmpFile;
            remove("tmp.h5");
        }
    } catch(std::exception &e) {
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
void rechunkDataset(HDF5File::HDF5Dataset *srcDataset, HDF5File *hDF5OutputFile, hsize_t maxChunkSize) {
    hsize_t *dims = srcDataset->getDims();

    // Chunk size
    hsize_t chunkSize[3] = {maxChunkSize, maxChunkSize, maxChunkSize};
    if (chunkSize[0] > dims[0]) chunkSize[0] = dims[0];
    if (chunkSize[1] > dims[1]) chunkSize[1] = dims[1];
    if (chunkSize[2] > dims[2]) chunkSize[2] = dims[2];
    hDF5OutputFile->createDatasetF(srcDataset->getName(), 3, dims, chunkSize, true);
    HDF5File::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName());

    // Copy attributes
    for (int i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5File::HDF5Group::HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr);
        delete attr;
    }

    float *data;
    float minV, maxV;
    float minValueGlobal, maxValueGlobal;
    bool first = true;
    hsize_t xO, yO, zO, xC, yC, zC;
    srcDataset->initBlockReading();
    do {
        srcDataset->readBlock(zO, yO, xO, zC, yC, xC, data, minV, maxV);
        dstDataset->write3DDataset(zO, yO, xO, zC, yC, xC, data, true);
        delete [] data;
        if (first) {
            minValueGlobal = minV;
            maxValueGlobal = maxV;
            first = false;
        }
        if (minValueGlobal > minV) minValueGlobal = minV;
        if (maxValueGlobal < maxV) maxValueGlobal = maxV;
    } while (!srcDataset->isLastBlock());
    dstDataset->setAttribute("min", minValueGlobal);
    dstDataset->setAttribute("max", maxValueGlobal);
    hDF5OutputFile->closeDataset(dstDataset->getName());
}

/**
 * @brief rechunk Change chunk size of datasets
 * @param hDF5SimulationOutputFile
 * @param hDF5OutputFile
 * @param datasetsForProcessing
 */
void rechunk(HDF5File *hDF5SimulationOutputFile, HDF5File *hDF5OutputFile, DatasetsForProcessing *datasetsForProcessing)
{
    try {
        if (datasetsForProcessing->datasets3DType.empty() && datasetsForProcessing->datasets3DTypeDwnsmpl.empty() && datasetsForProcessing->datasetsGroupType.empty() && datasetsForProcessing->datasetsGroupTypeDwnsmpl.empty()) {
            std::cout << "No dataset for change chunks in simulation output file" << std::endl;
        } else {

            // For every 3D type dataset
            for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsForProcessing->datasets3DType.begin(); it != datasetsForProcessing->datasets3DType.end(); ++it) {
                HDF5File::HDF5Dataset *srcDataset = it->second;
                rechunkDataset(srcDataset, hDF5OutputFile, maxChunkSize);
            }

            // For every 3D type downsampled dataset
            for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsForProcessing->datasets3DTypeDwnsmpl.begin(); it != datasetsForProcessing->datasets3DTypeDwnsmpl.end(); ++it) {
                HDF5File::HDF5Dataset *srcDataset = it->second;
                rechunkDataset(srcDataset, hDF5OutputFile, maxChunkSize);
            }

            // For every reshaped mask type
            for (std::map<const H5std_string, HDF5File::HDF5Group *>::iterator it = datasetsForProcessing->datasetsGroupType.begin(); it != datasetsForProcessing->datasetsGroupType.end(); ++it) {
                HDF5File::HDF5Group *srcGroup = it->second;
                hsize_t count = srcGroup->getNumObjs();
                // Try create group, if it already exists, then do not rewrite it
                try {
                    hDF5OutputFile->createGroup(srcGroup->getName());
                } catch(std::exception &) {
                }
                HDF5File::HDF5Group *dstGroup = hDF5OutputFile->openGroup(srcGroup->getName());

                // Copy attributes
                for (int i = 0; i < srcGroup->getNumAttrs(); i++) {
                    HDF5File::HDF5Group::HDF5Attribute *attr = srcGroup->getAttribute(i);
                    dstGroup->setAttribute(attr);
                    delete attr;
                }

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5File::HDF5Dataset *srcDataset = hDF5SimulationOutputFile->openDataset(srcGroup->getName() + "/" + std::to_string(i));
                    rechunkDataset(srcDataset, hDF5OutputFile, maxChunkSize);
                    hDF5SimulationOutputFile->closeDataset(srcGroup->getName() + "/" + std::to_string(i));
                }
            }

            // For every reshaped mask type downsampled
            for (std::map<const H5std_string, HDF5File::HDF5Group *>::iterator it = datasetsForProcessing->datasetsGroupTypeDwnsmpl.begin(); it != datasetsForProcessing->datasetsGroupTypeDwnsmpl.end(); ++it) {
                HDF5File::HDF5Group *srcGroup = it->second;
                hsize_t count = srcGroup->getNumObjs();
                // Try create group, if it already exists, then do not rewrite it
                try {
                    hDF5OutputFile->createGroup(srcGroup->getName());
                } catch(std::exception &) {
                }
                HDF5File::HDF5Group *dstGroup = hDF5OutputFile->openGroup(srcGroup->getName());

                // Copy attributes
                for (int i = 0; i < srcGroup->getNumAttrs(); i++) {
                    HDF5File::HDF5Group::HDF5Attribute *attr = srcGroup->getAttribute(i);
                    dstGroup->setAttribute(attr);
                    delete attr;
                }

                // For every 3D type dataset in group
                for (unsigned int i = 0; i < count; i++) {
                    HDF5File::HDF5Dataset *srcDataset = hDF5SimulationOutputFile->openDataset(srcGroup->getName() + "/" + std::to_string(i));
                    rechunkDataset(srcDataset, hDF5OutputFile, maxChunkSize);
                    hDF5SimulationOutputFile->closeDataset(srcGroup->getName() + "/" + std::to_string(i));
                }
            }

        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief visualize Simple slice visualization
 * @param hDF5ViewFile
 */
void visualize(HDF5File *hDF5ViewFile)
{
    try {
        HDF5File::HDF5Dataset *dataset;
        HDF5File::HDF5Group *group;
        bool flagGroup = false;
        bool flagDataset = false;

        try {
            group = hDF5ViewFile->openGroup(datasetName);
            flagGroup = true;
        } catch(std::exception &) {
            //std::cerr << e.what() << std::endl;
            std::cout << "Dataset is not a time series" << std::endl;
            dataset = hDF5ViewFile->openDataset(datasetName);
            hsize_t *size = dataset->getDims();
            if (size[0] > hDF5ViewFile->getNZ() || size[1] > hDF5ViewFile->getNY() || size[1] > hDF5ViewFile->getNX())
                std::cout << "Dataset is too large" << std::endl;
            else
                flagDataset = true;
        }

        if (flagGroup) {
            float minValueSeriesGlobal = group->readAttributeF("min");
            float maxValueSeriesGlobal = group->readAttributeF("max");
            uint64_t count = group->readAttributeI("count");

            cv::namedWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex));
            cv::namedWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex));
            cv::namedWindow(datasetName + " series global " + cutType + " " + std::to_string(cutIndex));

            for (uint64_t i = 0; i < count; i++) {
                HDF5File::HDF5Dataset *dataset = hDF5ViewFile->openDataset(datasetName + "/" + std::to_string(i));

                float minValueGlobal = dataset->getGlobalMinValueF();
                float maxValueGlobal = dataset->getGlobalMaxValueF();

                float minValue = 0;
                float maxValue = 0;

                hsize_t *size = dataset->getDims();
                std::cout << "Dataset size:       " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                hsize_t *chunkSize = dataset->getChunkDims();
                std::cout << "Dataset chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;


                float *data = NULL;
                uint64_t height = 0;
                uint64_t width = 0;

                if (cutType == "YX") {
                    if (cutIndex >= size[0])
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex) + " >= " + std::to_string(size[0]));
                    dataset->read3DDataset(cutIndex, 0, 0, 1, size[1], size[2], data, minValue, maxValue); // YX
                    height = size[1];
                    width = size[2];
                } else if (cutType == "ZX") {
                    if (cutIndex >= size[1])
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex) + " >= " + std::to_string(size[1]));
                    dataset->read3DDataset(0, cutIndex, 0, size[0], 1, size[2], data, minValue, maxValue); // ZX
                    height = size[0];
                    width = size[2];
                } else if (cutType == "ZY") {
                    if (cutIndex >= size[2])
                        throw std::runtime_error("Wrong cutIndex - " + std::to_string(cutIndex) + " >= " + std::to_string(size[2]));
                    dataset->read3DDataset(0, 0, cutIndex, size[0], size[1], 1, data, minValue, maxValue); // ZY
                    height = size[0];
                    width = size[1];
                } else
                    throw std::runtime_error("Wrong cutType (cutType must be YX, ZX or ZY)");

                std::cout << "   minValueSeriesGlobal: " << minValueSeriesGlobal << "\tmaxValueSeriesGlobal: " << maxValueSeriesGlobal << std::endl;
                std::cout << "   minValueGlobal:       " << minValueGlobal <<       "\tmaxValueGlobal:      " << maxValueGlobal << std::endl;
                std::cout << "   minValue:             " << minValue <<             "\tmaxValue:            " << maxValue << std::endl;
                std::cout << "   width:                " << width <<                "\theight:              " << height << std::endl;

                cv::Mat imageL = cv::Mat((int) height, (int) width, CV_32FC1, data); // rows, cols (height, width)
                cv::Mat imageG;
                cv::Mat imageGG;

                imageL.convertTo(imageG, CV_8UC1, 255.0 / (maxValueGlobal - minValueGlobal), - minValueGlobal * 255.0 /(maxValueGlobal - minValueGlobal));
                imageL.convertTo(imageGG, CV_8UC1, 255.0 / (maxValueSeriesGlobal - minValueSeriesGlobal), - minValueSeriesGlobal * 255.0 /(maxValueSeriesGlobal - minValueSeriesGlobal));
                imageL.convertTo(imageL, CV_8UC1, 255.0 / (maxValue - minValue), - minValue * 255.0 /(maxValue - minValue));

                if (width > height) {
                    int dstWidth = 300;
                    cv::Size size(dstWidth, (int) ceil((double) imageL.size().height * dstWidth / imageL.size().width));
                    // Set original size
                    size = imageL.size();
                    cv::resize(imageL, imageL, size);
                    cv::resize(imageG, imageG, size);
                    cv::resize(imageGG, imageGG, size);
                } else {
                    int dstHeight = 300;
                    cv::Size size((int) ceil((double) imageL.size().width * dstHeight / imageL.size().height), dstHeight);
                    // Set original size
                    size = imageL.size();
                    cv::resize(imageL, imageL, size);
                    cv::resize(imageG, imageG, size);
                    cv::resize(imageGG, imageGG, size);
                }

                //cv::moveWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), 100, 50);
                //cv::moveWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex), imageL.size().width + 50 + 100, 50);
                //cv::moveWindow(datasetName + " series global " + cutType + " " + std::to_string(cutIndex), 2 * imageL.size().width  + 100 + 100, 50);

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
                delete [] data;
            }
        } else if (flagDataset) {
            float minValueGlobal = dataset->getGlobalMinValueF();
            float maxValueGlobal = dataset->getGlobalMaxValueF();

            float minValue = 0;
            float maxValue = 0;
            hsize_t *size = dataset->getDims();
            std::cout << "Dataset size:       " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
            hsize_t *chunkSize = dataset->getChunkDims();
            std::cout << "Dataset chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;

            float *data = NULL;
            uint64_t height = 0;
            uint64_t width = 0;

            if (cutType == "YX") {
                if (cutIndex >= size[0])
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(cutIndex, 0, 0, 1, size[1], size[2], data, minValue, maxValue); // YX
                height = size[1];
                width = size[2];
            } else if (cutType == "ZX") {
                if (cutIndex >= size[1])
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(0, cutIndex, 0, size[0], 1, size[2], data, minValue, maxValue); // ZX
                height = size[0];
                width = size[2];
            } else if (cutType == "ZY") {
                if (cutIndex >= size[2])
                    throw std::runtime_error("Wrong cutIndex - is too big");
                dataset->read3DDataset(0, 0, cutIndex, size[0], size[1], 1, data, minValue, maxValue); // ZY
                height = size[0];
                width = size[1];
            } else
                throw std::runtime_error("Wrong cutType (cutType must be YX, ZX or ZY)");

            std::cout << "   minValueGlobal: " << minValueGlobal << "\tmaxValueGlobal: " << maxValueGlobal << std::endl;
            std::cout << "   minValue:       " << minValue <<       "\tmaxValue:       " << maxValue << std::endl;
            std::cout << "   width:          " << width <<          "\theight:         " << height << std::endl;

            cv::Mat imageL = cv::Mat((int) height, (int) width, CV_32FC1, data); // rows, cols (height, width)
            cv::Mat imageG;
            // cv::Size(width, height)
            imageL.convertTo(imageG, CV_8UC1, 255.0 / (maxValueGlobal - minValueGlobal), - minValueGlobal * 255.0 /(maxValueGlobal - minValueGlobal));
            imageL.convertTo(imageL, CV_8UC1, 255.0 / (maxValue - minValue), - minValue * 255.0 /(maxValue - minValue));

            cv::Mat imageLUP;
            cv::Mat imageGUP;

            if (width > height) {
                int dstWidth = 300;
                cv::Size size(dstWidth, (int) ceil((double) imageL.size().height * dstWidth / imageL.size().width));
                // Set original size
                size = imageL.size();
                cv::resize(imageL, imageLUP, size);
                cv::resize(imageG, imageGUP, size);
            } else {
                int dstHeight = 300;
                cv::Size size((int) ceil((double) imageL.size().width * dstHeight / imageL.size().height), dstHeight);
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

            //cv::moveWindow(datasetName + " local " + cutType + " " + std::to_string(cutIndex), 100, 50);
            //cv::moveWindow(datasetName + " global " + cutType + " " + std::to_string(cutIndex), imageL.size().width + 50 + 100, 50);

            //cv::imwrite(datasetName + " local " + cutType + " " + std::to_string(cutIndex) + ".png", imageL);
            //cv::imwrite(datasetName + " global " + cutType + " " + std::to_string(cutIndex) + ".png", imageG);

            imageLUP.release();
            imageGUP.release();
            imageG.release();
            imageL.release();
            delete [] data;
        } else {
            std::cout << "Dataset is not displayable" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    cv::waitKey(0);
    cv::destroyAllWindows();
}

/**
 * @brief copyDimensionsAndAttributes Copy dimensions and attributes
 * @param hDF5SimulationOutputFile
 * @param hDF5OutputFile
 */
void copyDimensionsAndAttributes(HDF5File *hDF5SimulationOutputFile, HDF5File *hDF5OutputFile)
{
    // Copy nT, nX, nY, nZ
    try {
        hsize_t size[3] = {1, 1, 1};
        uint64_t data[1];
        data[0] = hDF5SimulationOutputFile->getNT();
        hDF5OutputFile->createDatasetI(NT_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
        hDF5OutputFile->openDataset(NT_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
        data[0] = hDF5SimulationOutputFile->getNX();
        hDF5OutputFile->createDatasetI(NX_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
        hDF5OutputFile->openDataset(NX_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
        data[0] = hDF5SimulationOutputFile->getNY();
        hDF5OutputFile->createDatasetI(NY_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
        hDF5OutputFile->openDataset(NY_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
        data[0] = hDF5SimulationOutputFile->getNZ();
        hDF5OutputFile->createDatasetI(NZ_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
        hDF5OutputFile->openDataset(NZ_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Copy root (info) attributes to destination h5 file
    try {
        HDF5File::HDF5Group *srcGroup = hDF5SimulationOutputFile->openGroup("/");
        HDF5File::HDF5Group *dstGroup = hDF5OutputFile->openGroup("/");
        for (int i = 0; i < srcGroup->getNumAttrs(); i++) {
            HDF5File::HDF5Group::HDF5Attribute *attr = srcGroup->getAttribute(i);
            dstGroup->setAttribute(attr);
            delete attr;
        }
    } catch(std::exception &e) {
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
    HDF5File *hDF5SimulationOutputFile = NULL;
    HDF5File *hDF5SimulationInputFile = NULL;
    HDF5File *hDF5OutputFile = NULL;

    HDF5File *hDF5ViewFile = NULL;

    // Create helper class for datasets of various types
    DatasetsForProcessing *datasetsForProcessing = new DatasetsForProcessing();
    datasetsForProcessing->sensorMaskIndexDataset = NULL;
    datasetsForProcessing->sensorMaskSize = 0;

    // Load parameters
    getParams(argc, argv);

    // Load simulation output file
    if (simulationOutputFilename.empty()) {
        std::cerr << "Missing parameter -f (simulation output filename)" << std::endl;
        std::cout << help() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        printDebugTitle("Load simulation output file");
        hDF5SimulationOutputFile = loadSimulationFile(simulationOutputFilename);
    }

    // Load simulation input file
    if (!simulationInputFilename.empty()) {
        printDebugTitle("Load simulation input file");
        hDF5SimulationInputFile = loadSimulationFile(simulationInputFilename);
    }

    // Find and get sensor mask dataset
    if (flagReshape) {
        datasetsForProcessing->sensorMaskIndexDataset = findAndGetSensorMaskDataset(hDF5SimulationOutputFile, hDF5SimulationInputFile);
        // Get sensor mask size
        if (datasetsForProcessing->sensorMaskIndexDataset != NULL){
            hsize_t *size = datasetsForProcessing->sensorMaskIndexDataset->getDims();
            if (datasetsForProcessing->sensorMaskIndexDataset->getRank() == 3 && size[0] == 1 && size[1] == 1) {
                datasetsForProcessing->sensorMaskSize = size[2];
            } else {
                std::cerr << "Wrong sensor mask" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Sensor mask is not in simulation output or input file" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Find datasets for visualization to edit
    printDebugTitle("Find datasets for visualization for processing");
    findDatasetsForProcessing(hDF5SimulationOutputFile, datasetsForProcessing);

    // Test dataset reading
    if (flagTest) {
        printDebugTitle("Testing");
        testOfReading(datasetsForProcessing);
    }

    if (flagReshape || flagRechunk || flagDwnsmpl) {
        // Create new file
        printDebugTitle("Create or open output file");
        hDF5OutputFile = createOrOpenOutputFile(outputFilename);
        // Copy dimensions and attributes
        printDebugTitle("Copy dimensions and attributes");
        copyDimensionsAndAttributes(hDF5SimulationOutputFile, hDF5OutputFile);
    }

    // Processing of sensor mask
    if (flagReshape) {
        printDebugTitle("Reshaping");
        reshape(hDF5SimulationOutputFile, hDF5OutputFile, datasetsForProcessing);
    }

    // Downsampling
    if (flagDwnsmpl) {
        printDebugTitle("Downsampling");
        downsampling(hDF5SimulationOutputFile, hDF5OutputFile, datasetsForProcessing);
    }

    // Copy 3D datasets a set new chunking
    if (flagRechunk) {
        printDebugTitle("Change chunks");
        rechunk(hDF5SimulationOutputFile, hDF5OutputFile, datasetsForProcessing);
    }

    // Select file to view
    if (!outputFilename.empty() && (flagRechunk || flagReshape || flagDwnsmpl))
        hDF5ViewFile = hDF5OutputFile;
    else
        hDF5ViewFile = hDF5SimulationOutputFile;

    // Simple slice visualization
    if (flagView) {
        printDebugTitle("Visualization");
        visualize(hDF5ViewFile);
    }

    printDebugTitle("Closing files");

    // Close files
    delete hDF5SimulationOutputFile;
    delete hDF5SimulationInputFile;
    delete hDF5OutputFile;

    double t1 = HDF5File::getTime();

    std::cout << std::endl << std::endl << "Time of the entire process: " << (t1-t0) << " ms; \t" << std::endl << std::endl << std::endl;

    //return a.exec();
    std::exit(EXIT_SUCCESS);
}
