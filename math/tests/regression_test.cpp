#include "mio/math/regression.hpp"
#include "mio/altro/error.hpp"


void PolynomialTest(){
  std::vector<double> y_data, x_data, coeff_in = {7.3, 2.65, 1.5, 1.15}, coeff_out;
  for(int x = -25; x < 25; ++x){
    x_data.push_back(x);
    y_data.push_back(coeff_in[0] + coeff_in[1]*x + coeff_in[2]*x*x + coeff_in[3]*x*x*x);
  }
  mio::ComputePoly( y_data, x_data, 3, coeff_out, cv::Mat() );
  STD_INVALID_ARG_E( coeff_in.size() == coeff_out.size() )
  printf("actual coeff: %.15f, %.15f, %.15f, %.15f\n", coeff_in[0], coeff_in[1], coeff_in[2], coeff_in[3]);
  printf("estim. coeff: %.15f, %.15f, %.15f, %.15f\n", coeff_out[0], coeff_out[1], coeff_out[2], coeff_out[3]);
}


void PowerFuncTest(){
  std::vector<double> y_data, x_data, coeff_in = {14.715, 1.625}, coeff_out;
  for(int x = -25; x < 25; ++x){
    x_data.push_back(x);
    y_data.push_back( coeff_in[0] * std::pow(x, coeff_in[1]) );
  }
  mio::ComputePowerFunc(y_data, x_data, coeff_out);
  STD_INVALID_ARG_E( coeff_in.size() == coeff_out.size() )
  printf("actual coeff: %.15f, %.15f\n", coeff_in[0], coeff_in[1]);
  printf("estim. coeff: %.15f, %.15f\n", coeff_out[0], coeff_out[1]);
}


int main(int argc, char *argv[]){
  printf("Testing Polynomial Regression...\n");
  PolynomialTest();
  printf("\nTesting Power Function Regression...\n");
  PowerFuncTest();

  return 0;
}

