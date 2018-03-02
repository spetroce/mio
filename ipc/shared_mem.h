#ifndef __MIO_SH_MEM_H__
#define __MIO_SH_MEM_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdint>
#include "mio/altro/error.h"

//to list and remove shared memory, use ipcs and ipcrm, respectively


namespace mio{

template <class DATA_T>
class SharedMemory{
  private:
    size_t shm_size_;
    int shm_id_;
    bool is_init_, created_;
    struct shmid_ds shm_id_ds_;

  public:
    void *shm_addr_void_;
    DATA_T *shm_addr_;

    SharedMemory(){
      is_init_ = created_ = false;
      shm_addr_void_ = nullptr;
      shm_addr_ = nullptr;
    }

    ~SharedMemory(){
      if(is_init_)
        Uninit();
    }

    //If kTryCreate is true and kMustCreate is false, Init will attempt to create the
    //shared memory segment (no error thrown if it already exists).
    //If kTryCreate and kMustCreate are true, Init will throw an error if the segment already exists in the system.
    bool Init(const key_t kShmKey, const size_t kShmSize,
              const bool kTryCreate = true, const bool kMustCreate = false){
      EXP_CHK(!is_init_, return(true))
      EXP_CHK_M(kShmKey > 0, return(false), "invalid shared memory key value")
      EXP_CHK_M(kShmSize > 0, return(false), "invalid shared memory size")

      created_ = kTryCreate;
      if(kTryCreate){
        shm_id_ = shmget(kShmKey, kShmSize, 0666 | IPC_CREAT | IPC_EXCL);
        if(shm_id_ == -1){
          if(errno == EEXIST){
            if(kMustCreate){
              std::cerr << FL_STRM << "shmget() error: Shared memory segment already exists and kMustCreate was set.\n";
              return false;
            }
            EXP_CHK_ERRNO((shm_id_ = shmget(kShmKey, kShmSize, 0666)) != -1, return(false))
            created_ = false;
          }
          else{ //some other error besides EEXIST
            std::cerr << FL_STRM << "shmget() error. " << ERRNO_STRM << std::endl;
            return false;
          }
        }
      }
      else{
        EXP_CHK_ERRNO((shm_id_ = shmget(kShmKey, kShmSize, 0666)) != -1, return(false))
      }

      shm_addr_void_ = shmat(shm_id_, NULL, 0);
      int *shmat_result = static_cast<int*>(shm_addr_void_);
      EXP_CHK_ERRNO(shmat_result != (int*)(-1), return(false))
      shm_addr_ = static_cast<DATA_T*>(shm_addr_void_);

      shm_size_ = kShmSize;
      is_init_ = true;
    }

    //NOTE: proj_id is an int, but still only the 8 least significant bits are used to generate the key
    bool Init(const char *kFileName, const int kProjId, const size_t kShmSize,
              const bool kTryCreate = true, const bool kMustCreate = false){
      key_t key;
      EXP_CHK(kProjId > 0, return(false))
      EXP_CHK_ERRNO((key = ftok(kFileName, kProjId)) != -1, return(false))
      return Init(key, kShmSize, kTryCreate, kMustCreate);
    }

    bool Init(const std::string &kFileName, const int kProjId, const size_t kShmSize,
              const bool kTryCreate = true, const bool kMustCreate = false){
      return Init(kFileName.c_str(), kProjId, kShmSize, kTryCreate, kMustCreate);
    }

    bool Uninit(){
      EXP_CHK(is_init_, return(true))
      EXP_CHK_ERRNO(shmdt(shm_addr_void_) == 0, return(false)) //detach from shared memory segment
      if(created_){
        EXP_CHK_ERRNO(shmctl(shm_id_, IPC_RMID, &shm_id_ds_) != -1, return(false))
      }
      is_init_ = created_ = false;
      shm_addr_void_ = nullptr;
      return true;
    }

    size_t GetSize(){
      return shm_size_;
    }

    bool IsInit(){
      return is_init_;
    }
};

} //namespace mio

#endif //__MIO_SH_MEM_H__

