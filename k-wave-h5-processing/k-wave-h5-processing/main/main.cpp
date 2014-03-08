//#include <QCoreApplication>

#include <hdf5file.h>

#include <iostream>
#include <string>
#include <map>

#include <time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const std::string P_DATASET("p");
const std::string P_FINAL_DATASET("p_final");
const std::string P_MAX_DATASET("p_max");
const std::string P_MAX_ALL_DATASET("p_max_all");
const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");

#define MAX_SIZE 256;
#define CHUNK_SIZE 10;
#define SIZE_OF_DATA_PART 262144;

std::string h5repackFilepath = "h5repack";
std::string simulationOutputFilename = "";
std::string simulationIutputFilename = "";
std::string outputFilename = "";
bool sensorMaskFlag = false;
hsize_t maxSize = MAX_SIZE
hsize_t chunkSize = CHUNK_SIZE

void getParams(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -f (simulation output filename)" << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationOutputFilename = argv[i];
            std::cout << "simulation output filename:\n\t" << simulationOutputFilename << std::endl << std::endl;

        } else if (strcmp("-m", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -m (simulation iutput filename)" << std::endl;
                exit(EXIT_FAILURE);
            }
            simulationIutputFilename = argv[i];
            std::cout << "simulation iutput filename:\n\t" << simulationIutputFilename << std::endl << std::endl;
            continue;
        } else if (strcmp("-o", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -o (output filename)" << std::endl;
                exit(EXIT_FAILURE);
            }
            outputFilename = argv[i];
            std::cout << "output filename:\n\t" << outputFilename << std::endl << std::endl;

        } else if (strcmp("-r", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -r (h5repack tool file path)" << std::endl;
                exit(EXIT_FAILURE);
            }
            h5repackFilepath = argv[i];
            std::cout << "h5repack tool file path:\n\t" << h5repackFilepath << std::endl << std::endl;

        } else if (strcmp("-s", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -s (max size for downsampling)" << std::endl;
                exit(EXIT_FAILURE);
            }
            try {
                size_t s;
                maxSize = std::stoi(argv[i], &s);
                if (strlen(argv[i]) != s)
                    throw std::invalid_argument(argv[i]);
                std::cout << "max size for downsampling:\n\t" << maxSize << std::endl << std::endl;
            } catch (std::invalid_argument error) {
                std::cerr << "Wrong parameter -s (max size for downsampling)" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (strcmp("-ch", argv[i]) == 0){
            i++;
            if (argc <= i) {
                std::cerr << "Wrong parameter -ch (chunk size)" << std::endl;
                exit(EXIT_FAILURE);
            }
            try {
                size_t s;
                chunkSize = std::stoi(argv[i], &s);
                if (strlen(argv[i]) != s)
                    throw std::invalid_argument(argv[i]);
                std::cout << "chunk size:\n\t" << chunkSize << std::endl << std::endl;
            } catch (std::invalid_argument error) {
                std::cerr << "Wrong parameter -ch (chunk size)" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Wrong parameters (" << argv[i] << ")" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv)
{
    //QCoreApplication a(argc, argv);

    getParams(argc, argv); // Load parameters

    if (simulationOutputFilename.empty()) {
        std::cerr << "Missing parameter -f (simulation output filename)" << std::endl;
        exit(EXIT_FAILURE);
    }

    HDF5File *hDF5SimulationOutputFile;
    HDF5File *hDF5SimulationInputFile;


    // Load simulation output file
    try {
        hDF5SimulationOutputFile = new HDF5File(simulationOutputFilename, HDF5File::OPEN);
    } catch(std::exception e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    HDF5File::HDF5Dataset *sensorMaskIndexDataset;

    // Find and get sensor mask dataset
    try {
        sensorMaskIndexDataset = hDF5SimulationOutputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
        sensorMaskFlag = true;
    } catch(std::exception e) {
        //std::cerr << e.what() << std::endl;
        std::cout << "Sensor mask is not in simulation output file" << std::endl;
        // Try to load sensor mask from simulation input file
        if (simulationIutputFilename.empty()) {
            std::cout << "Missing sensor mask" << std::endl;
        } else
            try {
                hDF5SimulationInputFile = new HDF5File(simulationIutputFilename, HDF5File::OPEN);
                sensorMaskIndexDataset = hDF5SimulationInputFile->openDataset(SENSOR_MASK_INDEX_DATASET);
                sensorMaskFlag = true;
            } catch(std::exception e) {
                //std::cerr << e.what() << std::endl;
                std::cout << "Sensor mask is not in simulation input file" << std::endl;
            }
    }

    hsize_t num = hDF5SimulationOutputFile->getNumObjs();
    HDF5File::HDF5Dataset *dataset;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasets_mask_type;
    std::map<const H5std_string, HDF5File::HDF5Dataset *> datasets_3D_type;
    uint64_t nT = hDF5SimulationOutputFile->getNT();
    uint64_t nX = hDF5SimulationOutputFile->getNX();
    uint64_t nY = hDF5SimulationOutputFile->getNY();
    uint64_t nZ = hDF5SimulationOutputFile->getNZ();

    uint64_t sensorMaskSize = 0;
    if (sensorMaskFlag){
        hsize_t *size = sensorMaskIndexDataset->getDims();
        if (sensorMaskIndexDataset->getRank() == 3 && size[0] == 1 && size[1] == 1) {
            sensorMaskSize = size[2];
        } else {
            sensorMaskFlag = false;
            std::cerr << "Wrong sensor mask" << std::endl;
        }
    }

    // Find datasets for visualization
    for (hsize_t i = 0; i < num; i++) {
        try {
            dataset = hDF5SimulationOutputFile->openDataset(i);

            hsize_t *size = dataset->getDims();

            // 3D type
            if (dataset->getRank() == 3 && size[0] == nZ && size[1] == nY && size[2] == nX) {
                datasets_3D_type.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dataset->getName(), dataset));
                std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
            }
            // Sensor mask type
            else if (sensorMaskFlag && dataset->getRank() == 3 && size[0] == 1 && size[1] <= nT && size[2] == sensorMaskSize) {
                datasets_mask_type.insert(std::pair<const H5std_string, HDF5File::HDF5Dataset*>(dataset->getName(), dataset));
                std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
            } else
                hDF5SimulationOutputFile->closeDataset(dataset->getName());

        } catch(std::exception e) {
            std::cout << "Object " << i << " is not dataset" << std::endl;
        }
    }

    // Processing data stored using a mask



    return 0;



    HDF5File *hDF5File;
    try {

        hDF5File = new HDF5File(argv[1], HDF5File::OPEN);
        //hDF5File = new HDF5File("../../data/input_data_128_128_128_case_1.h5");
        //hDF5File = new HDF5File("../../data/output_data_128_128_128_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_256_256_256_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_512_512_512_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_1024_1024_1024_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/NewData/Data/kidney_output_data_sc_1_linear_single_source_sensor_mode_2.h5");

        //HDF5Dataset *dataset = hDF5File->openDataset(HDF5File::SENSOR_MASK_INDEX_DATASET);
        //HDF5Dataset *dataset = hDF5File->openDataset(HDF5File::P_DATASET);

        std::string datasets = "p_max_all";
        std::string chunk = std::to_string(10) + "x" + std::to_string(5) + "x" + std::to_string(20);
        std::string cmd = "e:/HDF5-1.8.12-win64-static-with-zlib_szip/bin/h5repack";
        std::string inFile = argv[1];
        size_t lastindex = inFile.find_last_of(".");
        std::string rawname = inFile.substr(0, lastindex);
        std::string outFile = rawname + "_modified.h5";
        int ret = system((cmd + " -l " + datasets + ":CHUNK=" + chunk + " " + inFile + " " + outFile).c_str());
        std::cout << ret << std::endl;
        return 0;

        //hDF5File->createDataset("pokus", datatype, dataspace);
        //hDF5File->createGroup("skupina2");
        hsize_t size[3] = {50, 50, 50};
        hsize_t chunk_size[3] = {5, 12, 2};
        hDF5File->createDatasetF("pokus2", 3, size);
        hDF5File->createDatasetF("pokus3", 3, size, chunk_size);
        //hDF5File->unlinkLocation("pokus2");

        //for (int i = 1; i <= 1000; i++)
            //hDF5File->createGroup(std::string("skupina/") + stdp::to_string(i));


        //HDF5File::HDF5Group *group = hDF5File->getGroup("/");

        //HDF5File::HDF5Group::HDF5Attribute attr = group->getAttribute("uint64_t");
        //attr.getAttribute ();
        //std::cout << group->readAttributeS("created_by") << std::endl;
        //std::cout << group->readAttributeS("uint64_t") << std::endl;
        //std::cout << dataset->readAttributeF("float") << std::endl;
        //std::cout <<  dataset->readAttributeS("string") << std::endl;

        //HDF5File::HDF5Dataset *dataset = hDF5File->openDataset("p_max_all");
        //dataset->setAttribute("i", 456);
        //dataset->setAttribute("f", 4564.6546);
        //dataset->setAttribute("s", 45654);
        //group->setAttribute(dataset->getAttribute(0));
        //group->setAttribute(dataset->getAttribute("s"));
        //dataset->removeAttribute (0);
        std::cout << dataset->getNumAttrs() << std::endl;

        H5T_class_t DataType = dataset->getDataType();

        if (DataType == H5T_INTEGER) {
            std::cout << "min value: " << dataset->getGlobalMinValueI() << std::endl;
            std::cout << "max value: " << dataset->getGlobalMaxValueI() << std::endl;

            hsize_t *size = dataset->getDims();
            std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

            hsize_t *chunkSize = dataset->getChunkDims();
            if (chunkSize != NULL)
                std::cout << "chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;
            else
                std::cout << "chunk size: NONE" << std::endl;

            //uint64_t minV;
            //uint64_t maxV;

            //uint64_t *data = dataset->readDatasetI(0, 0, 0, 1, 1, size[2], minV, maxV);

            //std::cout << "minV: " << minV << "; maxV: " << maxV << std::endl;

        } else if (DataType == H5T_FLOAT) {
            std::cout << "min value: "  << dataset->getGlobalMinValueF() << std::endl;
            std::cout << "max value: "  << dataset->getGlobalMaxValueF() << std::endl;


            float minV;// = dataset->getGlobalMinValueF();
            float maxV;// = dataset->getGlobalMaxValueF();


            hsize_t *size = dataset->getDims();
            std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

            uint64_t height = size[2];
            uint64_t width = size[1];

            float *data = dataset->readDatasetF(0, 0, 0, 1, size[1], size[2], minV, maxV);

            //float *data = dataset->readDatasetF(100, 0, 0, 1, size[1], size[2], minV, maxV);

            //dataset1->writeDataset(100, 0, 0, 1, 512, 512, data);

            hsize_t *chunkSize = dataset->getChunkDims();
            if (chunkSize != NULL)
                std::cout << "chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;
            else
                std::cout << "chunk size: NONE" << std::endl;

            //float *data;// = dataset->readSlabOfDatasetF(0, 50, height, width, minV, maxV);
            std::cout << "minV: " << minV << "; maxV: " << maxV << std::endl;
            /*srand (time(NULL));
            for (int i = 0; i < 15; i++) {
                int dim = rand() % 3;
                data = dataset->readSlabOfDatasetF(dim, rand() % size[dim], height, width, minV, maxV);
            }*/

            cv::Mat image = cv::Mat::zeros((int) height, (int) width, CV_8UC1); // rows, cols (height, width)
            for (uint64_t y = 0; y < height; y++) {
                for (uint64_t x = 0; x < width; x++) {
                    float value = data[x * height + y];
                    //std::cout << value << std::endl;
                    value = ((value - minV) * 255) / (maxV - minV);
                    image.at<uchar>((int) y, (int) x) = cv::saturate_cast<uchar>(value);
                }
            }

            cv::applyColorMap(image, image, cv::COLORMAP_JET);
            cv::resize(image, image, cv::Size(image.size().width * 300 / image.size().height, 300));
            std::string windowname = "My View";
            cv::imshow(windowname, image);

            delete [] data;
        }

        delete hDF5File;

    } catch(std::exception e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    cvWaitKey(0);
    //return a.exec();
    //std::exit(EXIT_SUCCESS);
}
