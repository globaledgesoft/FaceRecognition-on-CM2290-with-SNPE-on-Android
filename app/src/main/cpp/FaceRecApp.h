
#ifndef FACERECOGNITION_FACERECAPP_H
#define FACERECOGNITION_FACERECAPP_H


#include <android/asset_manager.h>
#include <android/native_window.h>
#include <jni.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/features2d.hpp>

#include "Input_Image.h"
#include "Camera_Input.h"
#include "Util.h"

#include <unistd.h>
#include <time.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include "snpe.hpp"


class FaceRecApp{
public:
    FaceRecApp();
    ~FaceRecApp();
    FaceRecApp(const FaceRecApp& other) = delete;
    FaceRecApp& operator=(const FaceRecApp& other) = delete;

    void OnCreate();
    void SetJavaVM(JavaVM* pjava_vm) { java_vm = pjava_vm; }
    void SetNativeWindow(ANativeWindow* nativeWindow);

    void SetUpCamera();

    void CameraLoop();
private:
    JavaVM* java_vm;
    jobject calling_activity_obj;
    ANativeWindow* native_window;
    Camera_Input* m_Camera_Input;
    camera_type camera_type = BACK_CAMERA; // Default
    ImageFormat view{0, 0, 0};
    Input_Image* input_image;
    AImage* image;

    volatile bool cameraready;
    // for timing OpenCV bottlenecks
    clock_t start_t, end_t;
    // Used to detect up and down motion
    bool scan_mode;

    // OpenCV values
    cv::Mat img_mat;
    cv::Mat rgb_image;
    cv::Mat crop;
    cv::Mat grey_image;
    cv::Mat res_image;

    bool camera_thread = false;
    snpe *load_model;
    int i;

    std::string face_detection_model_path = "/storage/emulated/0/Database/models/haarcascade_frontalface_alt.xml";
    std::string database_images_path = "/storage/emulated/0/Database/images/";
    std::string face_recognition_model_path = "/storage/emulated/0/Database/models/face_rec_align_corners_false.dlc";

    std::map<std::string, std::vector<float>> pred_res;
    std::map<std::string, std::vector<float>> get_faces;
    cv::CascadeClassifier facerec;
    const int out_model_vec = 512;
    int face_inp_shape = 112;

    void embeddings(const char *dir_name);
    float euclediandist(float * , float * );
};


#endif //ONETRY_FACERECAPP_H
