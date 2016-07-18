#ifndef __MIO_MATH_REGRESSION_H__
#define __MIO_MATH_REGRESSION_H__

#include <algorithm>

#include "opencv2/imgproc.hpp"
#include "mio/altro/error.h"
#include "mio/altro/casting.h"
#include "mio/math/math.h"
#include "mio/altro/types.h"


namespace mio{

/*** START OLD POLYNOMIAL REGRESSION ***/

inline void GetDesignMatrix(const cv::InputArray _x_data, const size_t poly_degree, cv::Mat &A){
  const cv::Mat x_data = _x_data.getMat();
  STD_INVALID_ARG_E(x_data.cols > poly_degree && x_data.rows == 1 || x_data.rows > poly_degree && x_data.cols == 1)

  const size_t num_poly_terms = 1 + poly_degree;
  const cv::Mat x_data_col = (x_data.rows == 1) ? x_data.t() : x_data; //make sure data is a column vector

  A = cv::Mat( x_data_col.rows, num_poly_terms, x_data_col.type() );
  A.col(0).setTo(1.0); //y = k_1
  if(poly_degree >= 1) //y = k_1 + k_2 * x
    x_data_col.copyTo( A.col(1) );
  for(size_t m = 2; m <= poly_degree; ++m) //y = k_1 + k_2 * x + ... + k_m+1 * x^m
    cv::pow( x_data_col, m, A.col(m) );
}

/*
n = number data samples
m = degree of polynomial (implementation requires this to be greater than zero)
y = k_0 + (k_1 * x) + (k_2 * x^2) + ... + (k_m * x^m) ...
note: the number of coefficients is m+1

NOTE: A is AKA the design matrix
A = [ 1   ,  x_0 ,  x_0^2 ,  x_0^2 ,  ... ,  x_0^m;
      1   ,  x_1 ,  x_1^2 ,  x_1^2 ,  ... ,  x_1^m;
      1   ,  x_2 ,  x_2^2 ,  x_2^2 ,  ... ,  x_2^m;
      ... ,  ... ,  ...   ,  ...   ,  ... ,  ...  ;
      1   ,  x_n ,  x_n^2 ,  x_n^2 ,  ... ,  x_n^m;]

b = [ y_0,
      y_1,
      y_2,
      ...,
      y_n ]

solve linear equation Ax = b
A - coefficient matrix of order n (AKA the design matrix)
x - unknown, or solution n-vector
b - right hand side n-vector

if A is supplied, it will be used. Otherwise, A will be computed from _x_data.
*/
inline void ComputePoly(const cv::InputArray _y_data, const cv::InputArray _x_data,
                        const size_t poly_degree, cv::OutputArray coeff, cv::Mat A){
  STD_INVALID_ARG_E(poly_degree > 0)
  STD_INVALID_ARG_EM( !_x_data.empty() || !A.empty(), "you must supply either _x_data or A")

  const cv::Mat y_data = _y_data.getMat();
  if( A.empty() ){
    const cv::Mat x_data = _x_data.getMat();
    STD_INVALID_ARG_E( y_data.type() == x_data.type() && (y_data.type() == CV_32FC1 || y_data.type() == CV_64FC1) )
    //for x_data and y_data, one dimension must be 1 and the other n, where n > poly_degree
    STD_INVALID_ARG_E(x_data.cols == y_data.cols && x_data.rows == y_data.rows ||
                      x_data.cols == y_data.rows && x_data.rows == y_data.cols)
    STD_INVALID_ARG_E(y_data.cols > poly_degree && y_data.rows == 1 || y_data.rows > poly_degree && y_data.cols == 1)

    GetDesignMatrix(_x_data, poly_degree, A);
  }
  else{
    STD_INVALID_ARG_E( y_data.type() == A.type() && (y_data.type() == CV_32FC1 || y_data.type() == CV_64FC1) )
    STD_INVALID_ARG_E(A.cols == poly_degree+1)
    STD_INVALID_ARG_E(y_data.rows == A.rows && y_data.cols == 1 || y_data.cols == A.rows && y_data.rows == 1)
  }

  cv::Mat x;
  const cv::Mat y_data_col = (y_data.rows == 1) ? y_data.t() : y_data; //make sure data is a column vector
  cv::solve(A, y_data_col, x, cv::DECOMP_SVD); //cv::solve(A, b, x, cv::DECOMP_SVD)
  x.copyTo(coeff);
}


template<typename T>
inline T SolvePoly(const cv::Mat &coeff, const T &x){
  STD_INVALID_ARG_E(coeff.rows > 0 && coeff.cols == 1 || coeff.cols > 0 && coeff.rows == 1)
  STD_INVALID_ARG_E(coeff.type() == CV_32FC1 || coeff.type() == CV_64FC1)

  #define SOLVE_POLY_MACRO(data_type)\
  {\
  total = coeff.at<data_type>(0);\
  for(size_t i = 1; i < num_coeff; ++i)\
    total += coeff.at<data_type>(i) * std::pow(x, i);\
  }\

  double total = -1;
  const size_t num_coeff = std::max(coeff.rows, coeff.cols);
  const int type = coeff.type();
  if(type == CV_32FC1)
    SOLVE_POLY_MACRO(float)
  else
    SOLVE_POLY_MACRO(double)

  #undef SOLVE_POLY_MACRO

  return total;
}


template<typename T>
inline T SolvePoly(const std::vector<T> &coeff, const T &x){
  STD_INVALID_ARG_E(coeff.size() > 0)

  double total = coeff[0];
  const size_t num_coeff = coeff.size();
  for(size_t i = 0; i < num_coeff; ++i)
    total += coeff[i] * std::pow(x, i);

  return total;
}

/*** END OLD POLYNOMIAL REGRESSION ***/


/*
Example usage:

y  =  k_0 + (k_1 * x) + (k_2 * x^2)  +  (k_3 * z) + (k_4 * z^2) + (k_5 * z^4)
const size_t num_sample = 15;
std::vector<double> y_data(num_sample), x_data(num_sample), coeff;

...fill y_data and x_data...

std::vector<int> x_expo = {1, 2},
                 z_expo = {1, 2, 4};
const size_t num_coeff = 1 + x_expo.size() + z_expo.size();
CMultiVarPoly multi_var_poly;
multi_var_poly.InitDesignMatrix(num_sample, 6, CV_64F);
multi_var_poly.InsertConstantCoeff();
multi_var_poly.InsertData(x_data, x_expo);
multi_var_poly.InsertData(z_data, x_expo);
multi_var_poly.Solve(y_data, coeff);
*/
class CMultiVarPoly{
  cv::Mat A;
  size_t m_num_coeff;
  size_t m_coeff_idx;

  public:
    CMultiVarPoly() : m_num_coeff(0), m_coeff_idx(0) {};

    void InitDesignMatrix(const size_t num_sample, const size_t num_coeff, const int cv_type){
      m_num_coeff = num_coeff;
      A.create(num_sample, num_coeff, cv_type);
    }

    void InsertConstantCoeff(){
      EXP_CHK_EM(m_coeff_idx > 0 && m_coeff_idx < A.cols, return, "probably exceeded the initialized number of coeff\n")
      A.col(m_coeff_idx).setTo(1.0);
      m_coeff_idx++;
    }

    //exponent_vec.size() == poly_degree for the variable being added to A
    void InsertData(const cv::InputOutputArray _x_data, const std::vector<int> &exponent_vec){
      const cv::Mat x_data = _x_data.getMat();
      STD_INVALID_ARG_E(x_data.cols > exponent_vec.size() && x_data.rows == 1 ||
                        x_data.rows > exponent_vec.size() && x_data.cols == 1)

      const cv::Mat x_data_col = (x_data.rows == 1) ? x_data.t() : x_data; //make sure data is a column vector

      for(auto &expo : exponent_vec){
        EXP_CHK_EM(m_coeff_idx > 0 && m_coeff_idx < A.cols, return, "probably exceeded the initialized number of coeff\n")
        cv::pow( x_data_col, expo, A.col(m_coeff_idx) );
        m_coeff_idx++;
      }
    }

    void Solve(const cv::InputOutputArray _y_data, cv::OutputArray coeff){
      const cv::Mat y_data = _y_data.getMat();
      STD_INVALID_ARG_E(y_data.cols == A.rows && y_data.rows == 1 ||
                        y_data.rows == A.rows && y_data.cols == 1)
      cv::Mat x;
      const cv::Mat y_data_col = (y_data.rows == 1) ? y_data.t() : y_data; //make sure data is a column vector
      cv::solve(A, y_data_col, x, cv::DECOMP_SVD); //cv::solve(A, b, x, cv::DECOMP_SVD)
      x.copyTo(coeff);
    }
};


/*
  estimate a and b in power function y = ax^b
  convert to linear equation so we can use a linear solver
  log(y) = log(ax^b)
  log(y) = log(a) + b*log(x)
  (ie. looks like y = mx + b)

  we take the log of both the x and y data, so we can't have negative data values.
  if check_values is true, values <= zero are sorted out before solving.
*/
inline void ComputePowerFunc(const cv::InputArray _y_data, const cv::InputArray _x_data,
                             cv::OutputArray coeff, const bool check_values = true){
  const cv::Mat y_data = _y_data.getMat(), x_data = _x_data.getMat();

  STD_INVALID_ARG_E( y_data.type() == x_data.type() && (y_data.type() == CV_32FC1 || y_data.type() == CV_64FC1) )
  //data vectors should be (1 x n) or (n x 1); i.o.w., one dimension must be 1 and the other n
  STD_INVALID_ARG_E(x_data.cols == y_data.cols && x_data.rows == y_data.rows ||
                    x_data.cols == y_data.rows && x_data.rows == y_data.cols)
  //the larger dimension must be greater than 2 and the other equal to 1
  STD_INVALID_ARG_E(y_data.cols > 2 && y_data.rows == 1 || y_data.rows > 2 && y_data.cols == 1)
  STD_INVALID_ARG_E(x_data.cols > 2 && x_data.rows == 1 || x_data.rows > 2 && x_data.cols == 1)

  //make sure data is a column vector
  cv::Mat y_data_col, x_data_col;
  if(check_values){
    const size_t data_size = std::max(y_data.rows, y_data.cols);
    y_data_col = cv::Mat( data_size, 1, y_data.type() );
    x_data_col = cv::Mat( data_size, 1, x_data.type() );
    size_t good_data_idx = 0;
#define COMPUTE_POWER_FUNC_DATA_CHECK(data_type)                                \
  {                                                                             \
    data_type *y_data_ptr = mio::StaticCastPtr<data_type>(y_data.data),         \
              *x_data_ptr = mio::StaticCastPtr<data_type>(x_data.data),         \
              *y_data_col_ptr = mio::StaticCastPtr<data_type>(y_data_col.data), \
              *x_data_col_ptr = mio::StaticCastPtr<data_type>(x_data_col.data); \
    for(size_t i = 0; i < data_size; ++i)                                       \
      if(y_data_ptr[i] > 0 && x_data_ptr[i] > 0){                               \
        y_data_col_ptr[good_data_idx] = y_data_ptr[i];                          \
        x_data_col_ptr[good_data_idx] = x_data_ptr[i];                          \
        good_data_idx++;                                                        \
      }                                                                         \
  }
    if(y_data.depth() == CV_32F)
      COMPUTE_POWER_FUNC_DATA_CHECK(float)
    else
      COMPUTE_POWER_FUNC_DATA_CHECK(double)
#undef COMPUTE_POWER_FUNC_DATA_CHECK
    y_data_col.resize(good_data_idx);
    x_data_col.resize(good_data_idx);
  }
  else{
    y_data_col = (y_data.rows == 1) ? y_data.t() : y_data;
    x_data_col = (x_data.rows == 1) ? x_data.t() : x_data;
  }

  cv::Mat A( y_data_col.rows, 2, y_data_col.type() ), x, b;
  A.col(0).setTo(1);
  cv::log( x_data_col, A.col(1) );

  cv::log(y_data_col, b);

  cv::solve(A, b, x, cv::DECOMP_SVD);
  //convert constant coefficients
  cv::Mat x_first_elem = x( cv::Range(0, 1), cv::Range(0, 1) );
  cv::exp(x_first_elem, x_first_elem);
  x.copyTo(coeff);
}


template <typename T>
inline T SolvePowerFunc(const cv::InputArray coeff_, const T &x){
  const cv::Mat coeff = coeff_.getMat();
  STD_INVALID_ARG_E(coeff.cols == 1 && coeff.rows == 2 || coeff.rows == 1 && coeff.cols == 2)
  const int coeff_type = coeff.type();
  STD_INVALID_ARG_E(coeff_type == CV_32FC1 || coeff_type == CV_64FC1)
  if(coeff_type == CV_32FC1)
    return coeff.at<float>(0) * std::pow( x, coeff.at<float>(1) );
  else
    return coeff.at<double>(0) * std::pow( x, coeff.at<double>(1) );
}


//NOTE: Do not forget to sort the data points so x_data goes from least to greatest
template <typename PNT_T, typename DATA_T>
inline PNT_T LinearInterpolationRegression2(const std::vector<PNT_T> &pnts, const DATA_T x){
  const size_t pnts_size = pnts.size();
  STD_INVALID_ARG_E(pnts_size > 1)
  CLine3f line_coef;

  auto cmp = [&](const DATA_T &x, const PNT_T &p){ return x < p.x; };
  const auto it = std::upper_bound(pnts.begin(), pnts.end(), x, cmp);
  if(it == pnts.begin())
    sm::LineCoefFromPnt2(pnts[0], pnts[1], line_coef);
  else if(it == pnts.end())
    sm::LineCoefFromPnt2(pnts[pnts_size-2], pnts[pnts_size-1], line_coef);
  else
    sm::LineCoefFromPnt2(*(it-1), *it, line_coef);

  return sm::SolveLineEqn2<PNT_T>(line_coef, x);
}

//NOTE: Do not forget to sort the data points so x_data goes from least to greatest
//template <typename PNT_T, typename DATA_T>
//inline PNT_T LinearInterpolationRegression2(const std::vector<PNT_T> &pnts, const DATA_T x){
//  STD_INVALID_ARG_E(pnts.size() > 1)
//  CLine3f line_coef;
//  const size_t pnts_size = pnts.size();
//  const size_t pnts_size_less_one = pnts_size-1;

//  // Check if x is less than pnts.front().x
//  if(x < pnts[0].x){
//    sm::LineCoefFromPnt2(pnts[0], pnts[1], line_coef);
//    return sm::SolveLineEqn2<PNT_T>(line_coef, x);
//  }

//  if(x == pnts[0].x)
//    return pnts[0];

//  // Find the two values that x is between
//  for(size_t i = 1; i < pnts_size; ++i){
//    if(x == pnts[i].x)
//      return pnts[i];
//    if(x < pnts[i].x){
//      sm::LineCoefFromPnt2(pnts[i-1], pnts[i], line_coef);
//      return sm::SolveLineEqn2<PNT_T>(line_coef, x);
//    }
//  }

//  // x is greater than pnts.back().x
//  sm::LineCoefFromPnt2(pnts[pnts_size-2], pnts[pnts_size_less_one], line_coef);
//  return sm::SolveLineEqn2<PNT_T>(line_coef, x);

//  const bool not_found = false;
//  assert(not_found); // This point should never be reached
//}

} //namespace mio

#endif //__MIO_MATH_REGRESSION_H__

