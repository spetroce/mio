#include "mio/socket/socket.h"


int main(int argc, char *argv[]){
  mio::CServerUDP sock_udp;
  sock_udp.Init("", "3495");

  uint8_t number;
	struct sockaddr_storage client_addr;
	socklen_t addr_len = sizeof client_addr;
  sock_udp.RecvFromClient(&number, sizeof(uint8_t), 0, &client_addr, &addr_len);
  char cstr[INET6_ADDRSTRLEN];
  mio::inet_ntop_sin(&client_addr, cstr, sizeof cstr);
  printf("received number: %d from: %s\n", number, cstr);

  return 0;
}

