#include "mio/socket/socket.h"


int main(int argc, char *argv[]){
  mio::CClientTCP client_tcp;
  client_tcp.Init("192.168.1.20", "3495");

  if(argc == 2){
    uint8_t number;
    client_tcp.RecvFromServer(&number, sizeof(uint8_t));
    printf("received number: %d\n", number);
  }
  else{
    uint8_t number = 33;
    client_tcp.SendToServer(&number, sizeof(uint8_t));
  }

  return 0;
}

