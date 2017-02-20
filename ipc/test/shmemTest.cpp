#include "mio/ipc/shMem.h"
#include "mio/ipc/sem.h"


int main(int argc, char *argv[]){
  EXP_CHK(argc == 2, return(-1))

  mio::SharedMemory<int> shmem;
  shmem.Init(std::string(__FILE__), 33, sizeof(int));

  if(atoi(argv[1]) == 0){
    mio::Semaphore sem;
    sem.Init("/sem_test", 0); //initial semaphore value is 1 (server side)
    for(;;){
      int num;
      std::cout << "enter a number\n";
      std::cin >> num;
      shmem.shm_addr_[0] = num;
      sem.Post(); //increment semaphore
      if(num == 0)
        break;
    }
    sem.Uninit();
  }
  else{
    mio::Semaphore sem;
    sem.Init("/sem_test", 0, false); //don't try to create semaphore (client side)
    for(;;){
      sem.Wait(); //decrement semaphore
      int num = shmem.shm_addr_[0];
      std::cout << "shmem value: " << num << std::endl;
      if(num == 0)
        break;
    }
    sem.Uninit();
  }

  shmem.Uninit();

  return 0;
}

