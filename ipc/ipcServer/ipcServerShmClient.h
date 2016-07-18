#ifndef __MIO_IPC_SERVER_SHM_CLIENT__
#define __MIO_IPC_SERVER_SHM_CLIENT__

#include <functional>
#include <lcm/lcm.h>
#include "mio/lcm/lcmUtils.h"
#include "mio/ipc/shMem.hpp"
#include "lcm_types/lcm_create_shm_t.h"


namespace mio{

  inline size_t string_to_key(const std::string str){
    std::hash<std::string> hash_func;
    return hash_func(str);
  }

}


#define IPCS_CREATE_SHM_CLIENT(var_name, shm_name, shm_size)\
mio::CIpcServerShmClient var_name( shm_name, shm_size, mio::string_to_key( std::string(__FILE__) + std::to_string(__LINE__) ) );

#define IPCS_SHM_CLIENT_INIT(var_name, shm_name, shm_size)\
var_name.init( shm_name, shm_size, mio::string_to_key( std::string(__FILE__) + std::to_string(__LINE__) ) );


namespace mio{

class CIpcServerShmClient{
  lcm_t *m_lcm;
  int m_lcm_fd;
  bool m_is_init, m_init_success, m_numeric_id_match;
  size_t m_numeric_id;
  lcm_create_shm_t_subscription_t *m_response_sub;

  static void CreateShmResponse(const lcm_recv_buf_t *rbuf, const char *channel, 
                                const lcm_create_shm_t *msg, void *userdata);
  
  public:
    CSharedMemoryClient m_shm;

    CIpcServerShmClient() : m_lcm(NULL), m_response_sub(NULL){
      m_is_init = m_init_success = m_numeric_id_match = false;
    }

    CIpcServerShmClient(const std::string shm_name, const size_t shm_size, 
                        const size_t numeric_id) : m_lcm(NULL), m_response_sub(NULL){
      m_is_init = m_init_success = m_numeric_id_match = false;
      init(shm_name, shm_size, numeric_id);
    }

    void init(const std::string shm_name, const size_t shm_size, const size_t numeric_id){
      m_lcm = lcm_create(NULL);
      EXP_CHK_EM(m_lcm != NULL, return, std::string("lcm_create() error, shm_name: ") + shm_name)
      m_lcm_fd = lcm_get_fileno(m_lcm);

      m_response_sub = lcm_create_shm_t_subscribe(m_lcm, "ipcs_create_shm_response", &CreateShmResponse, this);
      lcm_create_shm_t_subscription_set_queue_capacity(m_response_sub, 3);

      lcm_create_shm_t msg;
      char *shm_name_c_str = strcpy( static_cast<char*>( malloc(shm_name.length() + 1) ), shm_name.c_str() );
      msg.shm_name = shm_name_c_str;
      msg.shm_size = shm_size;
      msg.numeric_id = m_numeric_id = numeric_id;
      lcm_create_shm_t_publish(m_lcm, "ipcs_create_shm", &msg);

      for(size_t time_out = 0;;)
        if( lcm_handle_to(m_lcm, m_lcm_fd, 1, 0) ){
          if(!m_numeric_id_match){
            printf("received a message with wrong id\n");
            continue;
          }
          EXP_CHK_EM(m_init_success, return, "critical error - IPC Server could not create shared memory segment")
          break;
        }
        else{
          time_out++;
          lcm_create_shm_t_publish(m_lcm, "ipcs_create_shm", &msg);
          EXP_CHK_EM(time_out > 2, return, "critical error - didn't receive response from IPC Server")
        }

      printf("successful init of mem segment - name: %s, size: %d\n", shm_name_c_str, shm_size);
      free(shm_name_c_str);
      m_is_init = true;
    }

    ~CIpcServerShmClient(){
      if(m_lcm != NULL){
        if( m_shm.isInit() )
          m_shm.uninit();
        lcm_create_shm_t_unsubscribe(m_lcm, m_response_sub);
        lcm_destroy(m_lcm);
      }
    }
};

} //namespace mio

#endif //__MIO_IPC_SERVER_SHM_CLIENT__

