#ifndef __MIO_FREQ_BUFFER_H__
#define __MIO_FREQ_BUFFER_H__

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include "mio/altro/error.h"

//#define __FREQ_BUF_DBG__


namespace mio{

template <typename DATA_T>
class CFreqBuffer{
  public:
    CFreqBuffer(){
      is_not_spurious_wake_up_[0] = is_not_spurious_wake_up_[1] = false;
      is_init_ = timed_pop_is_awake_ = exit_timed_pop_flag_ = false;
    }

    CFreqBuffer(std::function<void(DATA_T, void*)> user_func, const size_t freq = 10,
                const size_t max_buf_size = 5, void *user_data = nullptr){
      is_not_spurious_wake_up_[0] = is_not_spurious_wake_up_[1] = false;
      is_init_ = exit_timed_pop_flag_ = false;
      Init(user_func, freq, max_buf_size);
    }

    ~CFreqBuffer(){
#ifdef __FREQ_BUF_DBG__
      printf("%s : called\n", CURRENT_FUNC);
#endif
      EXP_CHK_EM(!is_init_, Uninit(), "user should be calling CFreqBuffer::Uninit()")
#ifdef __FREQ_BUF_DBG__
      printf("%s : destroyed\n", CURRENT_FUNC);
#endif
    }

    // A frequency of more than 99 issues a "no wait" policy (ie. frequency = infinity)
    void Init(std::function<void(DATA_T, void*)> user_func, const size_t freq = 10,
              const size_t max_buf_size = 5, void *user_data = nullptr){
      EXP_CHK_E(!is_init_, return)
      printf("%s - initializing...", CURRENT_FUNC);
      user_func_ = user_func;
      freq_ = freq;
      max_buf_size_ = max_buf_size;
      user_data_ = user_data;
      thread_ = std::thread(&CFreqBuffer::TimedPop, this);
      //wait for startup
      std::unique_lock<std::mutex> ul(cond_mtx_[1]);
      is_not_spurious_wake_up_[1] = false;
      is_init_ = false;
      std::function<bool()> pred_func = [this]{ is_init_ = is_not_spurious_wake_up_[1];
                                                return is_not_spurious_wake_up_[1]; };
      assert( ul.owns_lock() );
      condition_var_[1].wait(ul, pred_func);

      printf(" initialized.\n");
    }

    void Uninit(){
      EXP_CHK_E(is_init_, return)
#ifdef __FREQ_BUF_DBG__
      printf("%s Uninitializing...", CURRENT_FUNC);
#endif
      exit_timed_pop_flag_ = true;
      WakeUpTimedPop();
      thread_.join();
      is_init_ = false;
#ifdef __FREQ_BUF_DBG__
      printf(" uninitialized.\n");
#endif
    }

    void Push(const DATA_T value){
      EXP_CHK_E(is_init_, return)
      buf_mtx_.lock();
      if(fifo_value_buf_.size() > max_buf_size_)
        fifo_value_buf_.pop();
      fifo_value_buf_.push(value);
      buf_mtx_.unlock();
      WakeUpTimedPop();
    }

  private:
    size_t max_buf_size_ = 4, freq_ = 5;
    std::function<void(DATA_T, void*)> user_func_;
    void *user_data_;
    std::queue<DATA_T> fifo_value_buf_;
    std::thread thread_;
    std::condition_variable condition_var_[2];
    std::mutex buf_mtx_, cond_mtx_[2];
    bool is_not_spurious_wake_up_[2], timed_pop_is_awake_, is_init_, exit_timed_pop_flag_;

    void TimedPop(){
      assert(!is_init_); // Gets set below
      const bool no_wait = (freq_ > 99);
      const size_t period = no_wait ? 0 : (1.0/freq_)*1000.0;
      const size_t max_checks = no_wait ? 1 : freq_; // When no_wait=false, we check fifo_value_buf_ for one second
      size_t num_check = 0; // Count var for number of times the buffer was checked for a value (done for good measure)
      for(;;){
        if(exit_timed_pop_flag_)
          break;
        buf_mtx_.lock();
        num_check = (fifo_value_buf_.size() == 0) ? num_check+1 : 0;
        buf_mtx_.unlock();
        if(num_check > max_checks){
          std::unique_lock<std::mutex> ul(cond_mtx_[0]); // Wraps mutex in a unique_lock and calls lock()
          is_not_spurious_wake_up_[0] = false;
          timed_pop_is_awake_ = false;
          std::function<bool()> pred_func = [this]{ timed_pop_is_awake_ = is_not_spurious_wake_up_[0];
#ifdef __FREQ_BUF_DBG__
                                                    printf("pred_func called\n");
#endif
                                                    return is_not_spurious_wake_up_[0]; };
          assert( ul.owns_lock() ); // sanity check: ul must be locked before wait is called
          if(!is_init_){
            // signal Init() that thread is initialized
            std::unique_lock<std::mutex> is_init_ul(cond_mtx_[1]);
            is_not_spurious_wake_up_[1] = true;
            is_init_ul.unlock();
            condition_var_[1].notify_one();
          } 
          // wait() calls ul.unlock() and blocks thread until a notify func is called or a spurious signal is received.
          // We have the pred_func as a safe guard to spurious signals. After signal is received, ul.lock() is called.
          condition_var_[0].wait(ul, pred_func);
          num_check = 0;
          // ul destructor calls ul.unlock()
        }
        // Make sure we actually have a value and call the user func outside the buffer mutex lock
        DATA_T value;
        bool got_value = false;
        buf_mtx_.lock();
        if(fifo_value_buf_.size() > 0){
          assert(num_check == 0);
          value = fifo_value_buf_.front();
          got_value = true;
          fifo_value_buf_.pop();
        }
        buf_mtx_.unlock();
        if(got_value){
          user_func_(value, user_data_);
#ifdef __FREQ_BUF_DBG__
        std::cout << CURRENT_FUNC << " : " << "calling user_func with value " << value << std::endl;
#endif
        }
#ifdef __FREQ_BUF_DBG__
        else
          std::cout << CURRENT_FUNC << " : " << "checking for new value: " << num_check << std::endl;
#endif
        std::this_thread::sleep_for( std::chrono::milliseconds(period) );
      }
    }

    void WakeUpTimedPop(){
      assert(is_init_);
      std::unique_lock<std::mutex> ul(cond_mtx_[0]); // Wraps mutex in a unique_lock and calls lock()
      is_not_spurious_wake_up_[0] = true;
      if(!timed_pop_is_awake_){
#ifdef __FREQ_BUF_DBG__
        printf("%s : waking up TimedPop()\n", CURRENT_FUNC);
#endif
        ul.unlock();
        condition_var_[0].notify_one();
      }
    }
};

} //namespace mio

#endif //__MIO_FREQ_BUFFER_H__

