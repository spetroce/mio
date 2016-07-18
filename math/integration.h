#ifndef __MIO_INTEGRATION_H__
#define __MIO_INTEGRATION_H__

#include <cstdlib>
#include <functional>
#include <vector>

#if ICV_OPENCV_VERSION_MAJOR < 3
#include "opencv2/core/core.hpp"
#else
#include "opencv2/core.hpp"
#endif


enum IntegrationMethod{
  RECT_L = 0,
  RECT_M,
  RECT_R,
  TRAPZ,
  SIMPSON
};

double Integrate(const double a, const double b, const size_t num_step, 
                 std::function<double(double)> func, const IntegrationMethod int_type);

double Integrate(const std::vector<double> &x, const std::vector<double> &y, const IntegrationMethod int_type);

double Integrate(const cv::Mat &x, const cv::Mat &y, const IntegrationMethod int_type);

//#define DISCRETE_NUM_INT(data_array) [&](double x) -> double {return data_array[static_cast<size_t>(x)];}

#endif //__MIO_INTEGRATION_H__
