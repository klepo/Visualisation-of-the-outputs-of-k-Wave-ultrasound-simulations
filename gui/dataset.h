#ifndef DATASET_H
#define DATASET_H

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

#include <QMutex>

class Dataset {

    public:
        Dataset(H5std_string filename);
        cv::Mat getImage(int dimension, int index);
        int getMaximum(int dim);

    private:
        QMutex mutex;
        std::string p_max_all_DATASET_NAME;
        H5::H5File file;
        H5::DataSet p_max_all_dataset;
        H5::DataSpace p_max_all_filespace;
        //float *p_max_all_data_out;
        int p_max_all_X, p_max_all_Y;
        int p_max_all_index;
        int p_max_all_maxIndex;
        int p_max_all_dimension;
        float p_max_all_maxV;
        float p_max_all_minV;
        hsize_t p_max_all_dims[3];

        void getSlabData(int dimension, int index, H5::DataSet dataset, H5::DataSpace filespace, hsize_t dims[3], float **data_out, int &X, int &Y, int &maxIndex);
        void getMinAndMaxValue(float *data, int X, int Y, float &minV, float &maxV);
        cv::Mat createImage(float *data, int X, int Y, float minV, float maxV);
        size_t ShowMemoryUsageInMB();
};

#endif // DATASET_H
