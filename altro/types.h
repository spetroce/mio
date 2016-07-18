#ifndef __MIO_TYPES_H__
#define __MIO_TYPES_H__

#include <iostream>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "mio/altro/error.h"
#include "mio/altro/macros.h"


#define CHECK_VECTOR_SIZE(vec_, size_) if(vec_.size() != size_) vec_.resize(size_);


#define TWO_VAL_STRUCT(class_name, type_name, data_type, init_val, var_a, var_b) \
typedef class class_name{ \
  public: \
    typedef data_type DataType; \
    data_type var_a, var_b; \
\
    class_name() : var_a(init_val), var_b(init_val){}; \
    class_name(data_type a_, data_type b_) : var_a(a_), var_b(b_){}; \
\
    class_name operator+(const class_name &param) const{ \
      return( class_name(var_a + param.var_a, var_b + param.var_b) ); \
    } \
\
    class_name operator-(const class_name &param) const{ \
      return( class_name(var_a - param.var_a, var_b - param.var_b) ); \
    } \
\
    class_name operator/(const data_type val) const{ \
      return( class_name(var_a / val, var_b / val) ); \
    } \
\
    friend std::ostream& operator<<(std::ostream& stream, const class_name& s); \
\
    COMPARE_VAL_LESS_THAN(class_name, var_a)\
    COMPARE_VAL_LESS_THAN(class_name, var_b)\
    COMPARE_VAL_GREATER_THAN(class_name, var_a)\
    COMPARE_VAL_GREATER_THAN(class_name, var_b)\
\
    template<class DATA_T> \
    static void Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, std::vector<DATA_T> &dst1); \
\
    template<class DATA_T> \
    static void Merge(const std::vector<DATA_T> &src0, const std::vector<DATA_T> &src1, std::vector<class_name> &dst); \
\
}__attribute__((packed)) type_name; \
\
\
inline std::ostream& operator<<(std::ostream& stream, const class_name& s){ \
  stream << "(" << s.var_a << ", " << s.var_b << ")"; \
  return(stream); \
} \
\
\
template<class DATA_T> \
void class_name::Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, std::vector<DATA_T> &dst1){ \
  const size_t src_size = src.size(); \
  CHECK_VECTOR_SIZE(dst0, src_size) \
  CHECK_VECTOR_SIZE(dst1, src_size) \
  for(size_t i = 0; i < src_size; ++i){ \
    dst0[i] = src[i].var_a; \
    dst1[i] = src[i].var_b; \
  } \
}\
\
\
template<class DATA_T> \
void Merge(const std::vector<DATA_T> &src0, const std::vector<DATA_T> &src1, std::vector<class_name> &dst){ \
  STD_INVALID_ARG_E( src0.size() == src1.size() ) \
  const size_t src_size = src0.size(); \
  CHECK_VECTOR_SIZE(dst, src_size) \
  for(size_t i = 0; i < src_size; ++i) \
    dst[i] = class_name(src0[i], src1[i]); \
}


#define THREE_VAL_STRUCT(class_name, type_name, data_type, init_val, var_a, var_b, var_c) \
typedef class class_name{ \
  public: \
    typedef data_type DataType; \
    data_type var_a, var_b, var_c; \
\
    class_name() : var_a(init_val), var_b(init_val), var_c(init_val){}; \
    class_name(data_type a_, data_type b_, data_type c_) : var_a(a_), var_b(b_), var_c(c_){}; \
\
    class_name operator+(const class_name &param) const{ \
      return( class_name(var_a + param.var_a, var_b + param.var_b, var_c + param.var_c) ); \
    } \
\
    class_name operator-(const class_name &param) const{ \
      return( class_name(var_a - param.var_a, var_b - param.var_b, var_c - param.var_c) ); \
    } \
\
    class_name operator/(const data_type val) const{ \
      return( class_name(var_a / val, var_b / val, var_c / val) ); \
    } \
\
    friend std::ostream& operator<<(std::ostream& stream, const class_name& s); \
\
    COMPARE_VAL_LESS_THAN(class_name, var_a)\
    COMPARE_VAL_LESS_THAN(class_name, var_b)\
    COMPARE_VAL_LESS_THAN(class_name, var_c)\
    COMPARE_VAL_GREATER_THAN(class_name, var_a)\
    COMPARE_VAL_GREATER_THAN(class_name, var_b)\
    COMPARE_VAL_GREATER_THAN(class_name, var_c)\
\
    template<class DATA_T> \
    static void Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, \
                      std::vector<DATA_T> &dst1, std::vector<DATA_T> &dst2); \
\
}__attribute__((packed)) type_name; \
\
inline std::ostream& operator<<(std::ostream& stream, const class_name& s){ \
  stream << "(" << s.var_a << ", " << s.var_b << ", " << s.var_c << ")"; \
  return(stream); \
} \
\
template<class DATA_T> \
void class_name::Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, \
                       std::vector<DATA_T> &dst1, std::vector<DATA_T> &dst2){ \
  const size_t src_size = src.size(); \
  CHECK_VECTOR_SIZE(dst0, src_size) \
  CHECK_VECTOR_SIZE(dst1, src_size) \
  CHECK_VECTOR_SIZE(dst2, src_size) \
  for(size_t i = 0; i < src_size; ++i){ \
    dst0[i] = src[i].var_a; \
    dst1[i] = src[i].var_b; \
    dst2[i] = src[i].var_c; \
  } \
}


#define FOUR_VAL_STRUCT(class_name, type_name, data_type, init_val, var_a, var_b, var_c, var_d) \
typedef class class_name{ \
  public: \
    typedef data_type DataType;           \
    data_type var_a, var_b, var_c, var_d; \
\
    class_name() : var_a(init_val), var_b(init_val), var_c(init_val), var_d(init_val){}; \
    class_name(data_type a_, data_type b_, data_type c_, data_type d_) :                 \
        var_a(a_), var_b(b_), var_c(c_), var_d(d_){};                                    \
\
    class_name operator+(const class_name &param) const{                                                        \
      return( class_name(var_a + param.var_a, var_b + param.var_b, var_c + param.var_c, var_d + param.var_d) ); \
    }                                                                                                           \
\
    class_name operator-(const class_name &param) const{                                                        \
      return( class_name(var_a - param.var_a, var_b - param.var_b, var_c - param.var_c, var_d - param.var_d) ); \
    }                                                                                                           \
\
    class_name operator/(const data_type val) const{                            \
      return( class_name(var_a / val, var_b / val, var_c / val, var_d / val) ); \
    }                                                                           \
\
    friend std::ostream& operator<<(std::ostream& stream, const class_name& s); \
\
    COMPARE_VAL_LESS_THAN(class_name, var_a)    \
    COMPARE_VAL_LESS_THAN(class_name, var_b)    \
    COMPARE_VAL_LESS_THAN(class_name, var_c)    \
    COMPARE_VAL_LESS_THAN(class_name, var_d)    \
    COMPARE_VAL_GREATER_THAN(class_name, var_a) \
    COMPARE_VAL_GREATER_THAN(class_name, var_b) \
    COMPARE_VAL_GREATER_THAN(class_name, var_c) \
    COMPARE_VAL_GREATER_THAN(class_name, var_d) \
\
    template<class DATA_T>                                                                                      \
    static void Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, std::vector<DATA_T> &dst1, \
                      std::vector<DATA_T> &dst2, std::vector<DATA_T> &dst3);                                    \
\
}__attribute__((packed)) type_name; \
\
inline std::ostream& operator<<(std::ostream& stream, const class_name& s){                 \
  stream << "(" << s.var_a << ", " << s.var_b << ", " << s.var_c << ", " << s.var_d << ")"; \
  return(stream);                                                                           \
}                                                                                           \
\
template<class DATA_T>                                                                                           \
void class_name::Split(const std::vector<class_name> &src, std::vector<DATA_T> &dst0, std::vector<DATA_T> &dst1, \
                       std::vector<DATA_T> &dst2, std::vector<DATA_T> &dst3){                                    \
  const size_t src_size = src.size(); \
  CHECK_VECTOR_SIZE(dst0, src_size) \
  CHECK_VECTOR_SIZE(dst1, src_size) \
  CHECK_VECTOR_SIZE(dst2, src_size) \
  CHECK_VECTOR_SIZE(dst3, src_size) \
  for(size_t i = 0; i < src_size; ++i){ \
    dst0[i] = src[i].var_a;             \
    dst1[i] = src[i].var_b;             \
    dst2[i] = src[i].var_c;             \
    dst3[i] = src[i].var_d;             \
  }                                     \
}


// COLORS
THREE_VAL_STRUCT(CColor3ub, color3ub_t, uint8_t, 0, r, g, b)
THREE_VAL_STRUCT(CColor3f, color3f_t, float, 0.0f, r, g, b)
FOUR_VAL_STRUCT(CColor4ub, color4ub_t, uint8_t, 0, r, g, b, a)
FOUR_VAL_STRUCT(CColor4f, color4f_t, float, 0.0f, r, g, b, a)

// VERTEX
TWO_VAL_STRUCT(CVertex2n, vertex2n_t, int32_t, 0, x, y)
TWO_VAL_STRUCT(CVertex2f, vertex2f_t, float, 0.0f, x, y)
TWO_VAL_STRUCT(CVertex2d, vertex2d_t, double, 0.0, x, y)
THREE_VAL_STRUCT(CVertex3n, vertex3n_t, int32_t, 0, x, y, z)
THREE_VAL_STRUCT(CVertex3f, vertex3f_t, float, 0.0f, x, y, z)
THREE_VAL_STRUCT(CVertex3d, vertex3d_t, double, 0.0, x, y, z)
FOUR_VAL_STRUCT(CVertex4f, vertex4f_t, float, 0.0f, x, y, z, w)
FOUR_VAL_STRUCT(CVertex4d, vertex4d_t, double, 0.0, x, y, z, w)

// VECTOR
THREE_VAL_STRUCT(CVector3f, vec3f_t, float, 0.0f, x, y, z)
THREE_VAL_STRUCT(CVector3d, vec3d_t, double, 0.0, x, y, z)

// SCALAR
TWO_VAL_STRUCT(CScalar2n, scalar2n_t, int32_t, 0, val0, val1)
TWO_VAL_STRUCT(CScalar2f, scalar2f_t, float, 0.0f, val0, val1)
TWO_VAL_STRUCT(CScalar2d, scalar2d_t, double, 0.0, val0, val1)
THREE_VAL_STRUCT(CScalar3f, scalar3f_t, float, 0.0f, val0, val1, val2)
FOUR_VAL_STRUCT(CScalar4f, scalar4f_t, float, 0.0f, val0, val1, val2, val3)

// MODEL COEF
THREE_VAL_STRUCT(CLine3f, line3f_t, float, 0.0f, a, b, c)
THREE_VAL_STRUCT(CLine3d, line3d_t, double, 0.0, a, b, c)
FOUR_VAL_STRUCT(CPlane4f, plane4f_t, float, 0.0f, a, b, c, d)
FOUR_VAL_STRUCT(CPlane4d, plane4d_t, double, 0.0, a, b, c, d)

// OTHER
THREE_VAL_STRUCT(CDimension3f, dimension3f_t, float, 0.0f, x, y, z)
THREE_VAL_STRUCT(CPanTiltTranslate, ptt_t, int32_t, 0, pan, tilt, trans)
TWO_VAL_STRUCT(CSize2f, size2f_t, float, 0.0f, height, width)
TWO_VAL_STRUCT(CSize2n, size2n_t, int32_t, 0, height, width)


#undef COMPARE_VAL_LESS_THAN
#undef COMPARE_VAL_GREATER_THAN
#undef CHECK_VECTOR_SIZE
#undef TWO_VAL_STRUCT
#undef THREE_VAL_STRUCT
#undef FOUR_VAL_STRUCT

#endif //__MIO_TYPES_H__

