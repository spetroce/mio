#include "mio/ipc/shMem.h"


CSharedMemoryClient::CSharedMemoryClient(){
  m_is_init = false;
  m_pvShMemAddr = NULL;
}


CSharedMemoryClient::~CSharedMemoryClient(){
  if(m_is_init)
    uninit();
}


void CSharedMemoryClient::init(const key_t shm_key, const size_t shm_size){
  EXP_CHK_EM(!m_is_init, return, "shared memory segment already initialized")
  STD_INVALID_ARG_EM(shm_key > 0, "invalied shared memory key value")
  STD_INVALID_ARG_EM(shm_size > 0, "invalid shared memory size")
  m_shm_key = shm_key;
  m_shm_size = shm_size;

  int shm_flag = 0666;
  int shm_id = shmget(m_shm_key, m_shm_size, shm_flag);
  STD_SYSTEM_ERROR_E(shm_id != -1)

  int8_t *shm_addr = static_cast<int8_t*>( shmat(shm_id, NULL, 0) );
  STD_SYSTEM_ERROR_E( shm_addr != (int8_t*)(-1) )

  m_shm_id = shm_id;
  m_pvShMemAddr = static_cast<void*>(shm_addr);
  m_is_init = true;
}


void CSharedMemoryClient::init(const char *file_name_full, const int proj_id, const size_t shm_size){
  key_t key;
  //STD_INVALID_ARG_E(proj_id != 0)
  STD_SYSTEM_ERROR_E( ( key = ftok(file_name_full, proj_id) ) != -1 )
  CSharedMemoryClient::init(key, shm_size);
}

void CSharedMemoryClient::init(const std::string &file_name_full, const int proj_id, const size_t shm_size){
  CSharedMemoryClient::init(file_name_full.c_str(), proj_id, shm_size);
}


void CSharedMemoryClient::uninit(){
  EXP_CHK_EM(m_is_init, return, "shared memory segment already uninitialized");
  STD_SYSTEM_ERROR_E(shmdt(m_pvShMemAddr) == 0); //detach from shared memory segment

  m_is_init = false;
  m_pvShMemAddr = NULL;
}

