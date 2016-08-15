#ifndef __SH_MEM_HPP__
#define __SH_MEM_HPP__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdint>
#include "mio/altro/error.h"

//to remove posix semaphores and shared memory, as root, cd to /dev/shm and rm desired files.


class CSharedMemoryServer{
  private:
    key_t m_shm_key;
    size_t m_shm_size;
    int m_shm_id;
    bool m_is_init;
    struct shmid_ds m_shm_id_struct;

  public:
    void *m_pvShMemAddr;

    CSharedMemoryServer();
    ~CSharedMemoryServer();
    void init(const key_t shm_key, const size_t shm_size);
    void init(const char *file_name_full, const int proj_id, const size_t shm_size);
    void init(const std::string &file_name_full, const int proj_id, const size_t shm_size);
    void uninit();

    size_t getSize(){
      return m_shm_size;
    }

    bool isInit(){
      return m_is_init;
    }
};


class CSharedMemoryClient{
  private:
    key_t m_shm_key;
    size_t m_shm_size;
    int m_shm_id;
    bool m_is_init;

  public:
    void *m_pvShMemAddr;

    CSharedMemoryClient();
    ~CSharedMemoryClient();
    void init(const key_t shm_key, const size_t shm_size);
    void init(const char *file_name_full, const int proj_id, const size_t shm_size);
    void init(const std::string &file_name_full, const int proj_id, const size_t shm_size);
    void uninit();

    size_t getSize(){
      return m_shm_size;
    }

    bool isInit(){
      return m_is_init;
    }
};

#endif //__SH_MEM_HPP__

