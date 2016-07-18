#include "ipcServer.h"
#include "ipcServerShmClient.h"
#include "mio/altro/getch.h"


int main(int argc, char ** argv){
  if(false){
    CSharedMemoryServer shm_server;
    shm_server.init(__FILE__, 33, 1024);
    char *data_ptr = static_cast<char*>(shm_server.m_pvShMemAddr);
    snprintf(data_ptr, 1024, "[hello_world]\n");
    printf("data_ptr: [%s]\n", data_ptr);
  }

  mio::CIpcServer ipc_server;
  ipc_server.start();

  if(false){
    IPCS_CREATE_SHM_CLIENT(shm_test_1, "test_shm_name", 1024)
    IPCS_CREATE_SHM_CLIENT(shm_test_2, "test_shm_name", 1024)

    char *str_1 = static_cast<char*>(shm_test_1.m_shm.m_pvShMemAddr);
    char *str_2 = static_cast<char*>(shm_test_2.m_shm.m_pvShMemAddr);

    std::string hello_str("hello world");
    strcpy( str_1, hello_str.c_str() );
    printf("str_1: [%s]\n", str_1);
    printf("str_2: [%s]\n", str_2);
  }

  printf("press q to quit\n");
  for(;;)
    if(getch() == 'q')
      break;
    else
      printf("press q to quit\n");

  printf("exiting...\n");
  ipc_server.stop();
  return 0;
}

