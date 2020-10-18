#ifndef __MIO_ERROR_H__
#define __MIO_ERROR_H__

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <stdexcept>
#if __cplusplus > 199711L
#include <system_error>
#endif
#include <sstream>
#include <cassert>
#include <string>

#if defined __GNUC__
#define CURRENT_FUNC __PRETTY_FUNCTION__ //__func__
#elif defined _MSC_VER
#define CURRENT_FUNC __FUNCTION__
#else
#define CURRENT_FUNC ""
#endif

/*
  You can put the following into your CMakeLists.txt to make file paths relative
  to the project root dir.
  string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
  add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
*/
#ifndef SOURCE_PATH_SIZE
#define SOURCE_PATH_SIZE 0
#endif
#define FILENAME (__FILE__ + SOURCE_PATH_SIZE)

#define FRIENDLY_RETHROW(exception_){                                                                     \
  std::cout << "Exception caught here: " << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << "\n";   \
  throw exception_;                                                                                       \
}

//Exception system macros
#define EXCEPTION_MACRO_E(exp, exception_type)                                                         \
if( !!(exp) ) ; else{                                                                                  \
  std::ostringstream stream;                                                                           \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": (" << #exp << ") is false.\n";    \
  throw exception_type( stream.str() );                                                                \
}

#define EXCEPTION_MACRO_M(msg, exception_type)                                            \
{                                                                                         \
  std::ostringstream stream;                                                              \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ":" << msg << "\n";     \
  throw exception_type( stream.str() );                                                   \
}

#define EXCEPTION_MACRO_EM(exp, exception_type, opt_msg)            \
if( !!(exp) ) ; else{                                               \
  std::ostringstream stream;                                        \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ <<   \
            ": (" << #exp << ") is false. " << opt_msg << "\n";     \
  throw exception_type( stream.str() );                             \
}

//std::runtime_error
#define STD_RT_ERR_E(exp) EXCEPTION_MACRO_E(exp, std::runtime_error)
#define STD_RT_ERR_M(msg) EXCEPTION_MACRO_M(msg, std::runtime_error)
#define STD_RT_ERR_EM(exp, opt_msg) EXCEPTION_MACRO_EM(exp, std::runtime_error, opt_msg)

//std::logic_error
#define STD_LOG_ERR_E(exp) EXCEPTION_MACRO_E(exp, std::logic_error)
#define STD_LOG_ERR_M(msg) EXCEPTION_MACRO_M(msg, std::logic_error)
#define STD_LOG_ERR_EM(exp, opt_msg) EXCEPTION_MACRO_EM(exp, std::logic_error, opt_msg)

//std::invalid_argument()
#define STD_INVALID_ARG_E(exp) EXCEPTION_MACRO_E(exp, std::invalid_argument)
#define STD_INVALID_ARG_M(msg) EXCEPTION_MACRO_M(msg, std::invalid_argument)
#define STD_INVALID_ARG_EM(exp, opt_msg) EXCEPTION_MACRO_EM(exp, std::invalid_argument, opt_msg)

//std::length_error()
#define STD_LENGTH_ERROR_E(exp) EXCEPTION_MACRO_E(exp, std::length_error)
#define STD_LENGTH_ERROR_M(msg) EXCEPTION_MACRO_M(msg, std::length_error)
#define STD_LENGTH_ERROR_EM(exp, opt_msg) EXCEPTION_MACRO_EM(exp, std::length_error, opt_msg)

//std::system_error exception macro
#define EXP_CHK_SYSERR(exp)                                                                                         \
if( !!(exp) ) ; else{                                                                                               \
  std::ostringstream stream;                                                                                        \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": (" << #exp << ") is false. System message";    \
  throw std::system_error( errno, std::system_category(), stream.str() );                                           \
}

#define EXP_CHK_SYSERR_M(exp, opt_msg)                                         \
if( !!(exp) ) ; else{                                                          \
  std::ostringstream stream;                                                   \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ <<              \
            ": (" << #exp << ") is false. " << opt_msg << ". System message";  \
  throw std::system_error( errno, std::system_category(), stream.str() );      \
}


//Put this macro in a c++ stream to print the current function name and line number
//eg. std::cout << FL_STRM << "there was an error\n";
#define FL_STRM CURRENT_FUNC << ":" << __LINE__ << ": "

//Put this macro in a c++ stream to print the current file name, function name, and line number
#define FFL_STRM FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": "

#define ERRNO_STRM "errno message: " << std::strerror(errno)


/*
Use to check boolean expression that should normally evaluate as true.
Prints a formatted message that includes file name, function name, line number, and the expression.
Exit function can be any code you want to execute if the expression evaluates as false.
For example:
EXP_CHK(value > 0, return(false))
The above line will print the formatted message and call return if value is <= 0
*/
#define EXP_CHK(exp, exit_function)                                    \
if( !!(exp) ) ; else{                                                  \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ <<   \
               ": (" << #exp << ") is false.\n";                       \
  exit_function;                                                       \
}

/*
Same as EXP_CHK, but includes a user message
For example:
EXP_CHK(value > 0, return(false), "you entered an incorrect value")
*/
#define EXP_CHK_M(exp, exit_function, opt_msg)                         \
if( !!(exp) ) ; else{                                                  \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ <<   \
               ": (" << #exp << ") is false. " << opt_msg << "\n";     \
  exit_function;                                                       \
}

//Expression checking macros with errno evaluation
#define EXP_CHK_ERRNO(exp, exit_function)                                                       \
if( !!(exp) ) ; else{                                                                           \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ <<                            \
               ": (" << #exp << ") is false. errno message: " << std::strerror(errno) << "\n";  \
  exit_function;                                                                                \
}

#define EXP_CHK_ERRNO_M(exp, exit_function, opt_msg)                                                       \
if( !!(exp) ) ; else{                                                                                      \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << " - (" << #exp << ") is false. " <<   \
               opt_msg << ": errno message: " << std::strerror(errno) << "\n";                             \
  exit_function;                                                                                           \
}

#define MIO_ASSERT( expr ) \
if(!!(expr)) ; \
else mio::error( mio::ErrCode::StsAssert, #expr, CURRENT_FUNC, FILENAME, __LINE__ )

#ifdef _DEBUG
#define MIO_DBG_ASSERT(expr) MIO_ASSERT(expr)
#else
#define MIO_DBG_ASSERT(expr)
#endif

#endif //__MIO_ERROR_H__
