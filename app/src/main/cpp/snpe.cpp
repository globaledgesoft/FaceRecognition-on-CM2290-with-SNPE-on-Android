#include <cstdio>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <functional>
#include <thread>
#include "snpe.hpp"
#include "Util.h"
#include <stdlib.h>

/************************************************************************
* Name : snpe <Constructor>
* Function: Constructor checks runtime availability, set runtime, set
*           output layers and then loads the DLC model to model handler 
************************************************************************/
snpe::snpe(std::string &dlc, int system_type) {
    std::ifstream dlc_file(dlc);
    zdl::DlSystem::Runtime_t runtime_cpu = zdl::DlSystem::Runtime_t::CPU;
    zdl::DlSystem::Runtime_t runtime_gpu = zdl::DlSystem::Runtime_t::GPU_FLOAT16;
    zdl::DlSystem::Runtime_t runtime_dsp = zdl::DlSystem::Runtime_t::DSP;
    zdl::DlSystem::Runtime_t runtime_aip = zdl::DlSystem::Runtime_t::AIP_FIXED8_TF;
    zdl::DlSystem::PerformanceProfile_t perf = zdl::DlSystem::PerformanceProfile_t::HIGH_PERFORMANCE;

    if (!dlc_file) {
        LOGI("%s\n", "Not a valid Dlc file  ");
        exit(0);
    } else {
        LOGI("%s\n", "Created Dlc file.");
    }

    //Loading Model and setting Runtime
    static zdl::DlSystem::Version_t Version = zdl::SNPE::SNPEFactory::getLibraryVersion();
    LOGI("%s %s %s\n","--------------SNPE Version: ", Version.asString().c_str(), "-----------------");
    dlcontainer = zdl::DlContainer::IDlContainer::open(zdl::DlSystem::String(dlc.c_str()));
    if (dlcontainer == nullptr) {
        LOGI("%s\n", "Error in opening the container file");
    }

    switch(system_type) {
        case 3: if(zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_aip)) {
                    runtime.add(runtime_aip);
                    LOGI("%s\n", "AIP added to runtime");
                }
                else
                    LOGI("%s\n", "AIP runtime not available");
        case 2: if(zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_dsp)) {
                    runtime.add(runtime_dsp);
                    LOGI("%s\n","DSP added to runtime");
                }
                else
                    LOGI("%s\n","DSP runtime not available");
        case 1: if(zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_gpu)) {
                    runtime.add(runtime_gpu);
                    LOGI("%s\n","GPU added to runtime");
                }
                else
                    LOGI("%s\n","GPU runtime not available");
//                break;
        case 0: if(zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_cpu)) {
                    runtime.add(runtime_cpu);
                    LOGI("%s\n","CPU added to runtime");
                }
                else
                    LOGI("%s\n","CPU runtime not available");
                break;
        default: LOGI("%s\n","Runtime is not valid");
                 runtime.add(runtime_cpu);
                 break;
    }
    if(runtime.size()>1) {
        LOGI("%s\n","Multiple runtime available");
    }
    zdl::SNPE::SNPEBuilder snpeBuilder(dlcontainer.get());
    handler = snpeBuilder.setOutputLayers({})
        .setRuntimeProcessorOrder(runtime)
        .setPerformanceProfile(perf)
        .build();

    if (handler == nullptr) {
        LOGI("%s\n","Error during SNPE object object creation. ");
    }

}

snpe::snpe(const snpe& qc) {
    handler = std::move(qc.handler);
    dlcontainer = std::move(qc.dlcontainer);
    runtime= qc.runtime;
    outputs = qc.outputs;
    tensor_map = qc.tensor_map;
    tensors_list = qc.tensors_list;
}

/************************************************************************
* Name : predict
************************************************************************/
std::map<std::string, std::vector<float>> snpe::predict(cv::Mat image) {

    unsigned long int in_size = 1;
    const zdl::DlSystem::TensorShape i_tensor_shape = handler->getInputDimensions();
    const zdl::DlSystem::Dimension *shapes = i_tensor_shape.getDimensions();
    size_t image_size = image.channels() * image.cols * image.rows;
    for(int i=1; i<i_tensor_shape.rank(); i++) {
      in_size *= shapes[i];
    }

    if(in_size != image_size) {
        LOGI("%s\n", "Input image Size mismatch!");
        LOGI("%s\n", "Expected image size: ");
        LOGI("%s\n", "Got image size: ");
    }
    std::unique_ptr<zdl::DlSystem::ITensor> input_tensor =
        zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(handler->getInputDimensions());
    zdl::DlSystem::ITensor *tensor_ptr = input_tensor.get();
    if(tensor_ptr == nullptr) {
        LOGI("%s\n", "Could not create SNPE input tensor");
    }
    float img_mean[3] = {0.485, 0.456, 0.406};
    float img_std[3] = {0.229, 0.224, 0.225};
    //Preprocess
    float *tensor_ptr_fl = reinterpret_cast<float *>(&(*input_tensor->begin()));
    for(auto i=0; i<image_size; i++) {
        tensor_ptr_fl[i] = static_cast<float>((((float)image.data[i] / 255.f) - img_mean[i%3]) / img_std[i%3]);
    }

    //infer
    auto start = std::chrono::high_resolution_clock::now();
    bool exec_status = handler->execute(tensor_ptr, tensor_map);
    auto end = std::chrono::high_resolution_clock::now();
    if (!exec_status) {
        LOGI("%s\n", "Error in executing the network");
    }
    std::chrono::duration<float> elapsed_time = (end - start);
    std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    LOGI("%s %f\n", "FPS: ", 1.0/elapsed_time.count());

    //postprocess step
    tensors_list = tensor_map.getTensorNames();
    std::map<std::string, std::vector<float>> out_itensor_map;
    for(size_t i=0; i<tensors_list.size(); i++) {
        zdl::DlSystem::ITensor *out_itensor = tensor_map.getTensor(tensors_list.at(i));
        std::vector<float> out_vec { reinterpret_cast<float *>(&(*out_itensor->begin())), reinterpret_cast<float *>(&(*out_itensor->end()))};
        out_itensor_map.insert(std::make_pair(std::string(tensors_list.at(i)), out_vec));
    }

    return out_itensor_map;
}
