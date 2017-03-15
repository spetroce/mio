#ifndef __CV_MAT_TO_QIMAGE_H__
#define __CV_MAT_TO_QIMAGE_H__

#include <QImage>
#if CV_MAJOR_VERSION < 3
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#else
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#endif
#include "mio/altro/error.h"

static QVector<QRgb> OpenCVMat2QImageColorTable;


//only initialize the color table once, used to translate colour indexes to qRgb values
inline void SetupMat2QImage(){
  if(OpenCVMat2QImageColorTable.size() == 0)
    for(int i = 0; i < 256; i++)
      OpenCVMat2QImageColorTable.push_back( qRgb(i, i, i) );
}


//returns true if the image 'has 1 or 3 channels' and 'has an 8U, 32F, or 64F depth'
inline bool OpenCVMat2QImage(const cv::Mat &src_img, QImage &dst_img, const bool copy_flag){
  const int channels = src_img.channels(),
            depth = src_img.depth();
  EXP_CHK( channels == 1 || channels == 3, return(false) )
  EXP_CHK( depth == CV_8U || depth == CV_32F || depth == CV_64F, return(false) )

  cv::Mat img;
  if(depth == CV_8U)
    img = src_img;
  else
    src_img.convertTo(img, CV_8U, 255);

  if(channels == 1){
    const auto img_data = img.data;
    if(copy_flag)
      dst_img = QImage(img_data, img.cols, img.rows, img.step, QImage::Format_Indexed8).copy();
    else
      dst_img = QImage(img_data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
    dst_img.setColorTable(OpenCVMat2QImageColorTable);
  }
  else{
    cv::Mat temp;
    cv::cvtColor(img, temp, cv::COLOR_BGR2RGB);
    const auto temp_data = temp.data;
    if(copy_flag)
      dst_img = QImage(temp_data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888).copy();
    else
      dst_img = QImage(temp_data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
  }

  return true;
}

#endif //__CV_MAT_TO_QIMAGE_H__

