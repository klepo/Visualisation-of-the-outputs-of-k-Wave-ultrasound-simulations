#include <HDF5File.h>
#include <HDF5Dataset.h>
#include <HDF5Group.h>
#include <HDF5Attribute.h>

#include <iostream>
#include <string>
#include <map>
#include <math.h>
#include <time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");

const std::string NT_DATASET("Nt");
const std::string NX_DATASET("Nx");
const std::string NY_DATASET("Ny");
const std::string NZ_DATASET("Nz");

#define MAX_SIZE 256
#define MAX_CHUNK_SIZE 10

#define MAX_NUMBER_OF_FRAMES 1 // TODO

std::string simulationOutputFilename = "";
std::string simulationIutputFilename = "";
std::string outputFilename = "";

bool flagSensorMask = false;

hsize_t maxSize = MAX_SIZE;
hsize_t maxChunkSize = MAX_CHUNK_SIZE;

std::string datasetName = "";
std::string cutType = "YX";
hsize_t cutIndex = 0;

bool flagReshape = false;
bool flagRechunk = false;
bool flagView = false;
bool flagDwnsmpl = false;

std::string help = "\n"
                   "Usage: k-wave-h5-processing [options]\n"
                   "where options include:\n\n"
                   "  -f HDF5SimulationOutputFilename ...... Required parameter.\n"
                   "                                         HDF5 file with simulation results.\n"
                   "\n"
                   "  -m HDF5SimulationInputFilename ....... Optional parameter. Simulation HDF5 input file\n"
                   "                                         with sensor_mask_index dataset.\n"
                   "\n"
                   "  -o HDF5OutputFilename ................ Required parameter for reshape or changeChunks mode.\n"
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
                   "  -help ................................ Prints this help message.\n"
                   "\n";

void getParams(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -f (simulation output filename)" << std::endl;
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationOutputFilename = argv[i];
            std::cout << "\n  Simulation output filename:\n    " << simulationOutputFilename << std::endl;

        } else if (strcmp("-help", argv[i]) == 0) {
            std::cout << help << std::endl;
            exit(EXIT_SUCCESS);
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
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
            datasetName = argv[i];
            std::cout << "\n  Name of dataset to visualize:\n    " << datasetName << std::endl;
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -view (missing type of cut)" << std::endl;
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
            cutType = argv[i];
            std::cout << "\n  Cut type to visualize:\n    " << cutType << std::endl;
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -view (missing index)" << std::endl;
                std::cout << help << std::endl;
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
                std::cout << help << std::endl;
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
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationIutputFilename = argv[i];
            std::cout << "\n  Simulation iutput filename:\n    " << simulationIutputFilename << std::endl;
            continue;
        } else if (strcmp("-o", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -o (output filename)" << std::endl;
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
            outputFilename = argv[i];
            std::cout << "\n  Output filename:\n    " << outputFilename << std::endl;

        } else if (strcmp("-s", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -s (max size for downsampling)" << std::endl;
                std::cout << help << std::endl;
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
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (strcmp("-ch", argv[i]) == 0) {
            i++;
            if (argc <= i) {
                std::cerr << "\n  Wrong parameter -ch (chunk size)" << std::endl;
                std::cout << help << std::endl;
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
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "\n  Unknown options (" << argv[i] << ")" << std::endl;
            std::cout << help << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv)
{
    HDF5File *hDF5SimulationOutputFile= NULL;
    bool flagSOF = false;

    HDF5File *hDF5SimulationInputFile = NULL;
    bool flagSIF = false;

    HDF5File *hDF5OutputFile = NULL;
    bool flagOF = false;

    HDF5File *hDF5ViewFile = NULL;

    HDF5File::HDF5Dataset *sensorMaskIndexDataset = NULL;
    uint64_t nT, nX, nY, nZ;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasetsMaskType;
    std::map<const H5std_string, HDF5File::HDF5Group *> datasetsGroupType;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasets3DType;
    uint64_t sensorMaskSize = 0;

    // Load parameters
    getParams(argc, argv);

    if (simulationOutputFilename.empty()) {
        std::cerr << "Missing parameter -f (simulation output filename)" << std::endl;
        std::cout << help << std::endl;
        exit(EXIT_FAILURE);
    }

    // Load simulation output file and dimensions
    try {
        std::cout << std::endl << std::endl << "---- Load simulation output file ----" << std::endl << std::endl << std::endl;
        hDF5SimulationOutputFile = new HDF5File(simulationOutputFilename, HDF5File::OPEN);
        flagSOF = true;
        nT = hDF5SimulationOutputFile->getNT();
        nX = hDF5SimulationOutputFile->getNX();
        nY = hDF5SimulationOutputFile->getNY();
        nZ = hDF5SimulationOutputFile->getNZ();
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Find and get sensor mask dataset
    if (flagReshape) {
        try {
            std::cout << std::endl << std::endl << "---- Find and get sensor mask dataset ----" << std::endl << std::endl << std::endl;
            sensorMaskIndexDataset = hDF5SimulationOutputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
            flagSensorMask = true;
        } catch(std::exception &) {
            std::cout << "Sensor mask is not in simulation output file" << std::endl;
            // Try to load sensor mask from simulation input file
            if (simulationIutputFilename.empty()) {
                std::cerr << "Missing sensor mask" << std::endl;
                std::cout << help << std::endl;
                exit(EXIT_FAILURE);
            } else {
                try {
                    hDF5SimulationInputFile = new HDF5File(simulationIutputFilename, HDF5File::OPEN);
                    flagSIF = true;
                    sensorMaskIndexDataset = hDF5SimulationInputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
                    if (sensorMaskIndexDataset->getDataType() == H5T_INTEGER)
                        flagSensorMask = true;
                } catch(std::exception &) {
                    //std::cerr << e.what() << std::endl;
                    std::cerr << "Sensor mask is not in simulation input file" << std::endl;
                    std::cout << help << std::endl;
                }
            }
        }
        // Get sensor mask size
        if (flagSensorMask){
            hsize_t *size = sensorMaskIndexDataset->getDims();
            if (sensorMaskIndexDataset->getRank() == 3 && size[0] == 1 && size[1] == 1) {
                sensorMaskSize = size[2];
            } else {
                flagSensorMask = false;
                std::cerr << "Wrong sensor mask" << std::endl;
            }
        }
    }

    // Find datasets for visualization to edit
    if (flagReshape || flagRechunk || flagDwnsmpl) {
        std::cout << std::endl << std::endl << "---- Find datasets for visualization for processing ----" << std::endl << std::endl << std::endl;
        for (hsize_t i = 0; i < hDF5SimulationOutputFile->getNumObjs(); i++) {
            try {
                HDF5File::HDF5Dataset *dataset = hDF5SimulationOutputFile->openDataset(i);
                hsize_t *size = dataset->getDims();
                // 3D type
                if (dataset->getDataType() == H5T_FLOAT && dataset->getRank() == 3 && size[0] == nZ && size[1] == nY && size[2] == nX) {
                    datasets3DType.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dataset->getName(), dataset));
                    std::cout << "----> 3D type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                }
                // Sensor mask type
                else if (dataset->getDataType() == H5T_FLOAT && flagSensorMask && dataset->getRank() == 3 && size[0] == 1 && size[1] <= nT && size[2] == sensorMaskSize) {
                    datasetsMaskType.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset *>(dataset->getName(), dataset));
                    std::cout << "----> Mask type dataset: "<< dataset->getName() << "; size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
                } else
                    hDF5SimulationOutputFile->closeDataset(dataset->getName());
            } catch(std::exception &) {
                std::cout << "Object " << i << " is not dataset" << std::endl;
                // Reshaped mask type to group datasetsGroupType
                try {
                    HDF5File::HDF5Group *group = hDF5SimulationOutputFile->openGroup(i);
                    try {
                        group->readAttributeI("src_group_id");
                        std::cout << "Object " << i << " is not original reshaped group" << std::endl;
                    } catch(std::exception &) {
                        uint64_t count = group->readAttributeI("count");
                        uint64_t posX = group->readAttributeI("positionX");
                        uint64_t posY = group->readAttributeI("positionY");
                        uint64_t posZ = group->readAttributeI("positionZ");
                        datasetsGroupType.insert(std::pair<const H5std_string, HDF5File::HDF5Group *>(group->getName(), group));
                        std::cout << "----> Reshaped mask type group: "<< group->getName() << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl;
                    }
                } catch(std::exception &) {
                    std::cout << "Object " << i << " is not reshaped group" << std::endl;
                }
            }
        }
    }

    // Create new file
    if (flagReshape || flagRechunk || flagDwnsmpl) {
        if (outputFilename.empty()) {
            size_t lastindex = simulationOutputFilename.find_last_of(".");
            std::string rawname = simulationOutputFilename.substr(0, lastindex);
            outputFilename = rawname + "_modified.h5";
        }
        std::cout << std::endl << std::endl << "---- Create or open output file ----" << std::endl << std::endl << std::endl;
        try {
            hDF5OutputFile = new HDF5File(outputFilename, HDF5File::OPEN);
            flagOF = true;
        } catch(std::exception &) {
            try {
                hDF5OutputFile = new HDF5File(outputFilename, HDF5File::CREATE);
                flagOF = true;
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    // Processing of sensor mask
    if (flagReshape) {
        std::cout << std::endl << std::endl << "---- Reshape ----" << std::endl << std::endl << std::endl;
        if (datasetsMaskType.empty()) {
            std::cout << "No dataset for reshape in output file" << std::endl;
        } else {
            try {
                uint64_t *data;
                uint64_t minV, maxV;
                hsize_t xO, yO, zO, xC, yC, zC;
                hsize_t minX = nX;
                hsize_t minY = nY;
                hsize_t minZ = nZ;
                hsize_t maxX = 0, maxY = 0, maxZ = 0;
                sensorMaskIndexDataset->initBlockReading();

                // Find min and max position
                hsize_t index = 0, zM = 0, yM = 0, xM = 0;
                do {
                    sensorMaskIndexDataset->readBlock(zO, yO, xO, zC, yC, xC, data, minV, maxV);
                    for (hsize_t z = 0; z < zC; z++)
                        for (hsize_t y = 0; y < yC; y++)
                            for (hsize_t x = 0; x < xC; x++) {
                                index = data[x + y * xC + z * xC * yC];
                                hDF5SimulationInputFile->convertlinearTo3D(index, zM, yM, xM);
                                if (xM < minX) minX = xM;
                                if (yM < minY) minY = yM;
                                if (zM < minZ) minZ = zM;
                                if (xM > maxX) maxX = xM;
                                if (yM > maxY) maxY = yM;
                                if (zM > maxZ) maxZ = zM;
                            }
                    delete [] data;
                } while (!sensorMaskIndexDataset->isLastBlock());

                std::cout << "   min point:\t" << " z: " << minZ << "\ty: " << minY << "\tx: " << minX << std::endl;
                std::cout << "   max point:\t" << " z: " << maxZ << "\ty: " << maxY << "\tx: " << maxX << std::endl;

                // Find first and last point TODO
                hsize_t *size = sensorMaskIndexDataset->getDims();
                hsize_t firstX = 0, firstY = 0, firstZ = 0, lastX = 0, lastY = 0, lastZ = 0;
                sensorMaskIndexDataset->read3DDataset(0, 0, 0, 1, 1, 1, data, minV, maxV);
                hDF5SimulationInputFile->convertlinearTo3D(data[0], firstZ, firstY, firstX);
                delete [] data;
                sensorMaskIndexDataset->read3DDataset(size[0]-1, size[1]-1, size[2]-1, 1, 1, 1, data, minV, maxV);
                hDF5SimulationInputFile->convertlinearTo3D(data[0], lastZ, lastY, lastX);
                delete [] data;
                std::cout << "   first point:\t" << " z: " << firstZ << "\ty: " << firstY << "\tx: " << firstX << std::endl;
                std::cout << "   last point:\t" << " z: " << lastZ << "\ty: " << lastY << "\tx: " << lastX << std::endl;

                // Compute chunk size
                hsize_t chunkSize[3];
                chunkSize[0] = std::min(maxChunkSize, maxZ - minZ + 1);
                chunkSize[1] = std::min(maxChunkSize, maxY - minY + 1);
                chunkSize[2] = std::min(maxChunkSize, maxX - minX + 1);
                // and dataset size
                hsize_t datasetSize[3] = {maxZ - minZ + 1, maxY - minY + 1, maxX - minX + 1};

                std::cout << "   new chunk size:\t" << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;

                // For every mask type dataset
                for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasetsMaskType.begin(); it != datasetsMaskType.end(); ++it) {
                    HDF5File::HDF5Dataset *dataset = it->second;

                    uint64_t *sensorMaskData;
                    float *datasetData;
                    uint64_t minVI, maxVI;
                    float minVF, maxVF;
                    float minVFG, maxVFG;
                    bool first = true;
                    hsize_t xO, yMO, yDO, zO, xC, yC, zC;

                    sensorMaskIndexDataset->initBlockReading();
                    dataset->initBlockReading(sensorMaskIndexDataset->getBlockSize());

                    hDF5OutputFile->createGroup(dataset->getName(), true);
                    HDF5File::HDF5Group *group = hDF5OutputFile->openGroup(dataset->getName());
                    group->setAttribute("positionZ", (uint64_t) minZ);
                    group->setAttribute("positionY", (uint64_t) minY);
                    group->setAttribute("positionX", (uint64_t) minX);
                    group->setAttribute("sizeZ", (uint64_t) datasetSize[0]);
                    group->setAttribute("sizeY", (uint64_t) datasetSize[1]);
                    group->setAttribute("sizeX", (uint64_t) datasetSize[2]);

                    hDF5OutputFile->createDatasetF(dataset->getName()  + "/" + std::to_string(0), 3, datasetSize, chunkSize, true);
                    HDF5File::HDF5Dataset *actualDataset = hDF5OutputFile->openDataset(dataset->getName()  + "/" + std::to_string(0));

                    hsize_t index = 0, zM = 0, yM = 0, xM = 0;
                    float data[1];

                    do {
                        if (sensorMaskIndexDataset->isLastBlock()) {
                            if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                                if (yDO + 1 >= MAX_NUMBER_OF_FRAMES)
                                    break;

                            sensorMaskIndexDataset->initBlockReading();
                            actualDataset->findAndSetGlobalMinAndMaxValue();

                            if (first){
                                minVFG = actualDataset->getGlobalMinValueF();
                                maxVFG = actualDataset->getGlobalMaxValueF();
                            }
                            first = false;

                            if (minVFG > actualDataset->getGlobalMinValueF()) minVFG = actualDataset->getGlobalMinValueF();
                            if (maxVFG < actualDataset->getGlobalMaxValueF()) maxVFG = actualDataset->getGlobalMaxValueF();

                            hDF5OutputFile->closeDataset(dataset->getName()  + "/" + std::to_string(yDO));
                            hDF5OutputFile->createDatasetF(dataset->getName()  + "/" + std::to_string(yDO+1), 3, datasetSize, chunkSize, true);
                            actualDataset = hDF5OutputFile->openDataset(dataset->getName()  + "/" + std::to_string(yDO+1));
                        }

                        sensorMaskIndexDataset->readBlock(zO, yMO, xO, zC, yC, xC, sensorMaskData, minVI, maxVI);
                        dataset->readBlock(zO, yDO, xO, zC, yC, xC, datasetData, minVF, maxVF);

                        int t4 = clock();
                        for (hsize_t z = 0; z < zC; z++)
                            for (hsize_t y = 0; y < yC; y++)
                                for (hsize_t x = 0; x < xC; x++) {
                                    index = sensorMaskData[x + y * xC + z * xC * yC];
                                    hDF5SimulationInputFile->convertlinearTo3D(index, zM, yM, xM);
                                    data[0] = datasetData[x + y * xC + z * xC * yC];
                                    actualDataset->write3DDataset(zM - minZ, yM - minY, xM - minX, 1, 1, 1, data, false);
                                }
                        int t5 = clock();

                        std::cout << "write time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << std::endl;

                        delete [] sensorMaskData;
                        delete [] datasetData;

                    } while (!dataset->isLastBlock());

                    actualDataset->findAndSetGlobalMinAndMaxValue();

                    if (first){
                        minVFG = actualDataset->getGlobalMinValueF();
                        maxVFG = actualDataset->getGlobalMaxValueF();
                    }
                    first = false;

                    if (minVFG > actualDataset->getGlobalMinValueF()) minVFG = actualDataset->getGlobalMinValueF();
                    if (maxVFG < actualDataset->getGlobalMaxValueF()) maxVFG = actualDataset->getGlobalMaxValueF();

                    group->setAttribute("min", minVFG);
                    group->setAttribute("max", maxVFG);
                    group->setAttribute("count", (uint64_t) yDO + 1);
                    hDF5OutputFile->closeDataset(actualDataset->getName());
                    hDF5OutputFile->closeGroup(dataset->getName());
                }
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::exit(EXIT_FAILURE);
            }

            std::cout << std::endl << std::endl << "---- Find datasetsGroupType ----" << std::endl << std::endl << std::endl;
            for (hsize_t i = 0; i < hDF5OutputFile->getNumObjs(); i++) {
                // Reshaped mask type to group datasetsGroupType
                try {
                    HDF5File::HDF5Group *group = hDF5OutputFile->openGroup(i);
                    try {
                        group->readAttributeI("src_group_id");
                        std::cout << "Object " << i << " is not original reshaped group" << std::endl;
                    } catch(std::exception &) {
                        uint64_t count = group->readAttributeI("count");
                        uint64_t posX = group->readAttributeI("positionX");
                        uint64_t posY = group->readAttributeI("positionY");
                        uint64_t posZ = group->readAttributeI("positionZ");
                        datasetsGroupType.insert(std::pair<const H5std_string, HDF5File::HDF5Group *>(group->getName(), group));
                        std::cout << "----> Reshaped mask type group: "<< group->getName() << "; count: " << count << "; posX: " << posX << " posY: " << posY << " posZ: " << posZ << std::endl;
                    }
                } catch(std::exception &) {
                    std::cout << "Object " << i << " is not reshaped group" << std::endl;
                }
            }
        }
    }

    // Copy 3D datasets a set new chunking
    if (flagRechunk) {
        std::cout << std::endl << std::endl << "---- Change chunks ----" << std::endl << std::endl << std::endl;
        try {
            hsize_t chunkSize[3] = {maxChunkSize, maxChunkSize, maxChunkSize};
            hsize_t datasetSize[3] = {nZ, nY, nX};
            if (datasets3DType.empty()) {
                std::cout << "No dataset for change chunks in simulation output file" << std::endl;
            } else {
                // For every 3D type dataset
                for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasets3DType.begin(); it != datasets3DType.end(); ++it) {
                    HDF5File::HDF5Dataset *srcDataset = it->second;
                    hDF5OutputFile->createDatasetF(srcDataset->getName(), 3, datasetSize, chunkSize, true);
                    HDF5File::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName());

                    float *data;
                    float minV, maxV;
                    float minValueGlobal, maxValueGlobal;
                    bool first = true;
                    hsize_t xO, yO, zO, xC, yC, zC;
                    srcDataset->initBlockReading();
                    do {
                        srcDataset->readBlock(zO, yO, xO, zC, yC, xC, data, minV, maxV);
                        dstDataset->write3DDataset(zO, yO, xO, zC, yC, xC, data);
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
            }
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    // Copy dimensions and attributes
    if ((flagSensorMask && flagReshape) || flagRechunk || flagDwnsmpl) {
        std::cout << std::endl << std::endl << "---- Copy dimensions and attributes ----" << std::endl << std::endl << std::endl;
        // Copy nT, nX, nY, nZ
        try {
            hsize_t size[3] = {1, 1, 1};
            uint64_t data[1];
            data[0] = nT;
            hDF5OutputFile->createDatasetI(NT_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
            hDF5OutputFile->openDataset(NT_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
            data[0] = nX;
            hDF5OutputFile->createDatasetI(NX_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
            hDF5OutputFile->openDataset(NX_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
            data[0] = nY;
            hDF5OutputFile->createDatasetI(NY_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
            hDF5OutputFile->openDataset(NY_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
            data[0] = nZ;
            hDF5OutputFile->createDatasetI(NZ_DATASET, 3, size, HDF5File::ZERO_CHUNK, true);
            hDF5OutputFile->openDataset(NZ_DATASET)->write3DDataset(0, 0, 0, 1, 1, 1, data);
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // Copy root attributes to destination h5 file
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

    // Downsampling
    if (flagDwnsmpl) {
        std::cout << std::endl << std::endl << "---- Downsampling ----" << std::endl << std::endl << std::endl;
        try {
            if (datasets3DType.empty() && datasetsGroupType.empty()) {
                std::cout << "No dataset for downsampling in simulation output file" << std::endl;
            } else if (std::max(std::max(nZ, nY), nX) <= maxSize) {
                // Check current size
                std::cout << "No dataset for downsampling - max(nZ, nY, nX) == " + std::to_string(std::max(std::max(nZ, nY), nX)) + " <= " + std::to_string(maxSize) << std::endl;
            } else {
                // Compute new size
                hsize_t nMax = std::max(std::max(nZ, nY), nX);
                double ratio = (double) maxSize / nMax;
                hsize_t nZd = (hsize_t) floor(nZ * ratio + 0.5); //floor( x + 0.5 )
                hsize_t nYd = (hsize_t) floor(nY * ratio + 0.5);
                hsize_t nXd = (hsize_t) floor(nX * ratio + 0.5);
                if (nZd < 1) nZd = 1;
                if (nYd < 1) nYd = 1;
                if (nXd < 1) nXd = 1;
                hsize_t newDatasetSize[3] = {nZd, nYd, nXd};
                hsize_t newTmpDatasetSize[3] = {nZ, nYd, nXd};
                hsize_t chunkSize[3] = {maxChunkSize, maxChunkSize, maxChunkSize};
                if (chunkSize[0] > nZd) chunkSize[0] = nZd;
                if (chunkSize[1] > nYd) chunkSize[1] = nYd;
                if (chunkSize[2] > nXd) chunkSize[2] = nXd;
                std::cout << "   new size:\t" << newDatasetSize[0] << " x " << newDatasetSize[1] << " x " << newDatasetSize[2] << std::endl;

                // Create temp file
                HDF5File *tmpFile = new HDF5File("tmp.h5", HDF5File::CREATE);

                // For every 3D type dataset
                for (std::map<const H5std_string, HDF5File::HDF5Dataset *>::iterator it = datasets3DType.begin(); it != datasets3DType.end(); ++it) {
                    HDF5File::HDF5Dataset *srcDataset = it->second;
                    tmpFile->createDatasetF(srcDataset->getName(), srcDataset->getRank(), newTmpDatasetSize);
                    HDF5File::HDF5Dataset *dstDataset = tmpFile->openDataset(srcDataset->getName());
                    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(maxSize), srcDataset->getRank(), newDatasetSize, chunkSize, true);
                    HDF5File::HDF5Dataset *dstDatasetFinal = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(maxSize));

                    //float *srcData;
                    float minV, maxV;
                    float minVG, maxVG;

                    for (unsigned int z = 0; z < nZ; z++) {
                        float *srcData;
                        srcDataset->read3DDataset(z, 0, 0, 1, nY, nX, srcData, minV, maxV);
                        cv::Mat image = cv::Mat((int) nY, (int) nX, CV_32FC1, srcData); // rows, cols (height, width)
                        cv::resize(image, image, cv::Size((int) nXd, (int) nYd), 0, 0, cv::INTER_NEAREST);
                        dstDataset->write3DDataset(z, 0, 0, 1, nYd, nXd, (float *) image.data);
                        image.release();
                        delete [] srcData;
                        srcData =  NULL;
                    }

                    for (unsigned int y = 0; y < nYd; y++) {
                        float *srcData;
                        dstDataset->read3DDataset(0, y, 0, nZ, 1, nXd, srcData, minV, maxV);
                        cv::Mat image = cv::Mat((int) nZ, (int) nXd, CV_32FC1, srcData); // rows, cols (height, width)
                        cv::resize(image, image, cv::Size((int) nXd, (int) nZd), 0, 0, cv::INTER_NEAREST);
                        dstDatasetFinal->write3DDataset(0, y, 0, nZd, 1, nXd, (float *) image.data);

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
                    //dstDatasetFinal->findAndSetGlobalMinAndMaxValue();
                    dstDatasetFinal->setAttribute("min", minVG);
                    dstDatasetFinal->setAttribute("max", maxVG);
                    dstDatasetFinal->setAttribute("dwnsmpl", (uint64_t) maxSize);
                    dstDatasetFinal->setAttribute("src_dataset_name", srcDataset->getName());
                    dstDatasetFinal->setAttribute("src_dataset_id", (uint64_t) srcDataset->getId());
                }

                // For every reshaped mask type
                for (std::map<const H5std_string, HDF5File::HDF5Group *>::iterator it = datasetsGroupType.begin(); it != datasetsGroupType.end(); ++it) {
                    HDF5File::HDF5Group *srcGroup = it->second;
                    tmpFile->createGroup(srcGroup->getName());
                    //HDF5File::HDF5Group *dstGroup = tmpFile->openGroup(srcGroup->getName());
                    hDF5OutputFile->createGroup(srcGroup->getName() + "_" + std::to_string(maxSize), true);
                    HDF5File::HDF5Group *dstGroupFinal = hDF5OutputFile->openGroup(srcGroup->getName() + "_" + std::to_string(maxSize));
                    hsize_t count = srcGroup->getNumObjs();
                    float minVG, maxVG;

                    // Compute new size
                    HDF5File::HDF5Dataset *tmp = hDF5OutputFile->openDataset(srcGroup->getName() + "/0");
                    hsize_t *dims = tmp->getDims();
                    minVG = tmp->readAttributeF("min");
                    maxVG = tmp->readAttributeF("max");
                    hsize_t nX = dims[2];
                    hsize_t nY = dims[1];
                    hsize_t nZ = dims[0];
                    hDF5OutputFile->closeDataset(srcGroup->getName() + "/0");

                    if (std::max(std::max(dims[0], dims[1]), dims[2]) <= maxSize) {
                        // Check current size
                        std::cout << "No dataset for downsampling - " + std::to_string(std::max(std::max(dims[0], dims[1]), dims[2])) + " <= " + std::to_string(maxSize) << std::endl;
                        break;
                    }

                    hsize_t nMax = std::max(std::max(nZ, nY), nX);
                    double ratio = (double) maxSize / nMax;
                    hsize_t nZd = (hsize_t) floor(nZ * ratio + 0.5); //floor( x + 0.5 )
                    hsize_t nYd = (hsize_t) floor(nY * ratio + 0.5);
                    hsize_t nXd = (hsize_t) floor(nX * ratio + 0.5);
                    if (nZd < 1) nZd = 1;
                    if (nYd < 1) nYd = 1;
                    if (nXd < 1) nXd = 1;
                    hsize_t newDatasetSize[3] = {nZd, nYd, nXd};
                    hsize_t newTmpDatasetSize[3] = {nZ, nYd, nXd};
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

                        for (unsigned int y = 0; y < nYd; y++) {
                            dstDataset->read3DDataset(0, y, 0, nZ, 1, nXd, srcData, minV, maxV);
                            cv::Mat image = cv::Mat((int) nZ, (int) nXd, CV_32FC1, srcData); // rows, cols (height, width)
                            cv::resize(image, image, cv::Size((int) nXd, (int) nZd));
                            dstData = (float *) image.data;
                            dstDatasetFinal->write3DDataset(0, y, 0, nZd, 1, nXd, dstData);
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

                    hsize_t newPositionZ = (hsize_t) floor((double) (srcGroup->readAttributeI("positionZ") + 1) * ratio + 0.5) - 1;
                    hsize_t newPositionY = (hsize_t) floor((double) (srcGroup->readAttributeI("positionY") + 1) * ratio + 0.5) - 1;
                    hsize_t newPositionX = (hsize_t) floor((double) (srcGroup->readAttributeI("positionX") + 1) * ratio + 0.5) - 1;
                    hsize_t newSizeZ = (hsize_t) floor((double) (srcGroup->readAttributeI("sizeZ") + 1) * ratio + 0.5) - 1;
                    hsize_t newSizeY = (hsize_t) floor((double) (srcGroup->readAttributeI("sizeY") + 1) * ratio + 0.5) - 1;
                    hsize_t newSizeX = (hsize_t) floor((double) (srcGroup->readAttributeI("sizeX") + 1) * ratio + 0.5) - 1;
                    if (newPositionZ < 0) newPositionZ = 0;
                    if (newPositionY < 0) newPositionY = 0;
                    if (newPositionX < 0) newPositionX = 0;
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
                    hDF5OutputFile->closeGroup(dstGroupFinal->getName());
                }

                delete tmpFile;
                remove("tmp.h5");
            }
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    // Select file to view
    if (!outputFilename.empty() && (flagRechunk || flagReshape || flagDwnsmpl))
        hDF5ViewFile = hDF5OutputFile;
    else
        hDF5ViewFile = hDF5SimulationOutputFile;

    // Simple slice visualization
    if (flagView) {
        std::cout << std::endl << std::endl << "---- Visualization ----" << std::endl << std::endl << std::endl;
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
                if (size[0] > nZ || size[1] > nY || size[1] > nX)
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
                        cv::resize(imageL, imageL, size);
                        cv::resize(imageG, imageG, size);
                        cv::resize(imageGG, imageGG, size);
                    } else {
                        int dstHeight = 300;
                        cv::Size size((int) ceil((double) imageL.size().width * dstHeight / imageL.size().height), dstHeight);
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
                    cv::resize(imageL, imageLUP, size);
                    cv::resize(imageG, imageGUP, size);
                } else {
                    int dstHeight = 300;
                    cv::Size size((int) ceil((double) imageL.size().width * dstHeight / imageL.size().height), dstHeight);
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

    std::cout << std::endl << std::endl << "---- Closing files ----" << std::endl << std::endl << std::endl;

    // Close files
    if (flagSOF)
        delete hDF5SimulationOutputFile;
    if (flagSIF)
        delete hDF5SimulationInputFile;
    if (flagOF)
        delete hDF5OutputFile;

    //return a.exec();
    std::exit(EXIT_SUCCESS);
}
