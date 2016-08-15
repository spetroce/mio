#ifndef __MIO_SEMAPHORE_HPP__
#define __MIO_SEMAPHORE_HPP__

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string>
#include "mio/altro/error.h"


namespace mio{

void create_semaphore(sem_t *&sem_addr, const char *sem_name, unsigned int initial_sem_val = 0){
  std::string error_str = std::string("sem_name: ") + sem_name;
  STD_INVALID_ARG_EM(strlen(sem_name) > 0 && strlen(sem_name) <= NAME_MAX-4, error_str)
  STD_INVALID_ARG_EM(sem_name[0] == '/', error_str)

  //const int default_flags = O_CREAT | O_EXCL;
  STD_SYSTEM_ERROR_EM( ( sem_addr = sem_open(sem_name, O_CREAT, 0666, initial_sem_val) ) == SEM_FAILED, error_str )
}


void create_semaphore(sem_t *&sem_addr, std::string sem_name, unsigned int initial_sem_val = 0){
  create_semaphore(sem_addr, sem_name.c_str(), initial_sem_val);
}


void destroy_semaphore(sem_t *&sem_addr, const char *sem_name, const bool system_level = false){
  std::string error_str = std::string("sem_name: ") + sem_name;
  STD_SYSTEM_ERROR_EM(sem_close(sem_addr) == -1, error_str)
  if(system_level)
    STD_SYSTEM_ERROR_EM(sem_unlink(sem_name) == -1, error_str)
}


void destroy_semaphore(sem_t *&sem_addr, std::string *sem_name, const bool system_level = false){
  destroy_semaphore(sem_addr, sem_name.c_str(), system_level);
}

} //namespace mio

#endif //__MIO_SEMAPHORE_HPP__

