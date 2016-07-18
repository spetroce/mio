#ifndef __MIO_SOCKET_H__
#define __MIO_SOCKET_H__

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h> //gethostbyname()
#include <netinet/in.h> //INET_ADDRSTRLEN
#include <thread>
#include "mio/altro/error.h"


namespace mio{

//For coonect mode sockets (eg. TCP sockets), dest_addr and dest_addr_len are ignored, therefore, not needed
inline int SendTo(const int sock_fd, const void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                  const int flags = 0, const struct sockaddr *dest_addr = NULL, socklen_t dest_addr_len = 0,
                  const unsigned int timeout_len_sec = 2, const unsigned int num_timeout_limit = 3,
                  const bool suppress_timeout_error = false){
  EXP_CHK_E(sock_fd > 0, return(-1))
  EXP_CHK_E(data_buf != nullptr, return(-1))
  EXP_CHK_E(data_buf_len > 0, return(0))
            
  int num_byte_sent, num_active_fd;
  unsigned int num_timeout = 0, total_num_byte_sent = 0;
  fd_set writefds;
  struct timeval tv, tv_temp;
  FD_ZERO(&writefds);
  FD_SET(sock_fd, &writefds);
  tv.tv_sec = timeout_len_sec;
  tv.tv_usec = 0;

  while(data_buf_len > total_num_byte_sent){
    do{
      tv_temp = tv; // Select may be modifying tv_temp so reset
      num_active_fd = select(sock_fd + 1, NULL, &writefds, NULL, &tv_temp); //getdtablesize()
      ERRNO_CHK_E(num_active_fd != -1, return(-1));
      if(num_active_fd == 0){
        num_timeout++;
        if(!suppress_timeout_error){
          printf("%s - timeout occurence %d\n", CURRENT_FUNC, num_timeout);
          EXP_CHK_E(num_timeout < num_timeout_limit, return(-3));
        }
        else if(num_timeout >= num_timeout_limit)
          return 0;
      }
      else{
        const size_t num_bytes_to_send = (packet_size == 0) ? (data_buf_len-total_num_byte_sent) :
                                                              std::min(data_buf_len-total_num_byte_sent, packet_size);
        ERRNO_CHK_E(( num_byte_sent = sendto(sock_fd, (uint8_t *)data_buf + total_num_byte_sent, 
                    num_bytes_to_send, flags, dest_addr, dest_addr_len) ) != -1, return(-1))
        EXP_CHK_EM(!(num_byte_sent == 0 && num_active_fd == 1), return(-1), "connection was lost");
        //printf("sent out %d bytes\n", num_byte_sent);
      }
    } while(num_active_fd == 0); // Loop for timeout check instances
    total_num_byte_sent += num_byte_sent;
  }

  return total_num_byte_sent;
}


//src_addr and src_addr_len, when provided, are filled in by recvfrom()
inline int RecvFrom(const int sock_fd, void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                    const int flags = 0, struct sockaddr *src_addr = NULL, socklen_t *src_addr_len = NULL,
                    const unsigned int timeout_len_sec = 2, const unsigned int num_timeout_limit = 3,
                    const bool suppress_timeout_error = false){
  EXP_CHK_E(sock_fd > 0, return(-1))
  EXP_CHK_E(data_buf != nullptr, return(-1))
  EXP_CHK_E(data_buf_len > 0, return(0))
  
  int num_byte_recv, num_active_fd;
  unsigned int num_timeout = 0, total_num_byte_recv = 0;
  fd_set readfds;
  struct timeval tv, tv_temp;
  FD_ZERO(&readfds);
  FD_SET(sock_fd, &readfds);
  tv.tv_sec = timeout_len_sec;
  tv.tv_usec = 0;
  
  while(data_buf_len > total_num_byte_recv){
    do{
      tv_temp = tv; //select may be modifying tv_temp so reset
      num_active_fd = select(sock_fd + 1, &readfds, NULL, NULL, &tv_temp); //getdtablesize()
      ERRNO_CHK_E(num_active_fd != -1, return(-1));
      if(num_active_fd == 0){
        num_timeout++;
        if(!suppress_timeout_error){
          printf("%s - timeout occurence %d\n", CURRENT_FUNC, num_timeout);
          EXP_CHK_E(num_timeout < num_timeout_limit, return(-3));
        }
        else if(num_timeout >= num_timeout_limit)
          return 0;
      }
      else{
        const size_t num_bytes_to_get = (packet_size == 0) ? (data_buf_len-total_num_byte_recv) :
                                                             std::min(data_buf_len-total_num_byte_recv, packet_size);
        ERRNO_CHK_E(( num_byte_recv = recvfrom(sock_fd, (uint8_t *)data_buf + total_num_byte_recv, 
                    num_bytes_to_get, flags, src_addr, src_addr_len) ) != -1, return(-1))
        EXP_CHK_EM(!(num_byte_recv == 0 && num_active_fd == 1), return(-1), "connection was lost");
        //printf("read in %d bytes\n", num_byte_recv);
      }
    } while(num_active_fd == 0); //loop for timeout check instances
    total_num_byte_recv += num_byte_recv;
  }
  
  return num_byte_recv;
}


// get sockaddr, IPv4 or IPv6:
inline void *GetAddrIn(struct sockaddr *sa){
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

inline void *GetAddrIn(struct addrinfo *ai){
  return GetAddrIn(ai->ai_addr);
}


inline uint32_t ntohs_port(struct sockaddr *sa){
	if(sa->sa_family == AF_INET)
    return static_cast<uint32_t>(ntohs(((struct sockaddr_in *)sa)->sin_port));
  return static_cast<uint32_t>(ntohs(((struct sockaddr_in6 *)sa)->sin6_port));
}

inline uint32_t ntohs_port(struct addrinfo *ai){
  return ntohs_port(ai->ai_addr);
}


inline const char *inet_ntop_sin(struct sockaddr *sa, char *dst, socklen_t size){
  return inet_ntop(sa->sa_family, GetAddrIn(sa), dst, size);
}

inline const char *inet_ntop_sin(struct sockaddr_storage *ss, char *dst, socklen_t size){
  return inet_ntop(ss->ss_family, GetAddrIn((struct sockaddr *)ss), dst, size);
}

inline const char *inet_ntop_sin(struct addrinfo *ai, char *dst, socklen_t size){
  return inet_ntop_sin(ai->ai_addr, dst, size);
}


// Print info from 'struct addrinfo' that is filled by getaddrinfo()
inline void PrintAddrInfo(struct addrinfo *addr_info){
  char cstr[INET6_ADDRSTRLEN];
  switch(addr_info->ai_family){
    case AF_INET:
      inet_ntop_sin(addr_info, cstr, sizeof cstr);
      printf("ai_family=AF_INET\nIPv4=%s\nsin_port=%d\n", cstr, ntohs_port(addr_info));
      break;
    case AF_INET6:
      inet_ntop_sin(addr_info, cstr, sizeof cstr);
      printf("ai_family=AF_INET6\nIPv6=%s\nsin_port=%d\n", cstr, ntohs_port(addr_info));
      break;
    default:
      printf("%s - invalid ai_socktype\n", CURRENT_FUNC);
      break;
  }
  switch(addr_info->ai_socktype){
    case SOCK_STREAM:
      printf("ai_socktype=SOCK_STREAM\n");
      break;
    case SOCK_DGRAM:
      printf("ai_socktype=SOCK_DGRAM\n");
      break;
    default:
      printf("%s - invalid ai_socktype\n", CURRENT_FUNC);
      break;
  }
  printf("\n");
}


class CServerTCP{
  struct addrinfo *result_, *p_; //addrinfo contains struct sockaddr *ai_addr and socklen_t ai_addrlen
  int sock_fd_, accept_sock_fd_;
  struct sockaddr_storage client_addr_;
  bool is_init_;

  public:
    CServerTCP() : is_init_(false), sock_fd_(0), accept_sock_fd_(0) {}

    ~CServerTCP(){
      if(is_init_)
        Uninit();
    }

    // interface_ip_addr_str is optional, port_num_str must be provided
    int Init(const std::string interface_ip_addr_str, const std::string port_num_str){
      EXP_CHK_E(!is_init_, return(0))
	    struct addrinfo hints;
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = interface_ip_addr_str.empty() ? AI_PASSIVE : 0; // AI_PASSIVE = Fill in IP for me
      int rv;
      EXP_CHK_EM((rv = getaddrinfo(interface_ip_addr_str.empty() ? NULL : interface_ip_addr_str.c_str(),
                                   port_num_str.c_str(), &hints, &result_)) == 0, return(-1), gai_strerror(rv))

	    // loop through all the results and bind to the first we can
	    for(p_ = result_; p_ != NULL; p_ = p_->ai_next) {
        // Create an endpoint for communication; get socket fd.
        ERRNO_CHK_E((sock_fd_ = socket(p_->ai_family, p_->ai_socktype, p_->ai_protocol)) != -1, continue)
        // Set some options
	      int yes = 1;
        ERRNO_CHK_E(setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != -1, return(-1))
        // Assign the address p_->ai_addr to the socket referred to by sock_fd_
        ERRNO_CHK_E(bind(sock_fd_, p_->ai_addr, p_->ai_addrlen) != -1, continue)
		    break;
	    }
      EXP_CHK_EM(p_ != NULL, return(-1), "failed to bind socket");

      is_init_ = true;
      return 0;
    }

    int ListenAccept(const int backlog = 10){
      EXP_CHK_E(is_init_, return(-1))
      printf("%s - listening...\n", CURRENT_FUNC);
      // Mark sock_fd as a passive socket (ie. a socket that will be used to accept incoming connection requests)
      ERRNO_CHK_E(listen(sock_fd_, backlog) != -1, return(-1))
      socklen_t client_addr_len = sizeof client_addr_;
      // Accept an incoming connection
      ERRNO_CHK_E((accept_sock_fd_ = accept(sock_fd_, (struct sockaddr *)&client_addr_,
                                            &client_addr_len)) != -1, return(-1))
	    char cstr[INET6_ADDRSTRLEN];
      inet_ntop_sin(&client_addr_, cstr, sizeof cstr);
      printf("%s - got connection from: %s\n", CURRENT_FUNC, cstr);
      return 0;
    }

    int Uninit(){
      EXP_CHK_E(is_init_, return(0))
      printf("%s: closing TCP socket...\n", CURRENT_FUNC);
      if(sock_fd_ > 0)
        ERRNO_CHK_E(close(sock_fd_) != -1, return(-1))
      if(accept_sock_fd_ > 0)
        ERRNO_CHK_E(close(accept_sock_fd_) != -1, return(-1))
      printf("%s: TCP socket closed.\n", CURRENT_FUNC);
      freeaddrinfo(result_);
      is_init_ = false;
      return 0;
    }

    int accept_sock_fd(){
      EXP_CHK_E(is_init_, return(-1))
      return accept_sock_fd_;
    }

    int SendToClient(const void *data_buf, const size_t data_buf_len, const int flags = 0){
      return send(accept_sock_fd_, (uint8_t*)data_buf, data_buf_len, flags);
    }

    int RecvFromClient(const void *data_buf, const size_t data_buf_len, const int flags = 0){
      return recv(accept_sock_fd_, (uint8_t*)data_buf, data_buf_len, flags);
    }

    int SendToClientAdv(const void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                        const int flags = 0, const unsigned int timeout_len_sec = 2,
                        const unsigned int num_timeout_limit = 3, const bool suppress_timeout_error = false){
      return mio::SendTo(accept_sock_fd_, data_buf, data_buf_len, packet_size, flags, NULL, 0,
                         timeout_len_sec, num_timeout_limit, suppress_timeout_error);
    }

    int RecvFromClientAdv(void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                          const int flags = 0, const unsigned int timeout_len_sec = 2,
                          const unsigned int num_timeout_limit = 3, const bool suppress_timeout_error = false){
      return mio::RecvFrom(accept_sock_fd_, data_buf, data_buf_len, packet_size, flags, NULL, NULL,
                           timeout_len_sec, num_timeout_limit, suppress_timeout_error);
    }
};


class CClientTCP{
  int sock_fd_;
  struct addrinfo *result_, *p_; //addrinfo contains struct sockaddr *ai_addr and socklen_t ai_addrlen
  bool is_init_;

  public:
    CClientTCP() : is_init_(false), sock_fd_(0) {}

    ~CClientTCP(){
      if(is_init_)
        Uninit();
    }

    // server_ip_str and port_num_str must be provided
    int Init(const std::string server_ip_str, const std::string port_num_str){
      EXP_CHK_E(!is_init_, return(0))
	    struct addrinfo hints;
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
      int rv;
      EXP_CHK_EM((rv = getaddrinfo(server_ip_str.c_str(), port_num_str.c_str(), &hints, &result_)) == 0,
                 return(-1), gai_strerror(rv))

	    // loop through all the results and connect to the first we can
	    for(p_ = result_; p_ != NULL; p_ = p_->ai_next) {
        // Create an endpoint for communication; get socket fd.
        ERRNO_CHK_E((sock_fd_ = socket(p_->ai_family, p_->ai_socktype, p_->ai_protocol)) != -1, continue)
        // Connect the socket referred to by sock_fd to the address specified by p->ai_addr
        ERRNO_CHK_E(connect(sock_fd_, p_->ai_addr, p_->ai_addrlen) != -1, continue)
		    break;
	    }
      EXP_CHK_EM(p_ != NULL, return(-1), "failed to connect socket");

	    char cstr[INET6_ADDRSTRLEN];
      inet_ntop_sin(p_, cstr, sizeof cstr);
	    printf("%s: connected to %s\n", CURRENT_FUNC, cstr);

      is_init_ = true;
	    return 0;
    }

    int Uninit(){
      EXP_CHK_E(is_init_, return(0))
      printf("%s: closing TCP socket...\n", CURRENT_FUNC);
      if(sock_fd_ > 0)
        ERRNO_CHK_E(close(sock_fd_) != -1, return(-1))
      printf("%s: TCP socket closed.\n", CURRENT_FUNC);
      freeaddrinfo(result_);
      is_init_ = false;
      return 0;
    }

    int server_sock_fd(){
      EXP_CHK_E(is_init_, return(-1))
      return sock_fd_;
    }

    struct sockaddr* server_ai_addr(){ // not necessary for com.
      EXP_CHK_E(is_init_, return(NULL))
      return p_->ai_addr;
    }

    socklen_t server_ai_addrlen(){ // not necessary for com.
      EXP_CHK_E(is_init_, return(0))
      return p_->ai_addrlen;
    }

    int SendToServer(const void *data_buf, const size_t data_buf_len, const int flags = 0){
      return send(sock_fd_, (uint8_t*)data_buf, data_buf_len, flags);
    }

    int RecvFromServer(const void *data_buf, const size_t data_buf_len, const int flags = 0){
      return recv(sock_fd_, (uint8_t*)data_buf, data_buf_len, flags);
    }
};


/*
  port_num - If you're a client and don't need a well-known port that others can use to locate you (since they will
  only respond to your messages), you can just let the operating system pick any available port number by specifying
  port 0. If you're a server, you'll generally pick a specific number since clients will need to know a port number
  to which to address messages.

  interface_ip_addr_str - This is just your machine's IP address. With IP, your machine will have one IP address for
  each network interface. Most of the time, we don't care to specify a specific interface and can let the operating
  system use whatever it wants. The special address for this is 0.0.0.0, defined by the symbolic constant INADDR_ANY.
*/

class CServerUDP{ // "listener"
	int sock_fd_;
	struct addrinfo *result_, *p_;
  bool is_init_;

  public:
    CServerUDP() : is_init_(false), sock_fd_(0) {}

    ~CServerUDP(){
      if(is_init_)
        Uninit();
    }

    int Init(const std::string interface_ip_addr_str, const std::string port_num_str){
	  struct addrinfo hints;
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	    hints.ai_socktype = SOCK_DGRAM;
	    hints.ai_flags = AI_PASSIVE; // use my IP
      int rv;
      EXP_CHK_EM((rv = getaddrinfo(interface_ip_addr_str.empty() ? NULL : interface_ip_addr_str.c_str(),
                                   port_num_str.c_str(), &hints, &result_)) == 0, return(-1), gai_strerror(rv))

	    // Loop through all the results and bind to the first we can
	    for(p_ = result_; p_ != NULL; p_ = p_->ai_next){
        // Create an endpoint for communication; get socket fd.
        ERRNO_CHK_E((sock_fd_ = socket(p_->ai_family, p_->ai_socktype, p_->ai_protocol)) != -1, continue)
        // Assign the address p_->ai_addr to the socket referred to by sock_fd_
        ERRNO_CHK_E(bind(sock_fd_, p_->ai_addr, p_->ai_addrlen) != -1, continue)
		    break;
	    }
      EXP_CHK_EM(p_ != NULL, return(-1), "failed to bind socket");

      is_init_ = true;
      return 0;
    }

    int Uninit(){
      EXP_CHK_E(is_init_, return(0))
      printf("%s: closing UDP socket...\n", CURRENT_FUNC);
      if(sock_fd_ > 0)
        ERRNO_CHK_E(close(sock_fd_) != -1, return(-1))
      printf("%s: UDP socket closed.\n", CURRENT_FUNC);
      freeaddrinfo(result_);
      is_init_ = false;
      return 0;
    }

    int interface_sock_fd(){
      EXP_CHK_E(is_init_, return(-1))
      return sock_fd_;
    }

    // client_addr and addr_len get filled in
    int RecvFromClient(void *data_buf, const size_t data_buf_len, const int flags = 0,
                       struct sockaddr_storage *client_addr = NULL, socklen_t *addr_len = NULL){
      EXP_CHK_E(is_init_, return(-1))
      int num_byte;
      ERRNO_CHK_E((num_byte = recvfrom(sock_fd_, data_buf, data_buf_len, flags,
                  (struct sockaddr *)client_addr, addr_len)) != -1, return(-1));
      return num_byte;
    }

    int RecvFromClientAdv(void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                          const int flags = 0, struct sockaddr *src_addr = NULL, socklen_t *src_addr_len = NULL,
                          const unsigned int timeout_len_sec = 2, const unsigned int num_timeout_limit = 3,
                          const bool suppress_timeout_error = false){
      EXP_CHK_E(is_init_, return(-1))
      return mio::RecvFrom(sock_fd_, data_buf, data_buf_len, packet_size, flags, src_addr, src_addr_len,
                           timeout_len_sec, num_timeout_limit, suppress_timeout_error);
    }
};


class CClientUDP{ // "talker"
	int sock_fd_;
	struct addrinfo *result_, *p_;
  bool is_init_;

  public:
    CClientUDP() : is_init_(false), sock_fd_(0) {}

    ~CClientUDP(){
      if(is_init_)
        Uninit();
    }

    // server_ip_str and server_port_num must be provided
    int Init(const std::string server_ip_str, const std::string server_port_num){
	    struct addrinfo hints;
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_DGRAM;
      int rv;
      EXP_CHK_EM((rv = getaddrinfo(server_ip_str.c_str(), server_port_num.c_str(), &hints, &result_)) == 0,
                  return(-1), gai_strerror(rv))

	    // loop through all the results and make a socket
	    for(p_ = result_; p_ != NULL; p_ = p_->ai_next) {
          // Create an endpoint for communication; get socket fd.
          ERRNO_CHK_E((sock_fd_ = socket(p_->ai_family, p_->ai_socktype, p_->ai_protocol)) != -1, continue)
		    break;
	    }
      EXP_CHK_EM(p_ != NULL, return(-1), "failed to bind socket");

	    char cstr[INET6_ADDRSTRLEN];
      inet_ntop_sin(p_, cstr, sizeof cstr);
	    printf("%s: connected to %s\n", CURRENT_FUNC, cstr);

      is_init_ = true;
      return 0;
    }

    int Uninit(){
      EXP_CHK_E(is_init_, return(0))
      printf("%s: closing UDP socket...\n", CURRENT_FUNC);
      if(sock_fd_ > 0)
        ERRNO_CHK_E(close(sock_fd_) != -1, return(-1))
      printf("%s: UDP socket closed.\n", CURRENT_FUNC);
      freeaddrinfo(result_);
      is_init_ = false;
      return 0;
    }

    int server_sock_fd(){
      EXP_CHK_E(is_init_, return(-1))
      return sock_fd_;
    }

    struct sockaddr* server_ai_addr(){
      EXP_CHK_E(is_init_, return(NULL))
      return p_->ai_addr;
    }

    socklen_t server_ai_addrlen(){
      EXP_CHK_E(is_init_, return(0))
      return p_->ai_addrlen;
    }

    int SendToServer(const void *data_buf, const size_t data_buf_len, const int flags = 0,
                     const unsigned int timeout_len_sec = 2, const unsigned int num_timeout_limit = 3,
                     const bool suppress_timeout_error = false){
      EXP_CHK_E(is_init_, return(-1))
      return SendTo(sock_fd_, data_buf, data_buf_len, 0, flags, p_->ai_addr, p_->ai_addrlen,
                    timeout_len_sec, num_timeout_limit, suppress_timeout_error);
    }

    int SendToServerAdv(const void *data_buf, const size_t data_buf_len, const size_t packet_size = 0,
                        const int flags = 0, const unsigned int timeout_len_sec = 2,
                        const unsigned int num_timeout_limit = 3, const bool suppress_timeout_error = false){
      EXP_CHK_E(is_init_, return(-1))
      return mio::SendTo(sock_fd_, data_buf, data_buf_len, packet_size, flags, p_->ai_addr, p_->ai_addrlen,
                         timeout_len_sec, num_timeout_limit, suppress_timeout_error);
    }
};


class TCPHandlerThread{
  public:
    mio::CClientTCP *tcp_sock_;
    uint8_t *tcp_data_;
    size_t tcp_data_len_;
    bool exit_thread_, started_;
    std::thread thread_;

    TCPHandlerThread() : exit_thread_(false), started_(false){}

    void Thread(){
      for(;;){
        if(exit_thread_)
          break;
        uint32_t number;
        EXP_CHK_E(tcp_sock_->RecvFromServer(&number, sizeof(uint32_t), 0) == -1, break);
        printf("received number %d\n", number);
      }
    }

    void Start(){
      EXP_CHK_E(started_ == false, return)
      exit_thread_ = false;
      thread_ = std::thread(&TCPHandlerThread::Thread, this);
      printf("%s - started\n", CURRENT_FUNC);
      started_ = true;
    }

    void Stop(){
      EXP_CHK_E(started_ == true, return)
      exit_thread_ = true;
      thread_.join();
      printf("%s - stopped\n", CURRENT_FUNC);
      started_ = false;
    }
};

}

#endif //__MIO_SOCKET_H__

