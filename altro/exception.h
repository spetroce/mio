#include <stdexcept>
#if __cplusplus > 199711L
#include <system_error>
#endif

#define FRIENDLY_RETHROW(exception_) \
{ \
  std::cout << "Exception caught here: " << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << "\n"; \
  throw exception_; \
}

//Exception system macros
#define EXCEPTION_MACRO_E(exp, exception_type) \
if (!!(exp)) ; else { \
  std::ostringstream stream; \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": (" << #exp << ") is false.\n"; \
  throw exception_type( stream.str() ); \
}

#define EXCEPTION_MACRO_M(msg, exception_type) \
{ \
  std::ostringstream stream; \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ":" << msg << "\n"; \
  throw exception_type(stream.str()); \
}

#define EXCEPTION_MACRO_EM(exp, exception_type, opt_msg) \
if (!!(exp)) ; else { \
  std::ostringstream stream; \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << \
            ": (" << #exp << ") is false. " << opt_msg << "\n"; \
  throw exception_type(stream.str()); \
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
#define EXP_CHK_SYSERR(exp) \
if (!!(exp)) ; else { \
  std::ostringstream stream; \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << ": (" << #exp << ") is false. System message"; \
  throw std::system_error( errno, std::system_category(), stream.str() ); \
}

#define EXP_CHK_SYSERR_M(exp, opt_msg) \
if (!!(exp)) ; else { \
  std::ostringstream stream; \
  stream << FILENAME << ":" << CURRENT_FUNC << ":" << __LINE__ << \
            ": (" << #exp << ") is false. " << opt_msg << ". System message"; \
  throw std::system_error( errno, std::system_category(), stream.str() ); \
}

/*
  below is a modified version of OpenCV's assertion/error system found in base.hpp, system.cpp, core.hpp
  just another way to do things...
*/
#if __cplusplus > 199711L
namespace mio{

enum ErrCode { StsOk =        0,
               StsError =    -1,
               StsInternal = -3,
               StsAssert =   -2 };


inline const char *ErrStr(int status){
  static char buf[256];

  switch(status){
    case ErrCode::StsOk :       return "No Error";
    case ErrCode::StsError :    return "Unspecified error";
    case ErrCode::StsInternal : return "Internal error";
    case ErrCode::StsAssert :   return "Assertion failed";
  };

  sprintf(buf, "Unknown %s code %d", status >= 0 ? "status":"error", status);
  return buf;
}


class Exception : public std::exception{
  public:
    int code_, line_;
    std::string msg_, err_, func_, file_;

    Exception(){
      code_ = 0;
      line_ = 0;
    }

    Exception(int code, const std::string &err, const std::string &func, const std::string &file, int line)
        : code_(code), line_(line), err_(err), func_(func), file_(file) {
      formatMessage();
    }

    ~Exception() throw() {}

    virtual const char *what() const throw();

    void formatMessage(){
      //TODO: copy what's being done with streams above
//      if(func_.size() > 0)
//        msg_ = format("%s:%d: error: (%d) %s in function %s\n", file_.c_str(), line_, code_, err_.c_str(), func_.c_str());
//      else
//        msg_ = format("%s:%d: error: (%d) %s\n", file_.c_str(), line_, code_, err_.c_str());
    }
};

inline const char* mio::Exception::what() const throw(){
  return msg_.c_str();
}


inline void error(const mio::Exception &exc){
  char buf[1 << 16];
  sprintf(buf, "OpenCV Error: %s (%s) in %s, file %s, line %d",
          mio::ErrStr(exc.code_), exc.err_.c_str(), exc.func_.size() > 0 ?
          exc.func_.c_str() : "unknown function", exc.file_.c_str(), exc.line_);
  fprintf(stderr, "%s\n", buf);
  fflush(stderr);

  throw exc;
}


inline void error(int code, const std::string &err, const char *func, const char *file, int line){
  mio::error(mio::Exception(code, err, func, file, line));
}

#define MIO_ASSERT( expr ) \
if (!!(expr)) ; \
else mio::error( mio::ErrCode::StsAssert, #expr, CURRENT_FUNC, FILENAME, __LINE__ )

#ifdef _DEBUG
#define MIO_DBG_ASSERT(expr) MIO_ASSERT(expr)
#else
#define MIO_DBG_ASSERT(expr)
#endif

} //namespace mio
#endif