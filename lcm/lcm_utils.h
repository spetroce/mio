#ifndef __MIO_LCM_UTIL_H__
#define __MIO_LCM_UTIL_H__

#include <lcm/lcm.h>
#include <thread>
#include "mio/altro/error.h"


#define CV_MAT_TO_LCM_FRAME(mat, frame)          \
frame.rows = mat.rows;                           \
frame.cols = mat.cols;                           \
frame.channels = mat.channels();                 \
frame.openCvType = mat.type();                   \
frame.length = mat.cols*mat.rows*mat.elemSize(); \
void *void_ptr = static_cast<void*>(mat.data);   \
frame.data = static_cast<int8_t*>(void_ptr);


inline bool lcm_handle_to(lcm_t *lcm, int lcm_fd, long int tv_sec_, long int tv_usec_ = 0){
  //int lcm_fd = lcm_get_fileno(lcm);
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(lcm_fd, &fds);

  //wait a limited amount of time for an incoming message
  struct timeval timeout = {tv_sec_, tv_usec_};
  int status = select(lcm_fd + 1, &fds, 0, 0, &timeout);
  if(0 == status)
    return false;
  else if( FD_ISSET(lcm_fd, &fds) ){
    lcm_handle(lcm); //event(s) ready to be processed
    return true;
  }
}


class LCMHandlerThread{
  public:
    bool exit_thread_, started_;
    std::thread thread_;
    lcm_t *lcm_;

    LCMHandlerThread() : exit_thread_(false), started_(false){}

    LCMHandlerThread(lcm_t *lcm) : exit_thread_(false), started_(false){
      STD_INVALID_ARG_E(lcm != nullptr)
      lcm_ = lcm;
    }

    void SetLCM(lcm_t *lcm){
      STD_INVALID_ARG_E(lcm != nullptr)
      lcm_ = lcm;
    }

    void Thread(){
      int lcm_fd = lcm_get_fileno(lcm_);
      for(;;){
        if(exit_thread_)
          break;
        lcm_handle_to(lcm_, lcm_fd, 1);
      }
    }

    void Start(){
      EXP_CHK(started_ == false, return)
      exit_thread_ = false;
      thread_ = std::thread(&LCMHandlerThread::Thread, this);
      printf("%s - started\n", CURRENT_FUNC);
      started_ = true;
    }

    void Stop(){
      EXP_CHK(started_ == true, return)
      exit_thread_ = true;
      thread_.join();
      printf("%s - stopped\n", CURRENT_FUNC);
      started_ = false;
    }
};

#endif //__MIO_LCM_UTIL_H__

