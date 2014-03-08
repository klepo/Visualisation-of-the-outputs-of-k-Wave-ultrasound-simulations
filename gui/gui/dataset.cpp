#include "dataset.h"

#include <iostream>
#include <algorithm>    // std::max
#include <sstream>
#include <string>

#include <cfloat>

#include <time.h>

#include <Windows.h>	// windows
#include <Psapi.h>      // ShowMemoryUsageInMB

#include <cpp/H5Cpp.h>  // HDF5

#include <opencv2/opencv.hpp>

Dataset::Dataset(H5std_string filename) {
    //p_max_all_DATASET_NAME = std::string("p_max_all");
    // Try block to detect exceptions raised by any of the calls inside it
    try
    {
        H5::Exception::dontPrint();
        file = H5::H5File(filename.c_str(), H5F_ACC_RDONLY);
        p_max_all_dataset = file.openDataSet("p_max_all");
        p_max_all_filespace = p_max_all_dataset.getSpace();

        //*p_max_all_data_out = NULL;
        p_max_all_X, p_max_all_Y;

        p_max_all_index = 0;
        p_max_all_maxIndex = 0;
        p_max_all_dimension = 0;

        //int t0, t1, t2, t3, t4;

        p_max_all_maxV = 0;
        p_max_all_minV = FLT_MAX;

        p_max_all_filespace.getSimpleExtentDims(p_max_all_dims);
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException error) {
        error.printError();
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error) {
        error.printError();
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error) {
        error.printError();
    }
}

cv::Mat Dataset::getImage(int dimension, int index) {
    mutex.lock();
    float *p_max_all_data_out;
    p_max_all_index = index;
    p_max_all_dimension = dimension;
    getSlabData(p_max_all_dimension, p_max_all_index, p_max_all_dataset, p_max_all_filespace, p_max_all_dims, &p_max_all_data_out, p_max_all_X, p_max_all_Y, p_max_all_maxIndex);
    getMinAndMaxValue(p_max_all_data_out, p_max_all_X, p_max_all_Y, p_max_all_minV, p_max_all_maxV);
    cv::Mat image = createImage(p_max_all_data_out, p_max_all_X, p_max_all_Y, p_max_all_minV, p_max_all_maxV);
    //image.release();
    delete [] p_max_all_data_out;
    mutex.unlock();
    return image;
}

int Dataset::getMaximum(int dim) {
    if (dim < 0 || dim > 2)
        return -1;
    return p_max_all_dims[dim];
}

void Dataset::getSlabData(int dimension, int index, H5::DataSet dataset, H5::DataSpace filespace, hsize_t dims[3], float **data_out, int &X, int &Y, int &maxIndex) {
    int s = 1;
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    if (dimension == 0) {
        offset[0] = index % (dims[0]);
        offset[1] = 0;
        offset[2] = 0;
        count[0] = 1;
        count[1] = dims[1] / s;
        count[2] = dims[2] / s;
        maxIndex = dims[0];
        X = dims[1] / s;
        Y = dims[2] / s;
    } else if (dimension == 1) {
        offset[0] = 0;
        offset[1] = index % (dims[1]);
        offset[2] = 0;
        count[0] = dims[0] / s;
        count[1] = 1;
        count[2] = dims[2] / s;
        X = dims[0] / s;
        maxIndex = dims[1];
        Y = dims[2] / s;
    } else if (dimension == 2) {
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = index % (dims[2]);
        count[0] = dims[0] / s;
        count[1] = dims[1] / s;
        count[2] = 1;
        X = dims[0] / s;
        Y = dims[1] / s;
        maxIndex = dims[2];
    } else return;

    hsize_t stride[3] = {s, s, s};
    hsize_t block[3] = {1, 1, 1};

    //std::cout << "dims " << dims[0] << " " << dims[1] << " " << dims[2] << std::endl;
    //std::cout << "count " << count[0] << " " << count[1] << " " << count[2] << std::endl;
    //std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
    //std::cout << "stride " << stride[0] << " " << stride[1] << " " << stride[2] << std::endl;
    //std::cout << "block " << block[0] << " " << block[1] << " " << block[2] << std::endl;

    filespace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t offset_out[2];   // hyperslab offset in memory
    hsize_t count_out[2];    // size of the hyperslab in memory
    offset_out[0] = 0;
    offset_out[1] = 0;
    count_out[0]  = X;
    count_out[1]  = Y;
    H5::DataSpace memspace(2, count_out);

    memspace.selectHyperslab(H5S_SELECT_SET, count_out, offset_out);
    *data_out = new float[X * Y];
    int t4 = clock();
    //std::cout << "dataset.read " << dimension << " " << index << std::endl;
    //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
    dataset.read(*data_out, H5::PredType::NATIVE_FLOAT, memspace, filespace);
    //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
    int t5 = clock();

    std::cout << "dataset_read_time; " << dimension << "; " << index << "; " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms;" << std::endl;
}

void Dataset::getMinAndMaxValue(float *data, int X, int Y, float &minV, float &maxV) {
    maxV = 0;
    minV = 9999999;
    //float p0 = 2 * pow(10, -5);
    //std::cout << "Y " << Y << " X " << X << std::endl;
    for (long y = 0; y < Y; y++) {
        for (long x = 0; x < X; x++) {
            float value = data[x * Y + y];
            //if (x <= 4 && y <= 4) std::cout << "x: " << x << " y: " << y << " " << value << std::endl;
            //value = 20 * log10(value / p0);
            if (value < minV) minV = value;
            if (value > maxV) maxV = value;
        }
    }
}

cv::Mat Dataset::createImage(float *data, int X, int Y, float minV, float maxV) {
    cv::Mat image = cv::Mat::zeros(Y, X, CV_8UC1); // rows, cols (height, width)
    //float p0 = 2 * pow(10, -5);
    for (long y = 0; y < Y; y++) {
        for (long x = 0; x < X; x++) {
            float value = data[x * Y + y];
            //value = 20 * log10(value / p0);
            value = ((value - minV) * 254) / (maxV - minV);
            image.at<uchar>(y, x) = value;
        }
    }
    cv::applyColorMap(image, image, cv::COLORMAP_JET);
    cv::resize(image, image, cv::Size(image.size().width * 180 / image.size().height, 180));
    return image;
}

/**
 * Get maximal memory usage
 * @return memory usage in MB
 */
size_t Dataset::ShowMemoryUsageInMB() {
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
    PROCESS_VM_READ,
    FALSE, GetCurrentProcessId());
    GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc));
    CloseHandle(hProcess );
    return  pmc.PeakWorkingSetSize >> 20;    // windows
}
