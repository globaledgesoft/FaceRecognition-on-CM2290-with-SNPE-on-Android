
#ifndef FACERECOGNITION_INPUT_IMAGE_H
#define FACERECOGNITION_INPUT_IMAGE_H

#include "Util.h"
#include <media/NdkImageReader.h>
#include <opencv2/core.hpp>

class Input_Image {
 public:
  explicit Input_Image(ImageFormat* res, enum AIMAGE_FORMATS format);

  ~Input_Image();


  ANativeWindow* GetNativeWindow(void);

  AImage* GetNextImage(void);

  AImage* GetLatestImage(void);

  int32_t GetMaxImage(void);


  void DeleteImage(AImage* image);


  void ImageCallback(AImageReader* reader);


  bool DisplayImage(ANativeWindow_Buffer* buf, AImage* image);


  void SetPresentRotation(int32_t angle);

 private:
  int32_t presentRotation_;
  AImageReader* reader_;

  void PresentImage(ANativeWindow_Buffer* buf, AImage* image);
  void PresentImage90(ANativeWindow_Buffer* buf, AImage* image);
  void PresentImage180(ANativeWindow_Buffer* buf, AImage* image);
  void PresentImage270(ANativeWindow_Buffer* buf, AImage* image);

  int32_t imageHeight_;
  int32_t imageWidth_;

  uint8_t* imageBuffer_;

  int32_t yStride, uvStride;
  uint8_t *yPixel, *uPixel, *vPixel;
  int32_t yLen, uLen, vLen;
  int32_t uvPixelStride;
};

#endif  // OPENCV_NDK_IMAGE_READER_H
