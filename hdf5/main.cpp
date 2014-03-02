#include <QCoreApplication>

#include "hdf5file.h"

#include <iostream>
#include <string>

#include <time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

const std::string P_DATASET("p");
const std::string P_FINAL_DATASET("p_final");
const std::string P_MAX_DATASET("p_max");
const std::string P_MAX_ALL_DATASET("p_max_all");
const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HDF5File *hDF5File;
    try {

        hDF5File = new HDF5File("../../data/input_data_128_128_128_case_1.h5");
        //hDF5File = new HDF5File("../../data/output_data_128_128_128_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_256_256_256_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_512_512_512_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_1024_1024_1024_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/NewData/Data/kidney_output_data_sc_1_linear_single_source_sensor_mode_2.h5");

        //HDF5Dataset *dataset = hDF5File->getDataset(HDF5File::SENSOR_MASK_INDEX_DATASET);
        //HDF5Dataset *dataset = hDF5File->getDataset(HDF5File::P_DATASET);

        //hDF5File->createDataset("pokus", datatype, dataspace);
        //hDF5File->createGroup("skupina");
        //hsize_t size[3] = {5, 5, 5};
        //hDF5File->createDatasetI("skupina/pokus2", 3, size);

        //for (int i = 1; i <= 1000; i++)
            //hDF5File->createGroup(std::string("skupina/") + std::to_string(i));

        HDF5Dataset *dataset = hDF5File->getDataset(SENSOR_MASK_INDEX_DATASET);

        //std::cout << dataset->readAttributeI("uint64_t") << std::endl;
        //std::cout << dataset->readAttributeF("float") << std::endl;
        //std::cout <<  dataset->readAttributeS("string") << std::endl;

        H5T_class_t DataType = dataset->getDataType();

        if (DataType == H5T_INTEGER) {
            std::cout << "min value: " << dataset->getGlobalMinValueI() << std::endl;
            std::cout << "max value: " << dataset->getGlobalMaxValueI() << std::endl;

            size_t *size = dataset->getDims();
            std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

            size_t *chunkSize = dataset->getChunkDims();
            if (chunkSize != NULL)
                std::cout << "chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;
            else
                std::cout << "chunk size: NONE" << std::endl;

            uint64_t minV;
            uint64_t maxV;

            uint64_t *data = dataset->readDatasetI(0, 0, 0, 1, 1, size[2], minV, maxV);

            std::cout << "minV: " << minV << "; maxV: " << maxV << std::endl;

        } else if (DataType == H5T_FLOAT) {
            //std::cout << "min value: "  << dataset->getGlobalMinValueF() << std::endl;
            //std::cout << "max value: "  << dataset->getGlobalMaxValueF() << std::endl;

            uint64_t height = 256;
            uint64_t width = 256;

            float minV;// = dataset->getGlobalMinValueF();
            float maxV;// = dataset->getGlobalMaxValueF();


            size_t *size = dataset->getDims();
            //std::cout << "size: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

            float *data = dataset->readDatasetF(0, 100, 0, 1, 1, size[2], minV, maxV);


            /*size_t *chunkSize = dataset->getChunkDims();
            if (chunkSize != NULL)
                std::cout << "chunk size: " << chunkSize[0] << " x " << chunkSize[1] << " x " << chunkSize[2] << std::endl;
            else
                std::cout << "chunk size: NONE" << std::endl;*/

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
                    image.at<uchar>(y, x) = value;
                }
            }

            cv::applyColorMap(image, image, cv::COLORMAP_JET);
            cv::resize(image, image, cv::Size(image.size().width * 300 / image.size().height, 300));
            char windowname[8] = "My View";
            cv::imshow(windowname, image);

            delete [] data;
        }

        delete hDF5File;

    } catch(std::exception e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return a.exec();
    //std::exit(EXIT_SUCCESS);
}
