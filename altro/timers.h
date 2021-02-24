#ifndef __MIO_TIMERS_H__
#define __MIO_TIMERS_H__

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>


namespace mio{

double diff(timespec time_start, timespec time_stop) {
  timespec time_diff;
  if ((time_stop.tv_nsec - time_start.tv_nsec) < 0) {
    time_diff.tv_sec = time_stop.tv_sec-time_start.tv_sec - 1;
    time_diff.tv_nsec = 1000000000 + time_stop.tv_nsec - time_start.tv_nsec;
  } else {
    time_diff.tv_sec = time_stop.tv_sec - time_start.tv_sec;
    time_diff.tv_nsec = time_stop.tv_nsec - time_start.tv_nsec;
  }
  return static_cast<double>(time_diff.tv_sec) +
         (static_cast<double>(time_diff.tv_nsec) * 0.000000001);
}

#define MIO_HR_TIMER_START \
  timespec time_start, time_stop; \
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);

#define MIO_HR_TIMER_STOP \
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);

#define MIO_HR_TIMER_DIFF diff(time_start, time_stop)

#define MIO_HR_TIMER_DIFF_STREAM \
std::fixed << std::setprecision(9) << diff(time_start, time_stop)


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
