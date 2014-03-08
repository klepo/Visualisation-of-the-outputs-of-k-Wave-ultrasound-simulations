//#include <QCoreApplication>

#include "hdf5file/hdf5file.h"

#include <iostream>
#include <string>

#include <time.h>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

const std::string P_DATASET("p");
const std::string P_FINAL_DATASET("p_final");
const std::string P_MAX_DATASET("p_max");
const std::string P_MAX_ALL_DATASET("p_max_all");
const std::string SENSOR_MASK_INDEX_DATASET("sensor_mask_index");

int main(int argc, char **argv)
{
    //QCoreApplication a(argc, argv);

    //std::cout << "argc: " << argc << " argv: " << argv[1] << std::endl;

    if (argc != 2) {
        std::cerr << "Wrong parameters" << std::endl;
        return 1;
    }

    HDF5File *hDF5File;
    try {

        hDF5File = new HDF5File(argv[1], HDF5File::OPEN);
        //hDF5File = new HDF5File("../../data/input_data_128_128_128_case_1.h5");
        //hDF5File = new HDF5File("../../data/output_data_128_128_128_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_256_256_256_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_512_512_512_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/output_data_1024_1024_1024_case_1_no_compression.h5");
        //hDF5File = new HDF5File("../../data/NewData/Data/kidney_output_data_sc_1_linear_single_source_sensor_mode_2.h5");

        //HDF5Dataset *dataset = hDF5File->getDataset(HDF5File::SENSOR_MASK_INDEX_DATASET);
        //HDF5Dataset *dataset = hDF5File->getDataset(HDF5File::P_DATASET);

        //hDF5File->createDataset("pokus", datatype, dataspace);
        //hDF5File->createGroup("skupina2");
        hsize_t size[3] = {50, 50, 50};
        hsize_t chunk_size[3] = {5, 12, 2};
        hDF5File->createDatasetF("pokus2", 3, size);
        hDF5File->createDatasetF("pokus3", 3, size, chunk_size);
        //hDF5File->unlinkLocation("pokus2");
        // TODO remove dataset, chunk, delete buffer

        //for (int i = 1; i <= 1000; i++)
            //hDF5File->createGroup(std::string("skupina/") + stdp::to_string(i));


        HDF5File::HDF5Group *group = hDF5File->getGroup("/");

        //HDF5File::HDF5Group::HDF5Attribute attr = group->getAttribute("uint64_t");
        //attr.getAttribute ();
        //std::cout << group->readAttributeS("created_by") << std::endl;
        //std::cout << group->readAttributeS("uint64_t") << std::endl;
        //std::cout << dataset->readAttributeF("float") << std::endl;
        //std::cout <<  dataset->readAttributeS("string") << std::endl;

        HDF5File::HDF5Dataset *dataset = hDF5File->getDataset("p_max_all");
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
