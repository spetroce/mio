#ifndef __MIO_SEM_H__
#define __MIO_SEM_H__

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string>
#include <linux/limits.h> //NAME_MAX
#include "mio/altro/error.h"

//to remove posix semaphores and shared memory, as root, cd to /dev/shm and rm desired files.


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
        EXP_CHK(Uninit() == 0, return)
    }

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

    bool Post(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_post(sem_addr_) == 0, return(false))
      return true;
    }

    bool Wait(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_wait(sem_addr_) == 0, return(false))
      return true;
    }

    //The semaphore will be decremented if its value is greater than zero. If the value of the semaphore is zero, 
    //then sem_trywait() will return -1 and set errno to EAGAIN.
    bool TryWait(){
      EXP_CHK(is_init_, return(false))
      EXP_CHK_ERRNO(sem_trywait(sem_addr_) == 0, return(false))
      return true;
    }

    bool IsInit(){
      return is_init_;
    }
};

} //namespace mio

#endif //__MIO_SEM_H__

