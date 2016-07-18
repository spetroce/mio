#include "mio/socket/socket.h"


int main(int argc, char *argv[]){
  mio::CServerTCP server_tcp;
  server_tcp.Init("", "3495");
  server_tcp.ListenAccept();

  if(argc == 2){
    uint8_t number = 33;
    printf("Sending 33 to Client\n");
    server_tcp.SendToClient(&number, sizeof(uint8_t));
  }
  else{
    uint8_t number;
    server_tcp.RecvFromClient(&number, sizeof(uint8_t));
    printf("received number: %d\n", number);
  }

  return 0;
}

