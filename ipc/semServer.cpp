/*
#include <fcntl.h>    // For O_* constants (ie. oflag definitions)
#include <sys/stat.h> // For mode constants (ie. file permission constants)
#include <semaphore.h>
sem_t *sem_open(const char *name, int oflag);
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
*/

#include "mio/ipc/sem.h"


CSemaphoreServer::CSemaphoreServer() : m_bIsInit(false) {}


CSemaphoreServer::CSemaphoreServer(std::string strSemName, unsigned int initial_sem_val) : 
  m_strSemName(strSemName), m_initial_sem_val(initial_sem_val), m_bIsInit(false) {}


CSemaphoreServer::~CSemaphoreServer(){
  if(m_bIsInit)
    EXP_CHK_E(uninit() == 0, return)
}


int CSemaphoreServer::init(bool bForce){
  EXP_CHK_EM(!m_bIsInit, return(0), "semaphore already initialized")
  int oflag = O_CREAT | O_EXCL;
  //O_EXCL causes sem_open() to fail if O_CREAT flag is set and semaphore was already exists
  if( ( m_semAddr = sem_open(m_strSemName.c_str(), oflag, 0666, m_initial_sem_val) ) == SEM_FAILED ){
    if( (errno == EEXIST) && bForce ){
      oflag = 0;
      printf("%s - semaphore exists, second attempt: open it\n", CURRENT_FUNC);
      ERRNO_CHK_EM((m_semAddr = sem_open(m_strSemName.c_str(), oflag)) != SEM_FAILED, return(-1),
                   "error on second attempt")
    }
    else{
      ERRNO_CHK_EM(m_semAddr != SEM_FAILED, return(-1), "sem_open() error")
    }
  }
  m_bIsInit = true;
  return 0;
}


int CSemaphoreServer::init(std::string strSemName, unsigned int initial_sem_val, bool bForce){
  m_strSemName = strSemName;
  m_initial_sem_val = initial_sem_val;
  return init(bForce);
}


//sem_unlink() - tells OS it can remove the named semaphore once all processes using the semaphore call sem_close on it
int CSemaphoreServer::uninit(){
  EXP_CHK_EM(m_bIsInit, return(0), "semaphore already uninitialized")
  ERRNO_CHK_E(sem_close(m_semAddr) == 0, return(-1))
  ERRNO_CHK_E(sem_unlink( m_strSemName.c_str() ) == 0, return(-1))
  m_bIsInit = false;
  return 0;
}


int CSemaphoreServer::post(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_post(m_semAddr) == 0, return(-1))
  return 0;
}


int CSemaphoreServer::wait(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_wait(m_semAddr) == 0, return(-1))
  return 0;
}

//The semaphore will be decremented if its value is greater than zero. If the value of the semaphore is zero, 
//then sem_trywait() will return -1 and set errno to EAGAIN.
int CSemaphoreServer::tryWait(){
  EXP_CHK_E(m_bIsInit, return(-1))
  ERRNO_CHK_E(sem_trywait(m_semAddr) == 0, return(-1))
  return 0;
}

