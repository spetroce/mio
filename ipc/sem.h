#ifndef __MIO_SEM_H__
#define __MIO_SEM_H__

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string>
#include <linux/limits.h> //NAME_MAX
#include "mio/altro/error.h"

//to remove posix semaphores, as root, cd to /dev/shm and rm desired files.


namespace mio{

class Semaphore{
  private:
    std::string sem_name_;
    bool is_init_, created_;

  public:
    sem_t *sem_addr_;

    Semaphore() : is_init_(false), created_(false), sem_name_("") {}

    ~Semaphore(){
      if(is_init_)
        EXP_CHK(Uninit() == true, return)
    }

    //If kTryCreate is true and kMustCreate is false, Init will attempt
    //to create the semaphore (no error thrown if it already exists).
    //If kTryCreate and kMustCreate are true, Init will throw an error if the semaphore already exists in the system.
    bool Init(const std::string kSemName, const unsigned int kInitialSemValue = 0,
              const bool kTryCreate = true, const bool kMustCreate = false){
      EXP_CHK(!is_init_, return(true))
      EXP_CHK(kSemName.size() > 0 && kSemName.size() <= NAME_MAX-4, return(false))
      EXP_CHK(kSemName[0] == '/', return(false))

      created_ = kTryCreate;
      if(kTryCreate){
        sem_addr_ = sem_open(kSemName.c_str(), O_CREAT | O_EXCL, 0666, kInitialSemValue);
        if(sem_addr_ == SEM_FAILED){
          if(errno == EEXIST){
            if(kMustCreate){
              std::cerr << LF_STRM << "sem_open() error: Semaphore already exists and kMustCreate was set.\n";
              return false;
            }
            EXP_CHK_ERRNO((sem_addr_ = sem_open(kSemName.c_str(), 0)) != SEM_FAILED, return(false))
            created_ = false;
          }
          else{ //some other error besides EEXIST
            std::cerr << LF_STRM << "sem_open() error. " << ERRNO_STRM << std::endl;
            return false;
          }
        }
      }
      else{
        EXP_CHK_ERRNO((sem_addr_ = sem_open(kSemName.c_str(), 0)) != SEM_FAILED, return(false))
      }

      sem_name_ = kSemName;
      is_init_ = true;
      return true;
    }

    bool Init(const char *kSemName, const unsigned int kInitialSemValue = 0,
              const bool kTryCreate = true, const bool kMustCreate = false){
      return Init(std::string(kSemName), kInitialSemValue, kTryCreate, kMustCreate);
    }

    bool Uninit(){
      EXP_CHK(is_init_, return(true))
      EXP_CHK_ERRNO(sem_close(sem_addr_) == 0, return(false))
      if(created_){
        //tells OS it can remove the named semaphore once all processes using the semaphore call sem_close on it
        EXP_CHK_ERRNO(sem_unlink(sem_name_.c_str()) == 0, return(false))
      }
      sem_name_ = "";
      is_init_ = created_ = false;
      return true;
    }

    /*
    sem_post() increments (unlocks) the semaphore pointed to by sem.  If
    the semaphore's value consequently becomes greater than zero, then
    another process or thread blocked in a sem_wait(3) call will be woken
    up and proceed to lock the semaphore.
    */
    bool Post(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_post(sem_addr_) == 0, return(false))
      return true;
    }

    /*
    sem_wait() decrements (locks) the semaphore pointed to by sem.  If
    the semaphore's value is greater than zero, then the decrement
    proceeds, and the function returns, immediately.  If the semaphore
    currently has the value zero, then the call blocks until either it
    becomes possible to perform the decrement (i.e., the semaphore value
    rises above zero), or a signal handler interrupts the call.
    */
    bool Wait(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_wait(sem_addr_) == 0, return(false))
      return true;
    }

    /*
    sem_trywait() is the same as sem_wait(), except that if the decrement
    cannot be immediately performed, then call returns an error (errno
    set to EAGAIN) instead of blocking.
    */
    bool TryWait(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_trywait(sem_addr_) == 0, return(false))
      return true;
    }

    /*
    sem_timedwait() is the same as sem_wait(), except that abs_timeout
    specifies a limit on the amount of time that the call should block if
    the decrement cannot be immediately performed.  The abs_timeout
    argument points to a structure that specifies an absolute timeout in
    seconds and nanoseconds since the Epoch, 1970-01-01 00:00:00 +0000
    (UTC).  This structure is defined as follows:

       struct timespec {
           time_t tv_sec;      // Seconds
           long   tv_nsec;     // Nanoseconds [0 .. 999999999]
       };

    If the timeout has already expired by the time of the call, and the
    semaphore could not be locked immediately, then sem_timedwait() fails
    with a timeout error (errno set to ETIMEDOUT).

    If the operation can be performed immediately, then sem_timedwait()
    never fails with a timeout error, regardless of the value of
    abs_timeout.  Furthermore, the validity of abs_timeout is not checked
    in this case.
    */
    bool TimedWait(const time_t kSec, const long kNanosec){
      EXP_CHK(is_init_, return(false))
      timespec abs_timeout;
      abs_timeout.tv_sec = kSec;
      abs_timeout.tv_nsec = kNanosec;
      EXP_CHK_ERRNO(sem_timedwait(sem_addr_, &abs_timeout) == 0, return(false))
      return true;
    }

    bool IsInit(){
      return is_init_;
    }
};

} //namespace mio

#endif //__MIO_SEM_H__

