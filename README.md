# Face Recognition on CM2290 with SNPE on Android
## Introduction
This project is intended to build and deploy a face recognition on CM2290 with SNPE on android application using ArcFace model with Integration on Thundercomm TurboX CM2290 LA (Linux Android) DK (Development Kit), which helps to deploy AI Camera Applications on edge device. An android face detection API identify face from live video, and display the information about faces.


## Prerequisites 
- Download the android studio based on platform. Refer the link provided (https://developer.android.com/studio?gclid=EAIaIQobChMI8qLmyajE-wIVmX4rCh1npgMDEAAYASAAEgLQAPD_BwE&gclsrc=aw.ds).
- Install Android Platform tools (ADB, Fastboot) on the host system. 
- Download OpenCV android SDK, select releases from library menu and select andriod. Refer the given link (https://opencv.org/)
- Download and Install the Application Tool chain SDK. (Setup instructions can be found in Application SDK User Manual).
- Setup the SNPE SDK in the host system, download SNPE SDK here and select Tools and Resources and Download version 1.51.



## Steps to build and deploy Face Recognition on CM2290 with SNPE on Android Application

  1.	Clone the project on the host system.
  ```sh
     $ git clone <THIS_REPO>
     $ cd <CLONED_SOURCE>/
  ```
  2.	Launch Andriod studio by executing ‘studio.sh’ file (for Linux user).
  ```sh
     <andriod_studio_directory>/bin $ ./studio.sh
  ```
  3. From the file menu open the folder of cloned Android application source.	Launch Andriod studio by executing ‘studio.sh’ file (for Linux user).

  4. After opening the folder Face Recognition on CM2290 with SNPE on Andriod in Android studio select the SDK Manager icon in that select SDK Tools and check the Show Package Details.

  5. In the NDK section select version 17.2 version and CMake of 3.18.1 and click on apply. This will download the Android NDK and CMake required by SNPE SDK.

  6. Go to ‘CMakeLists.txt’ file and make the following changes for setting up OPENCV_DIR and include directories of SNPE SDK.

     - set(OpenCV_DIR "<opencv_directory>/opencv-4.5.4-
android-sdk/OpenCV-android-sdk/sdk/native/jni").

     - include_directories(<SNPE_ROOT>/include/zdl)

  
## Steps to Run Face Recognition on CM2290 with SNPE on Andriod Application:

Before running the application set the board environment by following below steps:

1. Connect kit to the host system, enter following adb commands
```sh
   $ adb disable-verity
   $ adb reboot
   $ adb root
   $ adb remount
```   

2. Turn on Wifi in android kit. Select the available Wifi by entering password.

3. Set the latest UTC time and date in setting->Date and time setting.

4. Copy the Library files of snpe 1.51 on to the kit from host system.
```sh
   $ adb push <SNPE_ROOT>/lib/arm-android-clang6.0/ /system/lib/
```
5. Copy the DSP files of snpe 1.51 on to the kit from host system.
```sh
   $ adb push <SNPE_ROOT>/lib/dsp/ /system/vendor/lib/rfsa/adsp/
```
6. Copy the libcdsprpc.so file from “/vendor/lib” folder to “/system/lib” folder
```sh
   $ adb shell 
   /# cp /vendor/lib/libcdsprpc.so ./system/lib/
```
7. Pushing the required database files to internal storage of device.
```sh
  - /# cd storage/emulated/0/   
    /# mkdir database
    /# exit

  - $ adb shell
    $ adb push Face Recognition on CM2290 with SNPE on Andriod/database/images/ /storage/emulated/0/database/   

  - $ adb push Face Recognition on CM2290 with SNPE on Andriod/database/models/ /storage/emulated/0/database/ 
```   

## Steps for running the main application are as follows:

1. Go to android studio and generate APK file for Face Recognition on CM2290 with SNPE on Andriod.

  - Once the build successful, go to Build menu and select Build
    Bundle(s)/APK(s) from the dropdown then select Build APK(s).

  - Above step will download APK file in the following directory.  
    Face Recognition on CM2290 with SNPE on Andriod/app/build/outputs/apk/debug/app-debug.apk

2. Install APK file on CM2290 kit.
```sh
   $ adb install app-debug.apk
```
3. Open the application on kit. It will open the live camera and Recognize the face by comparing with database images stored on kit.

4. To add a new person images to the database, create folder with person's name and add persons face crop images to the folder. Push that new folder inside database images.
```sh
   $ adb push <person_name>/ /storage/emulated/0/database/images/
```
