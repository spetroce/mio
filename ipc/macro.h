//YOU MUST INCLUDE BOTH sem.hpp AND shMem.hpp BEFORE INCLUDING THIS FILE

#ifndef __IPC_MACRO_H__
#define __IPC_MACRO_H__
#ifdef __SH_MEM_HPP__
#ifdef __SEM_HPP__

#define SHM_SEM_PAIR_SERVER(shm_var_name, shm_file_str, key, shm_size, \
                            sem_var_name, sem_str, sem_val, sem_force, err_str, err_exit_func) \
  /*ERR_CHKN(shm_var_name.init(shm_file_str, key, shm_size) == -1, err_str" - CSharedMemoryClient::init() error", key, err_exit_func);*/\
  shm_var_name.init(shm_file_str, key, shm_size);\
  {\
    char szMutexName[256];\
    snprintf(szMutexName, 256, "%s_%d", sem_str, key);\
    ERR_CHKN(sem_var_name.init(std::string(szMutexName), sem_val, sem_force) == -1, \
             err_str" - CSemaphoreClient::init() error", key, err_exit_func);\
  }

#define SHM_SEM_PAIR_CLIENT(shm_var_name, shm_file_str, key, shm_size, \
                            sem_var_name, sem_str, err_str, err_exit_func) \
  /*ERR_CHKN(shm_var_name.init(shm_file_str, key, shm_size) == -1, err_str" - CSharedMemoryClient::init() error", key, err_exit_func);*/\
  shm_var_name.init(shm_file_str, key, shm_size);\
  {\
    char szMutexName[256];\
    snprintf(szMutexName, 256, "%s_%d", sem_str, key);\
    ERR_CHKN(sem_var_name.init( std::string(szMutexName) ) == -1, err_str" - CSemaphoreClient::init() error", key, err_exit_func);\
  }

#define SHM_SEM_PAIR_UNINIT(shm_var_name, sem_var_name, key, err_str, err_exit_func)\
    /*ERR_CHKN(shm_var_name.uninit() == -1, err_str" - CSharedMemoryServer::uninit() error", key, err_exit_func);*/\
    shm_var_name.uninit();\
    ERR_CHKN(sem_var_name.uninit() == -1, err_str" - CSemaphoreServer::uninit() error", key, err_exit_func);

#endif //__SEM_HPP__
#endif //__SH_MEM_HPP__
#endif //__IPC_MACRO_H__

