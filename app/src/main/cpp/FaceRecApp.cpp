#include "FaceRecApp.h"
#include <unistd.h>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <string>
#include <cstdlib>
#include <mutex>
#include <glob.h>
#include <dirent.h>





FaceRecApp::FaceRecApp()
    : cameraready(false), image(nullptr), input_image(nullptr), m_Camera_Input(nullptr){}

FaceRecApp::~FaceRecApp(){
    JNIEnv *env;
    java_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    env->DeleteGlobalRef(calling_activity_obj);
    calling_activity_obj = nullptr;

    if (m_Camera_Input != nullptr) {
        delete m_Camera_Input;
        m_Camera_Input = nullptr;
    }

    if (native_window != nullptr) {
        ANativeWindow_release(native_window);
        native_window = nullptr;
    }

    if (input_image != nullptr) {
        delete (input_image);
        input_image = nullptr;
    }
}

void FaceRecApp::OnCreate() {
    facerec.load(face_detection_model_path);
    load_model = new snpe(face_recognition_model_path, 2);
    embeddings(database_images_path.c_str());
}


void FaceRecApp::SetNativeWindow(ANativeWindow* nativeWindow) {
    // Save native window
    nativeWindow = nativeWindow;
}

void FaceRecApp::SetUpCamera() {

    m_Camera_Input = new Camera_Input(camera_type);
    m_Camera_Input->MatchCaptureSizeRequest(&view,
                                             ANativeWindow_getWidth(native_window),
                                             ANativeWindow_getHeight(native_window));

    ANativeWindow_setBuffersGeometry(native_window, view.width, view.height,
                                     WINDOW_FORMAT_RGBX_8888);
    input_image = new Input_Image(&view, AIMAGE_FORMAT_YUV_420_888);
    input_image->SetPresentRotation(m_Camera_Input->GetOrientation());
    ANativeWindow* image_reader_window = input_image->GetNativeWindow();
    cameraready = m_Camera_Input->CreateCaptureSession(image_reader_window);
}

void FaceRecApp::CameraLoop() {
    bool b_p= false;

    while (1) {
        if (camera_thread) { break; }
        if (!cameraready || !input_image) { continue; }
        //reading the image from ndk reader
        image = input_image->GetNextImage();
        if (image == nullptr) { continue; }

        ANativeWindow_acquire(native_window);
        ANativeWindow_Buffer buffer;
        if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
            input_image->DeleteImage(image);
            image = nullptr;
            continue;
        }
        if (false == b_p) {
            b_p = true;
            LOGI("/// %d, %d, %d, %d", buffer.height, buffer.width, buffer.stride,
                 buffer.format);
        }

        //display the image
        input_image->DisplayImage(&buffer, image);
        //converting the ndk image into opencv format
        img_mat = cv::Mat(buffer.height, buffer.stride, CV_8UC4, buffer.bits);

        grey_image = cv::Mat(img_mat.rows, img_mat.cols, CV_8UC1);
        cv::cvtColor(img_mat, grey_image, cv::COLOR_RGBA2GRAY);

        rgb_image = cv::Mat(img_mat.rows, img_mat.cols, CV_8UC3);
        cv::cvtColor(img_mat, rgb_image, cv::COLOR_RGBA2RGB);

        res_image = cv::Mat(270, 480, CV_8UC1);
        cv::resize(grey_image, res_image, cv::Size(480, 270));

        i++;
        if (i%2==0) {
            i=1;

            std::vector<cv::Rect> faces;

            facerec.detectMultiScale(res_image, faces);

            for (i = 0; i < faces.size(); i++) {
                int x1 = (int) ((faces[i].x / 480.0) * 1920);
                int y1 = (int) (((faces[i].y / 270.0) * 1080));
                int x2 = (int) (((faces[i].x + faces[i].width) / 480.0) * 1920);
                int y2 = (int) (((faces[i].y + faces[i].height) / 270.0) * 1080);

                LOGI("___________x1  %d\t, y1  %d\t, x2  %d\t, y2  %d\n", faces[i].x, faces[i].y,
                     faces[i].width, faces[i].height);
                LOGI("___________x1  %d\t, y1  %d\t, x2  %d\t, y2  %d\n", x1, y1, x2, y2);
                LOGI("--------rgb %d  %d", rgb_image.size[0], rgb_image.size[1]);

                cv::Mat output = rgb_image(cv::Range(y1, y2), cv::Range(x1, x2));
                cv::Mat result = cv::Mat(face_inp_shape, face_inp_shape, CV_8UC3);
                cv::resize(output, result, cv::Size(face_inp_shape, face_inp_shape));

                pred_res = load_model->predict(result);
                for (auto j = pred_res.begin(); j != pred_res.end(); j++) {
                    double val = 0;
                    for (float k : j->second) {
                        val += pow(k, 2);
                    }
                    val = std::sqrt(val);
                    float emd[512] = {0};
                    for (int k = 0; k < j->second.size(); k++) {
                        emd[k] = j->second[k] / val;
                    }
                    int p=0;
                    for (auto k = get_faces.begin(); k != get_faces.end(); k++) {
                        p++;
                        float d = euclediandist(emd, &k->second[0]);
                        LOGI("   %f\n", d);

                        if (d < 0.7) {
                            cv::rectangle(img_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 180, 0),10);
                            cv::Size rect = cv::getTextSize(k->first, cv::FONT_HERSHEY_DUPLEX, 2, 2, 0);
                            cv::rectangle(img_mat, cv::Point(x1, y1), cv::Point(x1 + rect.width+20, y1 + rect.height+20),
                                          cv::Scalar(0, 180, 0), -1);
                            cv::putText(img_mat, k->first, cv::Point(x1+20, y1 + 50),
                                        cv::FONT_HERSHEY_DUPLEX, 1.7, cv::Scalar(255, 255, 255), 2,
                                        false);
                            break;
                        } else if (p == get_faces.size()){
                            cv::rectangle(img_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 180, 0),10);
                            cv::Size rect = cv::getTextSize("Undefined", cv::FONT_HERSHEY_DUPLEX, 2, 2, 0);
                            cv::rectangle(img_mat, cv::Point(x1, y1), cv::Point(x1+rect.width+20, y1 + rect.height+20),
                                          cv::Scalar(180, 0, 0), -1);
                            cv::putText(img_mat, "Undefined", cv::Point(x1+20, y1 + 50),
                                        cv::FONT_HERSHEY_DUPLEX, 1.7,  cv::Scalar(255, 255, 255), 2,
                                        false);
                        }
                    }
                }
            }
        }
        pred_res.clear();
        ANativeWindow_unlockAndPost(native_window);
        ANativeWindow_release(native_window);
    }
}

float FaceRecApp::euclediandist(float* pr, float *gt){
    float dist = 0;
    for(int k=0; k < out_model_vec; k++){
        dist += pow((*(pr + k) - (*(gt + k))), 2);
    }
    return dist;
}

void FaceRecApp::embeddings(const char *dir_name){
    std::string tot_img_dir = dir_name;
    struct dirent *img_dir;

    DIR *tot_img_ptr = opendir(dir_name);
    if (tot_img_ptr == NULL) {
        return;
    }

    while ((img_dir = readdir(tot_img_ptr)) != NULL) {
        std::string img_dir_name = img_dir->d_name;

        if(strcmp(img_dir_name.c_str(), ".") == 0 || strcmp(img_dir_name.c_str(), "..") == 0){
            continue;
        }
        std::string img_dir_path = tot_img_dir + img_dir_name;


        struct dirent *img;
        DIR *img_dir_ptr = opendir(img_dir_path.c_str());

        while ((img = readdir(img_dir_ptr)) != NULL) {
            if(strcmp(img->d_name, ".") == 0 || strcmp(img->d_name, "..") == 0){
                continue;
            }
            std::string img_abs_path = img_dir_path + "/" +img->d_name;

            LOGI("))))))))))))))))))))))))))      %s\n", img_abs_path.c_str());

            cv::Mat gt_img = cv::imread(img_abs_path.c_str());
            cv::Mat gre_img;
            cv::cvtColor(gt_img, gre_img, cv::COLOR_BGR2GRAY);
            std::vector<cv::Rect> faces;
            facerec.detectMultiScale(gre_img, faces);
            for (i = 0; i < faces.size(); i++) {
                int x1 = (int)(faces[i].x);
                int y1 = (int)(faces[i].y);
                int x2 = (int)(faces[i].x + faces[i].width);
                int y2 = (int)(faces[i].y + faces[i].height);

                LOGI("___________x1  %d\t, y1  %d\t, x2  %d\t, y2  %d\n", x1, y1, x2, y2);
                LOGI("--------- %d %d", gt_img.rows, gt_img.cols);

                cv::Mat out_face = gt_img(cv::Range(y1, y2), cv::Range(x1, x2));
                cv::Mat res_img;
                cv::resize(out_face, res_img, cv::Size(face_inp_shape, face_inp_shape));

                pred_res = load_model->predict(res_img);

                for (auto j = pred_res.begin(); j != pred_res.end(); j++) {

                    double val = 0;
                    for (float k : j->second) {
                        val += pow(k, 2);
                    }
                    val = std::sqrt(val);
                    float emd[512] = {0};
                    for (int k = 0; k < j->second.size(); k++) {
                        emd[k] = j->second[k] / val;
                    }
                    std::vector<float> vec(emd, emd + out_model_vec);
                    get_faces.insert(std::make_pair(img_dir->d_name, vec));
                }
            }
        }
        closedir(img_dir_ptr);
    }
    closedir(tot_img_ptr);
}