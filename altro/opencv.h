#ifndef __MIO_OPENCV_H__
#define __MIO_OPENCV_H__

#if CV_MAJOR_VERSION < 3
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#else
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#endif

#include <cstdint>
#include <limits>
#include <stdio.h>
#include "mio/altro/error.h"
#include "mio/altro/io.h"

namespace mio{

//gives the uchar pointer to a pixel's data.  the pointer can be indexed through to access each 
//channel at that pixel location.  CV_PIX_PTR(mat, row_idx, col_idx)[0 to mat.channels()-1]
#define CV_PIX_PTR(mat, row_idx, col_idx) (mat.data + mat.step[0]*row_idx + mat.step[1]*col_idx)


inline bool SameMat(const cv::Mat &a, const cv::Mat &b){
  //can also check that the reference counter is the same value for both and greater than 1?
  return (a.size() == b.size() && a.type() == b.type() && a.data == b.data);
}


//given an image (img) and the maximum allowable dimension (max_dim),
//the function returns true if one of the image's dimensions exceeds max_dim.
//when true, max_img_size and max_dim_scale_factor are set.
inline bool GetMaxSize(const cv::Mat &img, const size_t max_dim, cv::Size &max_img_size, float &max_dim_scale_factor){
  max_dim_scale_factor = 1.0f;
  if(img.cols > max_dim || img.rows > max_dim){
    max_dim_scale_factor = static_cast<float>(max_dim) / static_cast<float>( std::max(img.rows, img.cols) );
    max_img_size.height = std::lround(static_cast<float>(img.rows) * max_dim_scale_factor);
    if(max_img_size.height > max_dim)
      max_img_size.height = max_dim;
    max_img_size.width = std::lround(static_cast<float>(img.cols) * max_dim_scale_factor);
    if(max_img_size.width > max_dim)
      max_img_size.width = max_dim;
    return true;
  }
  max_img_size = img.size();
  return false;
}

inline bool GetMaxSize(const cv::Mat &img, const size_t max_dim, cv::Size &max_img_size){
  float max_dim_scale_factor;
  return GetMaxSize(img, max_dim, max_img_size, max_dim_scale_factor);
}


inline void LimitSize(const cv::Mat &src, cv::Mat &dst, const size_t max_dim){
  STD_INVALID_ARG_E( !src.empty() )
  cv::Size max_img_size = src.size();
  if( mio::GetMaxSize(src, max_dim, max_img_size) )
    cv::resize(src, dst, max_img_size, 0, 0, cv::INTER_LINEAR);
  else if( !SameMat(src, dst) )
    dst = src;
}


template <typename T>
inline T *GetDataPtr(const cv::Mat &mat){
  void *void_ptr = static_cast<void*>(mat.data);
  return static_cast<T*>(void_ptr);
}


//TODO - see "Formatted output of cv::Mat" in opencv2/core.hpp
inline void Print(const cv::Mat &m, const char *m_name = NULL){
    std::cout << "\n" << (m_name ? std::string(m_name) : "cv_mat") << ":\n" << m << "\n\n";
}


inline std::string SizeStr(const cv::Size &size){
  return (std::string("(rows: ") + std::to_string(size.height) + ", cols: " + std::to_string(size.width) + ")");
}

inline std::string SizeStr(const cv::Mat &mat){
  return SizeStr(mat.size());
}


inline std::string DepthStr(const int depth){
  switch(depth){
    case CV_8U:
      return std::string("CV_8U");
    case CV_8S:
      return std::string("CV_8S");
    case CV_16U:
      return std::string("CV_16U");
    case CV_16S:
      return std::string("CV_16S");
    case CV_32S:
      return std::string("CV_32S");
    case CV_32F:
      return std::string("CV_32F");
    case CV_64F:
      return std::string("CV_64F");
    default:
      return (std::string("invalid depth: ") + std::to_string(depth));
  }
}

inline std::string DepthStr(const cv::Mat &mat){
  return DepthStr(mat.depth());
}


/*
Array type. Use CV_8UC1, ..., CV_64FC4 to create 1-4 channel matrices, or CV_8UC(n), ..., CV_64FC(n) to 
create multi-channel (up to CV_MAX_CN channels) matrices.

elemSize() is the number of channels multiplied by the data size
elemSize1() is the data size
*/
inline void PrintMatProp(const cv::Mat &mat, const char *mat_name = NULL){
  if(mat_name)
    printf("cv::Mat %s\n", mat_name);
  printf("elemSize: %lu\n", mat.elemSize()); //bytes per pixel, eg. 8-bit rgb image returns 3
  printf("elemSize1: %lu\n", mat.elemSize1()); //bytes per pixel (only in first channel), eg. 8-bit rgb image returns 1
  printf("size: %s\n", SizeStr(mat).c_str());
  printf("step1: %lu\n", mat.step1()); //matrix step divided by elemSize1()
  printf("CvMat type: %d\n", mat.type()); //an identifier compatible with the CvMat type system
  printf("channels: %d\n", mat.channels());
  printf("depth: %s\n\n", DepthStr(mat).c_str());
}


template<typename T>
inline double GetPixelAvg(const cv::Mat &mat, const cv::Point &pnt){
  uint8_t *ptr = CV_PIX_PTR(mat, pnt.y, pnt.x);
  double total = 0;
  size_t num_channels = mat.channels();
  for(size_t i = 0; i < num_channels; ++i)
    total += ptr[i];
  return( total / static_cast<double>(num_channels) );
}


inline double GetPixelValue(const cv::Mat &mat, const cv::Point &pnt){
  if(mat.channels() == 1){
    switch( mat.depth() ){
      case CV_8U:
        return mat.at<uint8_t>(pnt);
      case CV_8S:
        return mat.at<int8_t>(pnt);
      case CV_16U:
        return mat.at<uint16_t>(pnt);
      case CV_16S:
        return mat.at<int16_t>(pnt);
      case CV_32S:
        return mat.at<int32_t>(pnt);
      case CV_32F:
        return mat.at<float>(pnt);
      case CV_64F:
        return mat.at<double>(pnt);
      default:
        printf( "GetPixelValue() - invalid depth: %d\n", mat.depth() );
        break;
    }
  }
  else{
    switch( mat.depth() ){
      case CV_8U:
        return GetPixelAvg<uint8_t>(mat, pnt);
      case CV_8S:
        return GetPixelAvg<int8_t>(mat, pnt);
      case CV_16U:
        return GetPixelAvg<uint16_t>(mat, pnt);
      case CV_16S:
        return GetPixelAvg<int16_t>(mat, pnt);
      case CV_32S:
        return GetPixelAvg<int32_t>(mat, pnt);
      case CV_32F:
        return GetPixelAvg<float>(mat, pnt);
      case CV_64F:
        return GetPixelAvg<double>(mat, pnt);
      default:
        printf( "GetPixelValue() - invalid depth: %d\n", mat.depth() );
        break;
    }
  }

  return 0;
}


//convenience function for computing a seperate histogram for each channel of an image
inline void CalcHist(const std::vector<cv::Mat> &src_img_vec, 
                     const int histogram_size, //number of accumulation bins
                     const float range_low, //inclusive lower pixel value in histogram range
                     const float range_high, //exclusive upper pixel value in histogram range
                     std::vector<cv::Mat> &hist_vec,
                     const cv::Mat &mask){
  const unsigned int num_src_img = src_img_vec.size();
  hist_vec.resize(num_src_img);

  const int num_image = 1;
  int *channels = NULL;
  hist_vec.resize(num_src_img);
  const int histogram_dim = 1; //postive histogram dimensionality
  float temp_dims[] = {range_low, range_high};
  const float *dimension_ranges[] = {temp_dims};
  const bool uniform_bin_size_flag = true,
             accumulate_flag = false;
  for(int i = 0; i < num_src_img; ++i)
    cv::calcHist(&src_img_vec[i], num_image, channels, mask, hist_vec[i], histogram_dim, &histogram_size, 
                 dimension_ranges, uniform_bin_size_flag, accumulate_flag);
}

inline void CalcHist(const cv::Mat &src, 
                     const int histogram_size,
                     const float range_low,
                     const float range_high,
                     std::vector<cv::Mat> &hist_vec,
                     const cv::Mat &mask){
  std::vector<cv::Mat> split_channels_vec;
  cv::split(src, split_channels_vec);

  CalcHist(split_channels_vec, histogram_size, range_low, range_high, hist_vec, mask);
}

template <typename T>
inline void CalcHistBinCoordinates(const size_t num_bin, const T range_low,
                                   const T range_high, std::vector<T> &bin_coord){
  bin_coord.resize(num_bin);
  const double bin_width = (range_high - range_low) / static_cast<double>(num_bin);
  for(size_t i = 0; i < num_bin; ++i)
    bin_coord[i] = std::round( bin_width * static_cast<double>(i) );
}


//saves channeled cv::Mat's
//TODO - add a unique file header
inline int SaveOpenCVMat(const std::string file_full, const cv::Mat &mat, bool print_flag = false){
  if(print_flag)
    mio::PrintMatProp(mat);
  FILE *fd;

  EXP_CHK(mat.isContinuous(), return(-1))
  EXP_CHK_ERRNO(fd = fopen(file_full.c_str(), "wb"), return(-1))
  const int rows = mat.rows,
            cols = mat.cols,
            type = mat.type();
  EXP_CHK_ERRNO(fwrite(&rows, sizeof(int), 1, fd) == 1, return(-1))
  EXP_CHK_ERRNO(fwrite(&cols, sizeof(int), 1, fd) == 1, return(-1))
  EXP_CHK_ERRNO(fwrite(&type, sizeof(int), 1, fd) == 1, return(-1))

  const size_t data_length = rows * cols * mat.channels(),
               data_size = mat.elemSize1();
  const void *mat_data = static_cast<void*>(mat.data);
  EXP_CHK_ERRNO(fwrite(mat_data, data_size, data_length, fd) == data_length, return(-1))

  fclose(fd);
  return 0;
}


//reads channeled cv::Mat's
//TODO - add a unique file header
inline int ReadOpenCVMat(const std::string file_full, cv::Mat &mat, bool print_flag = false){
  int rows, cols, type;
  FILE *fd;

  EXP_CHK_ERRNO(fd = fopen(file_full.c_str(), "rb"), return(-1))
  EXP_CHK_ERRNO(fread(&rows, sizeof(int), 1, fd) == 1, return(-1))
  EXP_CHK_ERRNO(fread(&cols, sizeof(int), 1, fd) == 1, return(-1))
  EXP_CHK_ERRNO(fread(&type, sizeof(int), 1, fd) == 1, return(-1))
  mat = cv::Mat(rows, cols, type);

  const size_t data_length = rows * cols * mat.channels(),
               data_size = mat.elemSize1();
  void *mat_data = static_cast<void*>(mat.data);
  EXP_CHK_ERRNO(fread(mat_data, data_size, data_length, fd) == data_length, return(-1))
  if(print_flag)
    mio::PrintMatProp(mat);

  fclose(fd);
  return 0;
}


inline cv::Mat ReadImage(const std::string file_full, const int flags){
  cv::Mat mat;
  if( FileExists(file_full) ){
    std::string file_ext;
    FileNameExpand(file_full, ".", NULL, NULL, NULL, &file_ext);
    if(file_ext == "mat")
      mio::ReadOpenCVMat(file_full, mat);
    else
      mat = cv::imread(file_full, flags);
  }

  return mat;
}


inline void SolveSVD(const cv::Mat &A, const cv::Mat &b, cv::Mat &x){
  cv::SVD svd(A);
  const size_t rank = cv::countNonZero(svd.w); //rank = number of non-zero diagonal terms of sigma
  //svd.w is a single column prepresenting the diagonal of an otherwise square sigma matrix
  cv::Mat sigma_plus = cv::Mat::zeros( svd.w.rows, svd.w.rows, svd.w.depth() );
#define COMPUTE_SIGMA_PLUS(data_type)\
  for(size_t i = 0; i < svd.w.rows; ++i){\
    const data_type &val_sigma = svd.w.at<data_type>(i);\
    if(i < rank && val_sigma != 0)\
      sigma_plus.at<data_type>(i, i) = 1.0 / val_sigma;\
  }
  switch( svd.w.depth() ){
    case CV_32F:
      COMPUTE_SIGMA_PLUS(float)
      break;
    case CV_64F:
      COMPUTE_SIGMA_PLUS(double)
      break;
    default:
      std::cout << "ComputeSigmaPlus() - error, unsupprted matrix depth\n";
      break;
  }
#undef COMPUTE_SIGMA_PLUS

  //opencv supplies v^t so do (v^t)^t to get v
  cv::Mat A_psuedo_inverse = svd.vt.t() * sigma_plus * svd.u.t();
  x = A_psuedo_inverse * b;
}


inline double OpenCVDepthMin(const int depth){
  switch(depth){
    case CV_8U:
      return std::numeric_limits<uint8_t>::min();
    case CV_8S:
      return std::numeric_limits<int8_t>::min();
    case CV_16U:
      return std::numeric_limits<uint16_t>::min();
    case CV_16S:
      return std::numeric_limits<int16_t>::min();
    case CV_32S:
      return std::numeric_limits<int32_t>::min();
    case CV_32F:
      return std::numeric_limits<float>::min();
    case CV_64F:
      return std::numeric_limits<double>::min();
    default:
      STD_INVALID_ARG_M("invalid cv::Mat depth")
  }
}

inline double OpenCVDepthMax(const int depth){
  switch(depth){
    case CV_8U:
      return std::numeric_limits<uint8_t>::max();
    case CV_8S:
      return std::numeric_limits<int8_t>::max();
    case CV_16U:
      return std::numeric_limits<uint16_t>::max();
    case CV_16S:
      return std::numeric_limits<int16_t>::max();
    case CV_32S:
      return std::numeric_limits<int32_t>::max();
    case CV_32F:
      return std::numeric_limits<float>::max();
    case CV_64F:
      return std::numeric_limits<double>::max();
    default:
      STD_INVALID_ARG_M("invalid cv::Mat depth")
  }
}

inline bool OpenCVInDepth(const int depth, const double &value){
  return ( value >= mio::OpenCVDepthMin(depth) && value <= mio::OpenCVDepthMax(depth) );
}


inline void UnsharpMask(const cv::Mat &src_img_in, cv::Mat &dst_img_out, const cv::Size &blur_kernel_dim = cv::Size(7, 7),
                        const float src_coef = 2.0, const float blur_coef = 1.0){
  cv::Mat src_img, blur_img, temp;
  const double sigmaX = 0, sigmaY = 0;

  const int orig_depth = src_img_in.depth();
  if(orig_depth == CV_32F || orig_depth == CV_64F)
    src_img = mio::SameMat(src_img_in, dst_img_out) ? src_img_in.clone() : src_img_in;
  else
    src_img_in.convertTo(src_img, CV_32F);

  cv::GaussianBlur(src_img, blur_img, blur_kernel_dim, sigmaX, sigmaY, cv::BORDER_DEFAULT);
  temp = (src_coef * src_img) - (blur_coef * blur_img);
  temp.convertTo(dst_img_out, orig_depth);
}

inline void ShowRoi(cv::Mat &img, const cv::Mat &roi_mask){
  if(img.empty() || roi_mask.empty())
    return;
  cv::Mat tmp_img(img.size(), img.type());
  img.copyTo(tmp_img, roi_mask);
  img *= 0.4;
  tmp_img.copyTo(img, roi_mask);
}


//the input cv::Mat name is src
#define CV_FUNC_TEMPLATE(func_name, template_func_name, func_param_default, func_param)\
inline void func_name func_param_default{\
  switch( src.depth() ){\
    case CV_8U:\
      template_func_name<uint8_t>func_param;\
      break;\
    case CV_8S:\
      template_func_name<int8_t>func_param;\
      break;\
    case CV_16U:\
      template_func_name<uint16_t>func_param;\
      break;\
    case CV_16S:\
      template_func_name<int16_t>func_param;\
      break;\
    case CV_32F:\
      template_func_name<float>func_param;\
      break;\
    case CV_64F:\
      template_func_name<double>func_param;\
      break;\
    default:\
      STD_INVALID_ARG_M("invalid cv::Mat depth")\
  }\
}


//dst[i] = max_val if src[i] > thresh, otherwise dst[i] = src[i]
template<typename T>
inline void ThreshTemplate(const cv::Mat &src, cv::Mat &dst, const double thresh, const double max_val){
  STD_INVALID_ARG_E( !SameMat(src, dst) )
  STD_INVALID_ARG_E( src.channels() == 1 && !src.empty() && src.isContinuous() )
  T *src_data = mio::GetDataPtr<T>(src);
  if( src.size() != dst.size() || src.type() != dst.type() )
    dst.create( src.size(), src.type() );
  T *dst_data = mio::GetDataPtr<T>(dst);
  const size_t data_len = src.cols * src.rows;
  for(size_t i = 0; i < data_len; ++i)
    dst_data[i] = (src_data[i] > thresh) ? max_val : src_data[i];
}

CV_FUNC_TEMPLATE( Thresh, ThreshTemplate,
                  (const cv::Mat &src, cv::Mat &dst, const double thresh, const double max_val),
                  (src, dst, thresh, max_val) )


//if data_in_degrees is true, the function converts the data to radian measurements, then computes the cosine
template<typename T>
inline void CosineTemplate(const cv::Mat &src, cv::Mat &dst, const bool data_in_degrees){
  if( dst.empty() || dst.size() != src.size() || dst.type() != src.type() )
    dst = cv::Mat( src.size(), src.type() );

  T *src_data = mio::GetDataPtr<T>(src); //TODO - check if mat is continuous
  T *dst_data = mio::GetDataPtr<T>(dst);
  const size_t data_len = src.cols * src.rows;
  if(data_in_degrees)
    for(size_t i = 0; i < data_len; ++i)
      dst_data[i] = cos(src_data[i] * 0.0174532925);
  else
    for(size_t i = 0; i < data_len; ++i)
      dst_data[i] = cos(src_data[i]);
}

CV_FUNC_TEMPLATE( Cosine, CosineTemplate,
                  (const cv::Mat &src, cv::Mat &dst, const bool data_in_degrees = false),
                  (src, dst, data_in_degrees) )


template<typename T>
inline void StatOutRemTemplate(cv::Mat &src, const double std_dev_coef, const double *set_val = nullptr,
                               cv::Scalar *mean = nullptr, cv::Scalar *std_dev = nullptr, cv::Mat *mask = nullptr){
  STD_INVALID_ARG_E( src.channels() == 1 && !src.empty() && src.isContinuous() )
  cv::Scalar mean_, std_dev_;
  if(mean != nullptr && std_dev != nullptr){
    mean_ = *mean;
    std_dev_ = *std_dev;
  }
  else
    cv::meanStdDev(src, mean_, std_dev_);

  //a value beyond or below these throesholds (respectively) is considered an outlier
  const double thresh_offset = std::abs( std_dev_coef * std_dev_.val[0] );
  const double thresh_upper_ = mean_.val[0] + thresh_offset,
               thresh_lower_ = mean_.val[0] - thresh_offset,
               type_max = std::numeric_limits<T>::max(),
               type_min = std::numeric_limits<T>::min();
  const double thresh_upper = thresh_upper_ <= type_max ? thresh_upper_ : type_max,
               thresh_lower = thresh_lower_ >= type_min ? thresh_lower_ : type_min;

  const T set_val_ = (set_val == nullptr) ? mean_.val[0] : *set_val;
  T *data = reinterpret_cast<T*>(src.data);
  if(mask != nullptr){
    *mask = cv::Mat::zeros(src.size(), CV_8U);
    auto *mask_data = mask->data;
    for(size_t i = 0, data_len = src.cols * src.rows; i < data_len; ++i)
      if(data[i] > thresh_upper || data[i] < thresh_lower){
        data[i] = set_val_;
        mask_data[i] = 255;
      }
  }
  else
    for(size_t i = 0, data_len = src.cols * src.rows; i < data_len; ++i)
      if(data[i] > thresh_upper || data[i] < thresh_lower)
        data[i] = set_val_;
}

//the mask pixels are set to 255 for each pixel in src that was "removed", otherwise 0
CV_FUNC_TEMPLATE( StatOutRem, StatOutRemTemplate,
                  (cv::Mat &src, const double std_dev_coef, const double *set_val = nullptr,
                   cv::Scalar *mean = nullptr, cv::Scalar *std_dev = nullptr, cv::Mat *mask = nullptr),
                  (src, std_dev_coef, set_val, mean, std_dev, mask) )


#define BIT_SHIFT(template_function_name, function_name, function_name_str, operator)\
template<typename T>\
inline void template_function_name(cv::Mat &src, const int shift){\
  /*CV_Assert( src.type() == CV_MAKETYPE(DataType<T>::depth, 4) );*/\
  int rows = src.rows;\
  int cols = src.cols * src.channels();\
\
  if( src.isContinuous() ){\
    cols *= rows;\
    rows = 1;\
  }\
\
  int i, j;\
  T *ptr;\
  for(i = 0; i < rows; ++i){\
    ptr = src.ptr<T>(i);\
    for(j = 0; j < cols; ++j){/*TODO - make this whole thing a macro and just insert the computation part*/\
      ptr[j] operator shift;\
    }\
  }\
}\
\
inline void function_name(cv::Mat &src, const int shift){\
  switch( src.depth() ){\
    case CV_8U:\
      template_function_name<unsigned char>(src, shift);\
      break;\
    case CV_16U:\
      template_function_name<unsigned short>(src, shift);\
      break;\
    default:\
      printf(function_name_str"() - bit depth not supported\n");\
      break;\
  }\
}

BIT_SHIFT(BitShiftRightTemplate, BitShiftRight, "BitShiftRight", >>=)
BIT_SHIFT(BitShiftLeftTemplate, BitShiftLeft, "BitShiftLeft", <<=)


} //namespace mio

#endif //__MIO_OPENCV_H__

