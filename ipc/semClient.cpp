/*
#include <fcntl.h>    // For O_* constants (ie. oflag definitions)
#include <sys/stat.h> // For mode constants (ie. file permission constants)
#include <semaphore.h>
sem_t *sem_open(const char *name, int oflag);
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
*/

#include "mio/ipc/sem.h"


CSemaphoreClient::CSemaphoreClient() : m_bIsInit(false) {}


CSemaphoreClient::CSemaphoreClient(std::string strSemName) : m_strSemName(strSemName), m_bIsInit(false) {}


CSemaphoreClient::~CSemaphoreClient(){
  if(m_bIsInit)
    EXP_CHK_E(uninit() == 0, return)
}


int CSemaphoreClient::init(){
  EXP_CHK_E(!m_bIsInit, return(0));
  int oflag = 0;
  //O_EXCL causes sem_open() to fail if O_CREAT flag is set and semaphore was already exists
  ERRNO_CHK_E((m_semAddr = sem_open(m_strSemName.c_str(), oflag)) != SEM_FAILED, return(-1))
  m_bIsInit = true;
  return 0;
}


int CSemaphoreClient::init(std::string strSemName){
  m_strSemName = strSemName;
  return init();
}


int CSemaphoreClient::uninit(){
  EXP_CHK_EM(m_bIsInit, return(0), "semaphore already uninitialized")
  ERRNO_CHK_E(sem_close(m_semAddr) == 0, return(-1))
  m_bIsInit = false;
  return 0;
}


int CSemaphoreClient::post(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_post(m_semAddr) == 0, return(-1))
  return 0;
}


int CSemaphoreClient::wait(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_wait(m_semAddr) == 0, return(-1))
  return 0;
}


//The semaphore will be decremented if its value is greater than zero. If the value of the semaphore is zero, 
//then sem_trywait() will return -1 and set errno to EAGAIN.
int CSemaphoreClient::tryWait(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_trywait(m_semAddr) == 0, return(-1))
  return 0;
}

