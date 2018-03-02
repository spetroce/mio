#include "ipc_server.h"
#include "ipc_server_shm_client.h"


namespace mio{

void CIpcServer::CreateShm(const lcm_recv_buf_t *rbuf, const char *channel,
                           const lcm_create_shm_t *msg, void *userdata){
  std::string shm_name(msg->shm_name);
  EXP_CHK(!shm_name.empty(), return)
  EXP_CHK_M(msg->shm_size > 0, return, std::string("shm_name: ") + shm_name);
  printf("request to create shared memory segment %s with size %d\n", msg->shm_name, msg->shm_size);

  CIpcServer *ipcs = static_cast<CIpcServer*>(userdata);

  auto item_it = ipcs->m_shared_memory_map.find(shm_name);
  if( item_it == ipcs->m_shared_memory_map.end() ){
    ipcs->m_shared_memory_map.insert( std::pair<std::string, CSharedMemoryServer>( shm_name, CSharedMemoryServer() ) );
    CSharedMemoryServer &shm = ipcs->m_shared_memory_map.at(shm_name);
    shm.init(__FILE__, string_to_key(shm_name), msg->shm_size);
  }
  else{
    printf("CIpcServer::CreateShm() - a shared memory segment with this name exists.\n");
    const size_t existing_shm_size = ipcs->m_shared_memory_map[shm_name].getSize();
    std::string error_str = std::string("a memory segment with the name ") + shm_name +
                            "exists, but with a different size. Existing size: " + std::to_string(existing_shm_size) +
                            ", Requested size: " + std::to_string(msg->shm_size);
    EXP_CHK_M(existing_shm_size == msg->shm_size, return, error_str)
  }

  lcm_create_shm_t response_msg;
  response_msg.shm_name = msg->shm_name;
  response_msg.shm_size = msg->shm_size;
  response_msg.numeric_id = msg->numeric_id;
  lcm_create_shm_t_publish(ipcs->m_lcm, "ipcs_create_shm_response", &response_msg);
}


void CIpcServerShmClient::CreateShmResponse(const lcm_recv_buf_t *rbuf, const char *channel,
                                            const lcm_create_shm_t *msg, void *userdata){
  std::string shm_name(msg->shm_name);
  EXP_CHK(!shm_name.empty(), return)
  std::hash<std::string> hash_func;
  size_t name_hash_key = hash_func(shm_name);
  CIpcServerShmClient *shm_client = static_cast<CIpcServerShmClient*>(userdata);

  if(msg->numeric_id == shm_client->m_numeric_id){
    shm_client->m_numeric_id_match = true;
    shm_client->m_shm.init(__FILE__, string_to_key(shm_name), msg->shm_size); //TODO - error check
    shm_client->m_init_success = true;
  }
}

} //namespace mio

