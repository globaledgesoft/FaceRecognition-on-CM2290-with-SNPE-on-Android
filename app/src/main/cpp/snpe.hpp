#ifndef SNPE_H
#define SNPE_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <DlContainer/IDlContainer.hpp>
#include <DlSystem/RuntimeList.hpp>
#include <SNPE/SNPE.hpp>
#include <SNPE/SNPEBuilder.hpp>
#include <DlSystem/UDLFunc.hpp>
#include <DlSystem/ITensorFactory.hpp>
#include <SNPE/SNPEFactory.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <android/log.h>
#include <chrono>
#include <ctime>

typedef std::chrono::milliseconds ms; 

class snpe {
    private:
        std::shared_ptr<zdl::SNPE::SNPE> handler;
        std::shared_ptr<zdl::DlContainer::IDlContainer> dlcontainer;
        zdl::DlSystem::RuntimeList runtime;
        zdl::DlSystem::StringList outputs;
        zdl::DlSystem::TensorMap tensor_map;
        zdl::DlSystem::StringList tensors_list;

    public:
        snpe(std::string &dlc, int system_type);
        snpe(const snpe &qc);
		std::map<std::string, std::vector<float>> predict(cv::Mat image);
        std::vector<float> throughput_vec;
        std::vector<float> fps_vec;
		bool keypoint_det_mode = true;
};

#endif
