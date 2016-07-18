#ifndef __MIO_TIMERS_H__
#define __MIO_TIMERS_H__

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>


namespace mio{

class SysTimer{
  private:
    timeval tv_clk_a_, tv_clk_b_;
    const char *name_;
    double time_diff_;

  public:
    SysTimer(const char *name_) : name_(name_), time_diff_(0){ start(); }
    SysTimer() : name_(NULL), time_diff_(0){ start(); }
    ~SysTimer(){}

    void print(){
      if(name_)
        printf("%s system time: %.6f\n", name_, time_diff_);
      else
        printf("system time: %.6f\n", time_diff_);
    }

    void start(){ 
      gettimeofday(&tv_clk_a_, NULL);
    }

    double stop(bool print_flag = true){
      gettimeofday(&tv_clk_b_, NULL);
      time_diff_ = (tv_clk_b_.tv_sec - tv_clk_a_.tv_sec) + (tv_clk_b_.tv_usec - tv_clk_a_.tv_usec) * 0.000001;
      if(print_flag)
        print();
      return time_diff_;
    }

    double get_time_diff(){
      return time_diff_;
    }
};


class RUTimer{
  private:
    struct timeval tv_diff_;
    struct rusage ru_clk_a_, ru_clk_b_;
    double time_diff_;
    const char *name_;
    int who_;

  public:
    RUTimer(const char *name_, int who_ = RUSAGE_THREAD) : name_(name_), who_(who_), time_diff_(0){ start(); }
    RUTimer(int who_ = RUSAGE_THREAD) : name_(NULL), who_(who_), time_diff_(0){ start(); }
    ~RUTimer(){}

    void print(){
      if(name_)
        printf("%s rusage time: %.6f\n", name_, time_diff_);
      else
        printf("rusage time: %.6f\n", time_diff_);
    }

    void start(){ 
      getrusage(who_, &ru_clk_a_); 
    }

    double stop(bool print_flag = true){
      getrusage(who_, &ru_clk_b_);
      timersub(&ru_clk_b_.ru_utime, &ru_clk_a_.ru_utime, &tv_diff_);
      time_diff_ = tv_diff_.tv_sec + tv_diff_.tv_usec * 0.000001;
      if(print_flag)
        print();
      return time_diff_;
    }

    double get_time_diff(){
      return time_diff_;
    }
};

typedef SysTimer CSysTimer;
typedef RUTimer CRUTimer;

} //namespace mio

#endif //__MIO_TIMERS_H__
