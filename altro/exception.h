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

} //namespace mio
#endif