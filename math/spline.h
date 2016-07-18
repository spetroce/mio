#ifndef __MIO_SPLINE_H__
#define __MIO_SPLINE_H__
#include <vector>
#include <algorithm> //std::generate()
#include <cassert>


namespace sm{

//TODO/FIXME - this might be doubling up first and last coordinates in addition to CUBIC_SPLINE_FUNC()
template <typename T>
void MakeKnotPnts(const std::vector<T> &src, std::vector<T> &dst){
  assert(src.size() > 0);
  dst.resize(2*src.size() + 8);
  std::copy( src.end() - 4, src.end(), dst.begin() );
  std::reverse_copy(src.begin(), src.end(), dst.begin() + 4);
  std::copy( src.begin(), src.end(), dst.begin() + 4 + src.size() );
  std::reverse_copy(src.end() - 4, src.end(), dst.end() - 4);
}


enum SPLINE_TYPE{
  CUBIC_CATMULL_ROM = 0, // Interpolating spline (passes through all the knots)
  CUBIC_CARDINAL, // Interpolating spline
  CUBIC_B, // Approximating spline (doesn't necessarily pass through knots)
  CUBIC_BETA, // Approximating spline
};

typedef double matrix [16];


//Cubic Catmull-Rom Spline (aka Cardinal Spline)
void GetCubicCatmullRomSpline(matrix m){
  m[0] = -0.5;  m[1] =  1.5;  m[2] = -1.5;  m[3] =  0.5;
  m[4] =  1.0;  m[5] = -2.5;  m[6] =  2.0;  m[7] = -0.5;
  m[8] = -0.5;  m[9] =  0.0;  m[10] = 0.5;  m[11] = 0.0;
  m[12] = 0.0;  m[13] = 1.0;  m[14] = 0.0;  m[15] = 0.0;
}


//Cubic Cardinal Spline (generalization of the Catmull-Rom spline, with a tension parameter)
//a - tension parameter
//This is the same as a cubic Catmull-Rom spline when tension=0.5
void GetCubicCardinalMatrix(const double a, matrix m){
  m[0] = m[7] = m[8] = -a;
  m[1] = 2.-a;
  m[2] = a-2.;
  m[3] = m[10] = a;
  m[4] = 2.*a;
  m[5] = a-3.;
  m[6] = 3. - 2.*a;
  m[9] = m[11] = m[12] = m[14] = m[15] = 0.;
  m[13] = 1.;
}


//Classic Cubic B-Spline
void GetCubicBSpline(matrix m){
  const double k = 1.0/6.0;
  m[0] = -1.0*k;  m[1] =  3.0*k;  m[2] = -3.0*k;  m[3] =  1.0*k;
  m[4] = 3.0*k;   m[5] = -6.0*k;  m[6] =  3.0*k;  m[7] =  0.0;
  m[8] = -3.0*k;  m[9] =  0.0;    m[10] = 3.0*k;  m[11] = 0.0;
  m[12] = 1.0*k;  m[13] = 4.0*k;  m[14] = 1.0*k;  m[15] = 0.0;
}


//Cubic Beta-Spline (biased and tensioned Cubic B-Spline)
//b0 - bias parameter
//b1 - tension parameter
//This is the same (I think) as a cubic B spline when bias=1 and tension=0.5
void GetCubicBetaMatrix(const double b0, const double b1, matrix m){
  const double b2 = b0 * b0;
  const double b3 = b0 * b2;
  const double k = 1. / (b1 + 2.*b3 + 4.*b2 + 4.*b0 + 2.);

  m[0] = (-2.*b3)*k;
  m[1] = (2.*(b1+b3+b2+b0))*k;
  m[2] = (-2.*(b1+b2+b0+1.))*k;
  m[3] = m[14] = 2.*k;
  m[4] = (6.*b3)*k;
  m[5] = (-3. * (b1 + 2.*b3 + 2.*b2))*k;
  m[6] = (3. * (b1 + 2.*b2))*k;
  m[7] = m[11] = m[15] = 0.;
  m[8] = (-6.*b3)*k;
  m[9] = (6. * (b3-b0))*k;
  m[10] = (6.*b0)*k;
  m[12] = (2.*b3)*k;
  m[13] = (b1 + 4.*(b2+b0))*k;
}


double Matrix(const double &a, const double &b, const double &c, 
              const double &d, const double &alpha, const matrix &m){
  double p0, p1, p2, p3;
  p0 = m[0]*a  + m[1]*b  + m[2]*c  + m[3]*d;
  p1 = m[4]*a  + m[5]*b  + m[6]*c  + m[7]*d;
  p2 = m[8]*a  + m[9]*b  + m[10]*c + m[11]*d;
  p3 = m[12]*a + m[13]*b + m[14]*c + m[15]*d;

  return( p3 + alpha*( p2 + alpha*(p1 + alpha*p0) ) ); //p3 + alpha*p2 + (alpha^2)*p1 + (alpha^3)*p0
}


#define LOAD_ALPHA_POINTS_2D(idx_1, idx_2, idx_3, idx_4) \
  const T &km1 = knot_vec[idx_1],                        \
          &k0 = knot_vec[idx_2],                         \
          &k1 = knot_vec[idx_3],                         \
          &k2 = knot_vec[idx_4];                         \
  for(auto &alpha : alpha_vec){                          \
    T s;                                                 \
    s.x = Matrix(km1.x,  k0.x,  k1.x,  k2.x, alpha, m);  \
    s.y = Matrix(km1.y,  k0.y,  k1.y,  k2.y, alpha, m);  \
    spline_vec.push_back(s);                             \
  }


#define LOAD_ALPHA_POINTS_3D(idx_1, idx_2, idx_3, idx_4) \
  const T &km1 = knot_vec[idx_1],                        \
          &k0 = knot_vec[idx_2],                         \
          &k1 = knot_vec[idx_3],                         \
          &k2 = knot_vec[idx_4];                         \
  for(auto &alpha : alpha_vec){                          \
    T s;                                                 \
    s.x = Matrix(km1.x,  k0.x,  k1.x,  k2.x, alpha, m);  \
    s.y = Matrix(km1.y,  k0.y,  k1.y,  k2.y, alpha, m);  \
    s.z = Matrix(km1.z,  k0.z,  k1.z,  k2.z, alpha, m);  \
    spline_vec.push_back(s);                             \
  }


//grain - number of spline points to create between each knot
//first_last_duplicate - when true, spline points are computed between the first and second point, as well
//                       as between the second-to-last and last point. This is equivalent to making
//                       knot_vec[0] = knot_vec[1] and knot_vec[last_index-1] = knot_vec[last_index]
#define CUBIC_SPLINE_FUNC(func_name, load_alpha_macro) \
template <typename T>                                  \
void func_name(const std::vector<T> &knot_vec, std::vector<T> &spline_vec,                \
               SPLINE_TYPE spline_type = CUBIC_CATMULL_ROM,                               \
               const size_t grain = 5, const double tension = 0.5, const double bias = 0, \
               const bool first_last_duplicate = true){                                   \
  assert(knot_vec.size() > 3);                  \
  assert(grain > 0);                            \
  matrix m;                                     \
  const size_t num_knot = knot_vec.size();      \
\
  if(spline_type == CUBIC_CATMULL_ROM)           \
    GetCubicCatmullRomSpline(m);                 \
  else if(spline_type == CUBIC_CARDINAL)         \
    GetCubicCardinalMatrix(tension, m);          \
  else if(spline_type == CUBIC_B)                \
    GetCubicBSpline(m);                          \
  else if(spline_type == CUBIC_BETA){            \
    STD_INVALID_ARG_E(bias != 0 || tension != 0) \
    GetCubicBetaMatrix(bias, tension, m);        \
  }                                              \
  else                                           \
    assert("invalid spline type" && 0);          \
\
  /*alpha values zero and one will simply add the starting and stopping knots, respectively, so skip these values to
    avoid an unnecessary computation (ie we can just add the knots directly). Therefore, 'grain' becomes the number of
    spline points to create in between each two knots.*/\
  std::vector<double> alpha_vec(grain);                                                 \
  double i = 0;                                                                         \
  const double grain_inv = 1.0/static_cast<double>(grain+1);                            \
  std::generate(alpha_vec.begin(), alpha_vec.end(), [&](){ ++i; return i*grain_inv; }); \
  const size_t num_spline_point = num_knot + (num_knot-1)*grain;                        \
  spline_vec.clear();                                                                   \
  spline_vec.reserve(num_spline_point); /*TODO - make this a resize and use []*/        \
\
  /* Notice in each step, knot_vec points are directly added as per the above comment */ \
  /* 1 - This will create the spline points between the first and second knots*/         \
  if(first_last_duplicate){            \
    spline_vec.push_back(knot_vec[0]); \
    load_alpha_macro(0, 0, 1, 2)       \
  }                                    \
  /* 2 - Create spline points between knots:
     knot_vec[1] to knot_vec[2]
     knot_vec[2] to knot_vec[3]
     ...
     knot_vec[num_knot-3] to knot_vec[num_knot-2] */ \
  const size_t stop = num_knot-3;             \
  for(size_t i = 0; i < stop; i++){           \
    spline_vec.push_back(knot_vec[i+1]);      \
    load_alpha_macro(i, i+1, i+2, i+3)        \
  }                                           \
  spline_vec.push_back(knot_vec[num_knot-2]); \
  /* 3 - This will create the spline points between the second-to-last and last knots*/ \
  if(first_last_duplicate){                        \
    load_alpha_macro(stop, stop+1, stop+2, stop+2) \
    spline_vec.push_back(knot_vec[num_knot-1]);    \
  }                                                \
}


//void CubicSpline2D(const std::vector<T> &knot_vec, std::vector<T> &spline_vec, SPLINE_TYPE spline_type,
//                   const unsigned int grain, const double tension, const double bias)
CUBIC_SPLINE_FUNC(CubicSpline2D, LOAD_ALPHA_POINTS_2D)


//void CubicSpline3D(const std::vector<T> &knot_vec, std::vector<T> &spline_vec, SPLINE_TYPE spline_type,
//                   const unsigned int grain, const double tension, const double bias)
CUBIC_SPLINE_FUNC(CubicSpline3D, LOAD_ALPHA_POINTS_3D)

} //namespace sm

#endif //__MIO_SPLINE_H__

