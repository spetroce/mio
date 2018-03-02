#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include "opencv2/core.hpp"
#include "mio/math/integration.h"

typedef double (*func_t)(double);


/* test */
double f0(double x){
  return sin(x);
}

double f0a(double x){
  return -cos(x);
}


double f1(double x){ //function to integrate
  return x*x*x;
}
 
double f1a(double x){ //evaluated integral of function f1
  return x*x*x*x/4.0;
}


double f2(double x){
  return 1.0/x;
}
 
double f2a(double x){
  return log(x);
}


double f3(double x){
  return x;
}
 
double f3a(double x){
  return x*x/2.0;
}


int main(){
  std::vector<IntegrationMethod> int_method_vec {RECT_L, RECT_M, RECT_R, TRAPZ, SIMPSON};
  std::vector<std::string> int_func_names {"leftrect", "midrect", "rightrect", "trapezium", "simpson"};
  std::vector<func_t> func_vec {f0, f1,  f2,  f3,  f3};
  std::vector<func_t> eval_func_vec {f0a, f1a, f2a, f3a, f3a};

  struct SIntRange { double a, b; SIntRange(double a_, double b_) : a(a_), b(b_) {} };
  const SIntRange intRange[] = { SIntRange(0.0, M_PI),
                                 SIntRange(0.0, 1.0),
                                 SIntRange(1.0, 100.0),
                                 SIntRange(0.0, 5000.0),
                                 SIntRange(0.0, 6000.0) };
  const double approx[] = {1000, 100.0, 1000.0, 5000000.0, 6000000.0};

  for(int j = 0, i = 0; j < func_vec.size(); ++j, i = 0){
    const double a = intRange[j].a,
                 b = intRange[j].b;
    printf( "integral of function at index [%d] = %lf\n", j, (*eval_func_vec[j])(b) - (*eval_func_vec[j])(a) );
    for(auto int_method : int_method_vec){
      double ic = Integrate(a, b, approx[j], func_vec[j], int_method);
      printf("%10s integration: %+lf\n", int_func_names[i++].c_str(), ic);
    }
    printf("\n");
  }

  //test array integrators
  std::vector<double> x, y;
  const size_t func_idx = 4;
  const size_t num_step = approx[func_idx];
  const double step_size = (intRange[func_idx].b - intRange[func_idx].a) / static_cast<double>(num_step);
  const double stop = intRange[func_idx].b - step_size;
  double sum = intRange[func_idx].a;
  x.reserve(num_step + 1);
  y.reserve(num_step + 1);
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_real_distribution<double> dist(step_size * 0.5, step_size * 1.5);
  for(int i = 0; sum < stop; ++i){
    x.push_back(sum);
    y.push_back( (*func_vec[func_idx])( x.back() ) );
    sum += dist(gen);
  }
  x.push_back(intRange[func_idx].b);
  y.push_back( (*func_vec[func_idx])( x.back() ) );

  printf( "size: %d, front %f, front+1 %f, back-1 %f, back %f\n", x.size(), x[0], x[1], x[x.size()-2], x[x.size()-1] );

  cv::Mat x_mat(x, false), y_mat(y, false);
  printf( "Array integration value for function [%d]: %+lf\n", func_idx, Integrate(x_mat, y_mat, TRAPZ) );

}

