#include "mio/socket/socket.h"


int main(int argc, char *argv[]){
  mio::CClientUDP client_udp;
  client_udp.Init("192.168.1.9", "3495");

  uint8_t number = 33;
  client_udp.SendToServer(&number, sizeof(uint8_t));

  return 0;
}

