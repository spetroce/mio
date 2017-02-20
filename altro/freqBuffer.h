#ifndef __MIO_FREQ_BUFFER_H__
#define __MIO_FREQ_BUFFER_H__

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include "mio/altro/error.h"

#ifndef NDEBUG
#define __FREQ_BUF_DBG__
#endif


namespace mio{

template <typename DATA_T>
class CFreqBuffer{
  public:
    CFreqBuffer(){
      SetDefaultValues();
    }


    CFreqBuffer(std::function<void(DATA_T, void*)> user_func, const size_t freq = 10,
                const size_t max_buf_size = 5, const bool with_fifo_checking = false,
                void *user_data = nullptr){
      SetDefaultValues();
      Init(user_func, freq, max_buf_size, with_fifo_checking);
    }


    ~CFreqBuffer(){
#ifdef __FREQ_BUF_DBG__
      printf("%s - called\n", CURRENT_FUNC);
#endif
      EXP_CHK_M(!is_init_, Uninit(), "user should be calling CFreqBuffer::Uninit()")
#ifdef __FREQ_BUF_DBG__
      printf("%s - destroyed\n", CURRENT_FUNC);
#endif
    }


    void SetDefaultValues(){
      is_not_spurious_wake_up_[0] = is_not_spurious_wake_up_[1] = false;
      is_init_ = timed_pop_is_awake_ = exit_timed_pop_flag_ = with_fifo_checking_ = false;
    }


    // A frequency of more than 99 issues a "no wait" policy (ie. frequency = infinity)
    void Init(std::function<void(DATA_T, void*)> user_func, const size_t freq = 10,
              const size_t max_buf_size = 5, const bool with_fifo_checking = false,
              void *user_data = nullptr){
      EXP_CHK(!is_init_, return)
      printf("%s - Initializing...\n", CURRENT_FUNC);
      user_func_ = user_func;
      freq_ = freq;
      max_buf_size_ = max_buf_size;
      with_fifo_checking_ = with_fifo_checking;
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

      printf("%s - Initialized.\n", CURRENT_FUNC);
    }


    void Uninit(){
      EXP_CHK(is_init_, return)
#ifdef __FREQ_BUF_DBG__
      printf("%s - Uninitializing...\n", CURRENT_FUNC);
#endif
      exit_timed_pop_flag_ = true;
      WakeUpTimedPop();
      thread_.join();
      is_init_ = false;
#ifdef __FREQ_BUF_DBG__
      printf("%s - Uninitialized\n", CURRENT_FUNC);
#endif
    }


    void Push(const DATA_T value){
      EXP_CHK(is_init_, return)
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
    //condition_var_[0] is used to 'wake up' TimedPop()
    //condition_var_[1] is used in Init() to wait for TimedPop to start. After, it is used to signal
    //WakeUpTimedPop() that TimedPop has in fact woken up. This is necessary because should WakeUpTimedPop() be
    //called many times in an instant, timed_pop_is_awake_ will not have updated yet causing WakeUpTimedPop() to
    //try and wake up TimedPop() many times.
    std::condition_variable condition_var_[2];
    std::mutex buf_mtx_, cond_mtx_[2];
    bool is_not_spurious_wake_up_[2], timed_pop_is_awake_, is_init_, exit_timed_pop_flag_, with_fifo_checking_;


    void TimedPop(){
      assert(!is_init_); // Gets set below
      const size_t kPeriod = (1.0/freq_)*1000.0; // Period in milliseconds
      size_t i = 0;
      float max_check_time;
      do{
        ++i;
        max_check_time = i*kPeriod;
      }while(max_check_time < 100 && i <= 3);
      const size_t kMaxNumChecks = i; //maximum number of fifo checks
      size_t num_check = 0; // Number of times the buffer was checked for a value

      for(;;){
        if(exit_timed_pop_flag_)
          break;
        buf_mtx_.lock();
        const bool fifo_is_empty = (fifo_value_buf_.size() == 0);
        num_check = fifo_is_empty ? num_check+1 : 0;
        buf_mtx_.unlock();
        // if initializing, set with_fifo_checking to false, or exceeded 1 sec of checking, put thread to sleep
        if(!is_init_ || (!with_fifo_checking_ && fifo_is_empty) || num_check > kMaxNumChecks){
          std::unique_lock<std::mutex> ul(cond_mtx_[0]); // Wraps mutex in a unique_lock and calls lock()
          is_not_spurious_wake_up_[0] = false;
          timed_pop_is_awake_ = false;
          std::function<bool()> pred_func = [this]{ timed_pop_is_awake_ = is_not_spurious_wake_up_[0];
#ifdef __FREQ_BUF_DBG__
                                                    printf("%s - pred_func called\n", CURRENT_FUNC);
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

          // signal WakeUpTimedPop() that TimedPop() woke up
          std::unique_lock<std::mutex> is_awake_ul(cond_mtx_[1]);
          is_not_spurious_wake_up_[1] = true;
          is_awake_ul.unlock();
          condition_var_[1].notify_one();

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
#ifdef __FREQ_BUF_DBG__
        std::cout << CURRENT_FUNC << " : " << "calling user_func with value " << value << std::endl;
#endif
          user_func_(value, user_data_);
        }
#ifdef __FREQ_BUF_DBG__
        else
          printf("%s - fifo check count: %d\n", CURRENT_FUNC, num_check);
#endif
        std::this_thread::sleep_for( std::chrono::milliseconds(kPeriod) );
      }
    }


    void WakeUpTimedPop(){
      assert(is_init_);
      std::unique_lock<std::mutex> ul(cond_mtx_[0]); // Wraps mutex in a unique_lock and calls lock()
      is_not_spurious_wake_up_[0] = true;
      if(!timed_pop_is_awake_){
#ifdef __FREQ_BUF_DBG__
        printf("%s - waking up TimedPop()\n", CURRENT_FUNC);
#endif
        ul.unlock();
        condition_var_[0].notify_one();

        //wait for TimedPop() to wake up
        std::unique_lock<std::mutex> ul(cond_mtx_[1]);
        std::function<bool()> pred_func = [this]{return timed_pop_is_awake_;};
        assert( ul.owns_lock() );
        condition_var_[1].wait(ul, pred_func);
      }
    }
};

} //namespace mio

#endif //__MIO_FREQ_BUFFER_H__

