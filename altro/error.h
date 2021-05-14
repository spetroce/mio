#ifndef __MIO_ERROR_H__
#define __MIO_ERROR_H__

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>
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
#define EXP_CHK(exp, exit_function) \
if (!!(exp)) ; else { \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << \
               ": (" << #exp << ") is false.\n"; \
  exit_function; \
}

/*
Same as EXP_CHK, but includes a user message
For example:
EXP_CHK(value > 0, return(false), "you entered an incorrect value")
*/
#define EXP_CHK_M(exp, exit_function, opt_msg) \
if (!!(exp)) ; else { \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << \
               ": (" << #exp << ") is false. " << opt_msg << "\n"; \
  exit_function; \
}

//Expression checking macros with errno evaluation
#define EXP_CHK_ERRNO(exp, exit_function) \
if (!!(exp)) ; else { \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << \
               ": (" << #exp << ") is false. errno message: " << std::strerror(errno) << "\n"; \
  exit_function; \
}

#define EXP_CHK_ERRNO_M(exp, exit_function, opt_msg) \
if (!!(exp)) ; else { \
  std::cout << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << " - (" << #exp << ") is false. " << \
               opt_msg << ": errno message: " << std::strerror(errno) << "\n"; \
  exit_function; \
}

#ifdef WITH_MIO_BOOST_LOGGING

#define LOG(level) \
  BOOST_LOG_SEV(Logging::GeneralLog(), boost::log::trivial::level) << \
    FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": "

#define LOG_EXP(level, exp, exit_function) \
  if ( !!(exp) ) ; else { \
    BOOST_LOG_SEV(Logging::GeneralLog(), boost::log::trivial::level) << \
      FILENAME << ":" << CURRENT_FUNC << ":" <<  \
      __LINE__ << ": (" << #exp << ") is false.\n"; \
    exit_function; \
  }

#define LOG_EXP_M(level, exp, exit_function, message) \
  if ( !!(exp) ) ; else { \
    BOOST_LOG_SEV(Logging::GeneralLog(), boost::log::trivial::level) << \
      FILENAME << ":" << CURRENT_FUNC << ":" <<  \
      __LINE__ << ": (" << #exp << ") is false. " << message << "\n"; \
    exit_function; \
  }

#define LOG_EXP_ERRNO(level, exp, exit_function) \
  if( !!(exp) ) ; else { \
    BOOST_LOG_SEV(Logging::GeneralLog(), boost::log::trivial::level) << \
      FILENAME << ":" << CURRENT_FUNC << ":" << \
        __LINE__ << ": (" << #exp << ") is false." << \
        " errno message: " << std::strerror(errno) << "\n"; \
    exit_function; \
  }

#else

#define LOG(level) std::cout << FFL_STRM

#define LOG_EXP(level, exp, exit_function) \
  EXP_CHK(exp, exit_function)

#define LOG_EXP_M(level, exp, exit_function, message) \
  EXP_CHK_M(exp, exit_function, message)

#define LOG_EXP_ERRNO(level, exp, exit_function) \
  EXP_CHK_ERRNO(exp, exit_function)

#endif

#endif //__MIO_ERROR_H__
