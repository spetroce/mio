#include "mio/ipc/shared_mem.h"
#include "mio/ipc/sem.h"
#include <chrono>
#include <algorithm>
#include <numeric>

typedef std::chrono::high_resolution_clock std_hrc_t;


int main(int argc, char *argv[]) {
  EXP_CHK(argc == 2, return(-1))

  mio::SharedMemory<std_hrc_t::time_point> shmem;
  shmem.Init(std::string(__FILE__), 33, sizeof(std_hrc_t::time_point));

  const size_t kLargeShMemSize = 100;
  mio::SharedMemory<uint8_t> shmem_l;
  shmem_l.Init(std::string(__FILE__), 34, kLargeShMemSize);

  mio::Semaphore sem;
  sem.Init("/sem_test");

  const int kIterationCount = 10;
  if (atoi(argv[1]) == 0) {
    // Producer
    for (int i = 0;; ++i) {
      if (i == kIterationCount)
        break;
      std::cout << "press enter to post data\n";
      std::cin.ignore();
      std::fill_n(shmem_l.shm_addr_, kLargeShMemSize, i);
      shmem.shm_addr_[0] = std_hrc_t::now();
      sem.Post();
    }
  }
  else {
    // Consumer
    for (int i = 0;; ++i) {
      if (i == kIterationCount)
        break;
      sem.Wait();
      std_hrc_t::time_point hrc_now = std_hrc_t::now();
      std_hrc_t::time_point hrc_shm = shmem.shm_addr_[0];
      int sum = std::accumulate(shmem_l.shm_addr_, shmem_l.shm_addr_ + kLargeShMemSize, 0);
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(hrc_now - hrc_shm);
      std::cout << "duration: " << ns.count() << std::endl;
      std::cout << "summation: " << sum << std::endl << std::endl;
    }
  }

  shmem.Uninit();
  sem.Uninit();

  return 0;
}
