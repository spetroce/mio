#ifndef __SEM_HPP__
#define __SEM_HPP__

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string>
#include "mio/altro/error.h"

//to remove posix semaphores and shared memory, as root, cd to /dev/shm and rm desired files.


class CSemaphoreServer{
  private:
    std::string m_strSemName;
    unsigned int m_initial_sem_val;
    bool m_bIsInit;

  public:
    sem_t *m_semAddr;

    CSemaphoreServer();
    CSemaphoreServer(std::string strSemName, unsigned int unIntialSemVal);
    ~CSemaphoreServer();
    int init(bool bForce = false);
    int init(std::string strSemName, unsigned int unIntialSemVal, bool bForce = false);
    int uninit();
    int post();
    int wait();
    int tryWait();
};


class CSemaphoreClient{
  private:
    std::string m_strSemName;
    bool m_bIsInit;

  public:
    sem_t *m_semAddr;

    CSemaphoreClient();
    CSemaphoreClient(std::string strSemName);
    ~CSemaphoreClient();
    int init();
    int init(std::string strSemName);
    int uninit();
    int post();
    int wait();
    int tryWait();
};

#endif //__SEM_HPP__

