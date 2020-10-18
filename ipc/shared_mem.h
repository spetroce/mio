#ifndef __MIO_SH_MEM_H__
#define __MIO_SH_MEM_H__

#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>
#include "mio/altro/error.h"

// To list and remove Sys V shared memory, use 'ipcs' and 'ipcrm -M <shm key>', respectively
// To remove posix shared memory, as root, cd to /dev/shm and rm desired files.

namespace mio{

#ifdef USE_SYS_V_SHM

template <class DATA_T>
class SharedMemory {
  private:
    size_t shm_size_;
    int shm_id_;
    bool is_init_, created_;
    struct shmid_ds shm_id_ds_;

  public:
    void *shm_addr_void_;
    DATA_T *shm_addr_;

    SharedMemory() {
      is_init_ = created_ = false;
      shm_addr_void_ = nullptr;
      shm_addr_ = nullptr;
    }

    ~SharedMemory() {
      if(is_init_)
        Uninit();
    }

    // If try_create is true and must_create is false, Init will attempt to
    // create the shared memory segment (no error thrown if it already exists).
    // If try_create and must_create are true, Init will throw an error if the
    // segment already exists in the system.
    bool Init(const key_t shm_key,
              const size_t shm_size,
              const bool try_create = true,
              const bool must_create = false,
              const int shmflg = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) {
      EXP_CHK(!is_init_, return true)
      EXP_CHK_M(shm_key > 0, return false, "invalid shared memory key value")
      EXP_CHK_M(shm_size > 0, return false, "invalid shared memory size")
      created_ = false;
      if (try_create) {
        shm_id_ = shmget(shm_key, shm_size, shmflg | IPC_CREAT | IPC_EXCL);
        if (shm_id_ == -1 && (errno == EEXIST && must_create || errno != EEXIST)) {
          std::cerr << FL_STRM << "shmget() error. " << ERRNO_STRM << std::endl;
          return false;
        }
        // It's not an error if we are trying to create and fail
        created_ = (errno != EEXIST);
      }
      if (!created_) {
        EXP_CHK_ERRNO((shm_id_ = shmget(shm_key, shm_size, shmflg)) != -1, return false)
      }
      shm_addr_void_ = shmat(shm_id_, NULL, 0);
      int *shmat_result = static_cast<int*>(shm_addr_void_);
      EXP_CHK_ERRNO(shmat_result != (int*)(-1), return false)
      shm_addr_ = reinterpret_cast<DATA_T*>(shm_addr_void_);
      shm_size_ = shm_size;
      is_init_ = true;
      return true;
    }

    // NOTE: proj_id is an int, but still only the 8 least significant bits are used to generate the key
    bool Init(const char *file_name,
              const int proj_id,
              const size_t shm_size,
              const bool try_create = true,
              const bool must_create = false,
              const int shmflg = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) {
      key_t key;
      EXP_CHK(proj_id > 0, return false)
      EXP_CHK_ERRNO((key = ftok(file_name, proj_id)) != -1, return false)
      return Init(key, shm_size, try_create, must_create, shmflg);
    }

    bool Init(const std::string &file_name,
              const int proj_id,
              const size_t shm_size,
              const bool try_create = true,
              const bool must_create = false,
              const int shmflg = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) {
      return Init(file_name.c_str(), proj_id, shm_size, try_create, must_create, shmflg);
    }

    bool Uninit() {
      EXP_CHK(is_init_, return true)
      EXP_CHK_ERRNO(shmdt(shm_addr_void_) == 0, return false) //detach from shared memory segment
      // Only the creator should mark the shm for removal
      if (created_) {
        EXP_CHK_ERRNO(shmctl(shm_id_, IPC_RMID, &shm_id_ds_) != -1, return false)
      }
      is_init_ = created_ = false;
      shm_addr_void_ = shm_addr_ = nullptr;
      shm_size_ = 0;
      return true;
    }

    size_t GetSize() {
      return shm_size_;
    }

    bool IsInit() {
      return is_init_;
    }
};

#else

template <class DATA_T>
class SharedMemory {
  private:
    size_t shm_size_;
    int shm_fd_;
    bool is_init_, created_;
    std::string shm_name_;

  public:
    void *shm_addr_void_;
    DATA_T *shm_addr_;

    SharedMemory() {
      is_init_ = created_ = false;
      shm_addr_void_ = shm_addr_ = nullptr;
    }

    ~SharedMemory() {
      if (is_init_) {
        Uninit();
      }
    }

    // If try_create is true and must_create is false, Init will attempt to
    // create the shared memory segment (no error thrown if it already exists).
    // If try_create and must_create are true, Init will throw an error if the
    // segment already exists in the system.
    bool Init(const std::string shm_name,
              const size_t shm_size,
              const bool try_create = true,
              const bool must_create = false,
              const int oflag = O_RDWR,
              const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) {
      EXP_CHK(!is_init_, return true)
      EXP_CHK_M(shm_name.size() > 0, return false, "invalid shared memory name")
      EXP_CHK_M(shm_size > 0, return false, "invalid shared memory size")
      EXP_CHK_M(shm_name[0] == '/', return false, "semaphore name must start with a /")
      created_ = false;
      if (try_create) {
        shm_fd_ = shm_open(shm_name.c_str(), O_CREAT | O_EXCL | oflag, mode);
        if (shm_fd_ == -1 && ((errno == EEXIST && must_create) || errno != EEXIST)) {
          std::cerr << FL_STRM << "shm_open() error. " << ERRNO_STRM << std::endl;
          return false;
        }
        // It's not an error if we are trying to create and fail
        created_ = (errno != EEXIST);
      }
      if (!created_) {
        EXP_CHK_ERRNO((shm_fd_ = shm_open(shm_name.c_str(), oflag, 0)) != -1, return false)
      } else {
        EXP_CHK_ERRNO(ftruncate(shm_fd_, shm_size) != -1, return false)
      }
      EXP_CHK_ERRNO((shm_addr_void_ = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0)) != MAP_FAILED, return false)
      shm_addr_ = reinterpret_cast<DATA_T*>(shm_addr_void_);
      shm_size_ = shm_size;
      is_init_ = true;
      shm_name_ = shm_name;
      return true;
    }

    bool Uninit() {
      // Only the creator should mark the shm for removal
      if (created_) {
        EXP_CHK_ERRNO(shm_unlink(shm_name_.c_str()) != -1, return false)
      }
      is_init_ = created_ = false;
      shm_addr_void_ = shm_addr_ = nullptr;
      shm_size_ = 0;
      shm_name_ = "";
      return true;
    }

    size_t GetSize() {
      return shm_size_;
    }

    bool IsInit() {
      return is_init_;
    }
};

#endif

} //namespace mio

#endif //__MIO_SH_MEM_H__

