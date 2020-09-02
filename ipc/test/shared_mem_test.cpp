// #define USE_SYS_V_SHM

#include "mio/ipc/shared_mem.h"
#include "mio/ipc/sem.h"

int main(int argc, char *argv[]){
  EXP_CHK(argc == 2, return(-1))

  mio::SharedMemory<int> shmem;
#ifdef USE_SYS_V_SHM
  EXP_CHK(shmem.Init(std::string(__FILE__), 33, sizeof(int)), return -1);
#else
  EXP_CHK(shmem.Init("/shm_test", sizeof(int)), return -1);
#endif

  mio::Semaphore sem;
  EXP_CHK(sem.Init("/sem_test"), return -1);

  if (atoi(argv[1]) == 0) {
    // Producer
    for (;;) {
      int num;
      std::cout << "enter a number\n";
      std::cin >> num;
      shmem.shm_addr_[0] = num;
      sem.Post(); //increment semaphore
      if (num == 0)
        break;
    }
  }
  else {
    // Consumer
    for (;;) {
      sem.Wait(); //decrement semaphore
      int num = shmem.shm_addr_[0];
      std::cout << "shmem value: " << num << std::endl;
      if (num == 0)
        break;
    }
  }

  shmem.Uninit();
  sem.Uninit();

  return 0;
}
