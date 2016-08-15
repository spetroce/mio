#ifndef __MIO_IPC_SERVER__
#define __MIO_IPC_SERVER__

#include <thread>
#include <unordered_map>
#include <lcm/lcm.h>
#include "mio/ipc/shMem.h"
#include "mio/lcm/lcmUtils.h"
#include "lcm_types/lcm_create_shm_t.h"
#include "lcm_types/lcm_destroy_shm_t.h"


namespace mio{

class CIpcServer{
  std::unordered_map<std::string, CSharedMemoryServer> m_shared_memory_map;
  lcm_t *m_lcm;
  int m_lcm_fd;
  bool m_exit_flag;
  std::thread m_thread;
  lcm_create_shm_t_subscription_t *m_create_sub;
  lcm_destroy_shm_t_subscription_t *m_destroy_sub;

  static void CreateShm(const lcm_recv_buf_t *rbuf, const char *channel,
                        const lcm_create_shm_t *msg, void *userdata);

  static void DestroyShm(const lcm_recv_buf_t *rbuf, const char *channel,
                         const lcm_destroy_shm_t *msg, void *userdata){
    std::string shm_name(msg->shm_name);
    EXP_CHK_E(!shm_name.empty(), return)

    CIpcServer *ipcs = static_cast<CIpcServer*>(userdata);

    auto item_it = ipcs->m_shared_memory_map.find(shm_name);
    if( item_it == ipcs->m_shared_memory_map.end() )
      printf("CIpcServer::DestroyShm() - the shared memory segment %s does not exist\n", msg->shm_name);
    else
      EXP_CHK_EM(ipcs->m_shared_memory_map.erase(shm_name) == 1, return, "serious internal error");
  }

  public:
    CIpcServer() : m_lcm(NULL), m_exit_flag(false){
      m_lcm = lcm_create(NULL);
      EXP_CHK_EM(m_lcm != NULL, return, "lcm_create() error")
      m_lcm_fd = lcm_get_fileno(m_lcm);

      m_create_sub = lcm_create_shm_t_subscribe(m_lcm, "ipcs_create_shm", &CreateShm, this);
      m_destroy_sub = lcm_destroy_shm_t_subscribe(m_lcm, "ipcs_destroy_shm", &DestroyShm, this);
      lcm_create_shm_t_subscription_set_queue_capacity(m_create_sub, 3);
      lcm_destroy_shm_t_subscription_set_queue_capacity(m_destroy_sub, 3);
    }

    ~CIpcServer(){
      m_shared_memory_map.clear();
      if(m_lcm != NULL){
        lcm_create_shm_t_unsubscribe(m_lcm, m_create_sub);
        lcm_destroy_shm_t_unsubscribe(m_lcm, m_destroy_sub);
        lcm_destroy(m_lcm);
      }
    }

    void ipcServerHandlerThread(){
      for(;;){
        if(m_exit_flag)
          break;
        if( lcm_handle_to(m_lcm, m_lcm_fd, 1, 0) );
        //else printf("ipcServerHandlerThread() - timeout\n");
      }
    }

    void start(){
      if(!m_exit_flag)
        m_thread = std::thread(&CIpcServer::ipcServerHandlerThread, this);
    }

    void stop(){
      printf("CIpcServer::stop() - exiting...\n");
      m_exit_flag = true;
      m_thread.join();
      printf("CIpcServer::stop() - thread stopped.\n");
    }
};

} //namespace mio

#endif //__MIO_IPC_SERVER__

