#include "mio/ipc/shMem.h"
#include "mio/ipc/sem.h"
#include <thread>


int main(int argc, char *argv[]){
  EXP_CHK(argc == 2, return(-1))

  mio::SharedMemory<int> shmem;
  shmem.Init(std::string(__FILE__), 33, sizeof(int));

  if(atoi(argv[1]) == 0){
    for(;;){
      int num;
      std::cout << "enter a number\n";
      std::cin >> num;
      shmem.shm_addr_[0] = num;
      if(num == 0)
        break;
    }
  }
  else{
    for(int i = 0; i < 20; ++i){
      std::cout << "shmem value: " << shmem.shm_addr_[0] << std::endl;
      std::this_thread::sleep_for( std::chrono::milliseconds(750) );
    }
  }

  shmem.Uninit();

  return 0;
}

