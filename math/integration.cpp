#include "mio/math/integration.h"
#include "mio/altro/error.h"
#include "mio/altro/opencv.h"
#include <cstdio>

//for the three rect functions, we accumulate x rather than compute it as Trapzium and Simpson.
//this is slightly less accurate, but more efficeint, which is acceptable given the already 
//inherently low accuracy of rectangluar integrations.

double RectLeft(const double a, const double b, const size_t num_step, std::function<double(double)> func){
  const double step_size = (b-a)/static_cast<double>(num_step);
  double sum = 0.0,
         x = a;

  for(size_t i = 0; i < num_step; ++i, x += step_size)
    sum += func(x);

  return step_size*sum;
}


double RectMid(const double a, const double b, const size_t num_step, std::function<double(double)> func){
  const double step_size = (b-a)/static_cast<double>(num_step);
  double sum = 0.0, 
         x = a + step_size*0.5;

  for(size_t i = 0; i < num_step; ++i, x += step_size)
    sum += func(x);

  return step_size*sum;
}


double RectRight(const double a, const double b, const size_t num_step, std::function<double(double)> func){
  const double step_size = (b - a) / static_cast<double>(num_step);
  double sum = 0.0,
         x = a + step_size;

  for(size_t i = 0; i < num_step; ++i, x += step_size)
    sum += func(x);

  return step_size*sum;
}

 
double Trapzium(const double a, const double b, const size_t num_step, std::function<double(double)> func){
  double step_size = (b - a) / static_cast<double>(num_step);
  double sum = func(a) + func(b);

  for(size_t i = 1; i < num_step; i++)
    sum += 2.0 * func(a + i*step_size);

  return 0.5 * step_size * sum;
}


double Trapzium(const double *x, const double *y, const size_t num_pnt){
  const size_t num_step = num_pnt - 1;
  double sum = 0.0;

  for(size_t i = 0; i < num_step; ++i)
    sum += (x[i+1] - x[i]) * (y[i] + y[i+1]); 

  return 0.5 * sum;
}


double Trapzium(const std::vector<double> &x, const std::vector<double> &y){
  STD_INVALID_ARG_E( x.size() == y.size() && 2 < x.size() )
  return Trapzium( x.data(), y.data(), x.size() );
}


double Trapzium(const cv::Mat &x, const cv::Mat &y){
  STD_INVALID_ARG_E( x.size() == y.size() && x.isContinuous() && y.isContinuous() && 
                     ( (x.rows == 1 && x.cols > 1) || (x.rows > 1 && x.cols == 1) ) )
  return Trapzium( mio::GetDataPtr<double>(x), mio::GetDataPtr<double>(y), std::max(x.rows, x.cols) );
}


double Simpson( const double a, const double b, const size_t num_step, std::function<double(double)> func){
  double step_size = (b - a) / static_cast<double>(num_step);

  double sum1 = func(a + step_size*0.5), sum2 = 0.0;
  for(size_t i = 1; i < num_step; i++){
    double i_ = static_cast<double>(i);
    sum1 += func(a + step_size*i_ + step_size*0.5);
    sum2 += func(a + step_size*i_);
  }

  return (func(a) + func(b) + 4.0*sum1 + 2.0*sum2) * step_size / 6.0;
}

/*
def simpson(f, a, b, n):
    """Approximates the definite integral of f from a to b by
    the composite Simpson's rule, using n subintervals"""
    h = (b - a) / n
    s = f(a) + f(b)
 
    for i in range(1, n, 2):
        s += 4 * f(a + i * h)
    for i in range(2, n-1, 2):
        s += 2 * f(a + i * h)
 
    return s * h / 3


def faster_simpson(f, a, b, steps):
   h = (b-a)/steps
   a1 = a+h/2
   s1 = sum( f(a1+i*h) for i in range(0,steps))
   s2 = sum( f(a+i*h) for i in range(1,steps))
   return (f(a) + f(b) + 4.0*s1 + 2.0*s2) * (h/6.0)



   for(i = 0;i < n;i++)
      s1 += func(a + h * i + h / 2.0);
 
   for(i = 1;i < n;i++)
      s2 += func(a + h * i);
 
   return (func(a) + func(b) + 4.0*s1 + 2.0*s2) * h/6.0;


function out = simpson(f,a,b,n)
  h = (b-a)/n;
  int = f(a);
  for i = 1:n-1
    if mod(i,2) == 0
      int = int+2*f(i*h);
    else
      int = int+4*f(i*h);
    end
  end

  int = int +f(b);
  int = int * h/3;
  out = int;
*/


/*
integrates on range a to b
range is divided into num_step pieces
*/
double Integrate(const double a, const double b, const size_t num_step, 
                 std::function<double(double)> func, const IntegrationMethod int_type){
  if(int_type == RECT_L)
    return RectLeft(a, b, num_step, func);
  else if(int_type == RECT_M)
    return RectMid(a, b, num_step, func);
  else if(int_type == RECT_R)
    return RectRight(a, b, num_step, func);
  else if(int_type == TRAPZ)
    return Trapzium(a, b, num_step, func);
  else if(int_type == SIMPSON)
    return Simpson(a, b, num_step, func);
  else
    STD_INVALID_ARG_M("invalid IntegrationMethod value")
}


double Integrate(const std::vector<double> &x, const std::vector<double> &y, const IntegrationMethod int_type){
  if(int_type == TRAPZ)
    return Trapzium(x, y);
  else
    STD_INVALID_ARG_M("invalid IntegrationMethod value")
}


double Integrate(const cv::Mat &x, const cv::Mat &y, const IntegrationMethod int_type){
  if(int_type == TRAPZ)
    return Trapzium(x, y);
  else
    STD_INVALID_ARG_M("invalid IntegrationMethod value")
}

