// 6-Jan-2010 Samuel Petrocelli

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include "mio/serialCom/serialCom.h"


SerialCom::SerialCom() : is_init_(false), port_fd_(0){}

SerialCom::~SerialCom(){
  if(is_init_)
    Uninit(true);
}


int SerialCom::Init(const char *path_name, int nFlags){
  EXP_CHK_E(!is_init_, return(0))
  if( ( port_fd_ = open(path_name, nFlags) ) == -1){
    perror("SerialCom::Init() - open()");
    if(errno == EACCES)
      printf("It's possible that the current user is not part of the dialout group\n"
"Run the following command to check if you are a member of dialout:\n" 
"id -Gn <username>\n"
"Run the following command to add a user to the dialout group:\n"
"sudo usermod -a -G dialout <username>\n");
    return -1;
  }
  //save original termios settings in termios_orig_ (only for restoring settings in Uninit)
  ERRNO_CHK_E(tcgetattr(port_fd_, &termios_orig_) == 0, return(-1))
  //get original termios settings and put in termios_new_ (what is used throughtout library)
  ERRNO_CHK_E(tcgetattr(port_fd_, &termios_new_) == 0, return(-1))

  //used by select() in SerialCom::Read() and SerialCom::Write()
  FD_ZERO(&write_fd_set_);
  FD_SET(port_fd_, &write_fd_set_);
  FD_ZERO(&read_fd_set_);
  FD_SET(port_fd_, &read_fd_set_);
  
  is_init_ = true;
  return 0;
}


int SerialCom::Uninit(const bool kRestoreSettings){
  EXP_CHK_E(is_init_, return(0))

  if(kRestoreSettings)
    ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_orig_) == 0, return(-1))
  
  close(port_fd_);
  port_fd_ = 0;
  is_init_ = false;
  return 0;
}


int SerialCom::GetPortFD(){
  return port_fd_;
}


int SerialCom::SetDefaultControlFlags(){
  EXP_CHK_E(is_init_, return(-1))
  /*
    CLOCAL - local line, do not change "owner" of port
    CREAD - enable receiver, serial interface driver will read incoming data bytes
  */
  termios_new_.c_cflag |= (CLOCAL | CREAD);
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::SetOutputType(const outputType type){
  EXP_CHK_E(is_init_, return(-1))

  switch(type){
    case PROCESSED_OUTPUT:
      termios_new_.c_oflag |= OPOST;
      break;
    case RAW_OUTPUT:
      termios_new_.c_oflag &= ~OPOST;
      break;
    default:
      printf("%s - invalid outputType\n", CURRENT_FUNC);
      return -1;
  }
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
}


int SerialCom::SetInputType(const inputType type){
  EXP_CHK_E(is_init_, return(-1))

  switch(type){
    case CANONICAL_INPUT:
      termios_new_.c_lflag |= (ICANON | ECHO | ECHOE);
      break;
    case RAW_INPUT:
      termios_new_.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
      break;
    default:
      printf("%s - invalid inputType\n", CURRENT_FUNC);
      return -1;
  }
  
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


//baud rates defined in <bits/termios.h>
speed_t SerialCom::GetSpeedVal(const unsigned int baud_rate){
  switch(baud_rate){
#ifdef B0
    case 0: return(B0); //hang-up
#endif
#ifdef B50
    case 50: return(B50);
#endif
#ifdef B75
    case 75: return(B75);
#endif
#ifdef B110
    case 110: return(B110);
#endif
#ifdef B134
    case 134: return(B134);
#endif
#ifdef B150
    case 150: return(B150);
#endif
#ifdef B200
    case 200: return(B200);
#endif
#ifdef B300
   case 300: return(B300);
#endif
#ifdef B600
    case 600: return(B600);
#endif
#ifdef B1200
    case 1200: return(B1200);
#endif
#ifdef B1800
   case 1800: return(B1800);
#endif
#ifdef B2400
    case 2400: return(B2400);
#endif
#ifdef B4800
    case 4800: return(B4800);
#endif
#ifdef B7200
   case 7200: return(B7200); //not defined
#endif
#ifdef B9600
    case 9600: return(B9600);
#endif
#ifdef B14400
    case 14400: return(B14400);
#endif
#ifdef B19200
    case 19200: return(B19200);
#endif
#ifdef B28800
    case 28800: return(B28800); //not defined
#endif
#ifdef B38400
    case 38400: return(B38400);
#endif
#ifdef B57600
    case 57600: return(B57600);
#endif
#ifdef B76800
    case 76800: return(B76800); //not defined
#endif
#ifdef B115200
    case 115200: return(B115200);
#endif
#ifdef B128000
    case 128000: return(B128000); //not defined
#endif
#ifdef B230400
    case 230400: return(B230400);
#endif
#ifdef B460800
   case 460800: return(B460800);
#endif
#ifdef B500000
    case 500000: return(B500000);
#endif
#ifdef B576000
    case 576000: return(B576000);
#endif
#ifdef B921600
    case 921600: return(B921600);
#endif
#ifdef B1000000
    case 1000000: return(B1000000);
#endif
#ifdef B1152000
    case 1152000: return(B1152000);
#endif
#ifdef B1500000
    case 1500000: return(B1500000);
#endif
#ifdef B2000000
    case 2000000: return(B2000000);
#endif
#ifdef B2500000
    case 2500000: return(B2500000);
#endif
#ifdef B3000000
    case 3000000: return(B3000000);
#endif
#ifdef B3500000
    case 3500000: return(B3500000);
#endif
#ifdef B4000000
    case 4000000: return(B4000000);
#endif
    default: return -1;
  }
}


int SerialCom::SetOutBaudRate(const unsigned int baud_rate){
  speed_t speed;
  
  EXP_CHK_E(is_init_, return(-1))
  EXP_CHK_EM((speed = GetSpeedVal(baud_rate)) != -1, return(-1),
             std::string("invalid baud rate: ") + std::to_string(baud_rate));
  EXP_CHK_EM(cfgetospeed(&termios_new_) != speed, return(0), "already at requested baud rate, doing nothing")
  ERRNO_CHK_E(cfsetospeed(&termios_new_, speed) == 0, return(-1))
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::SetInBaudRate(const unsigned int baud_rate){
  speed_t speed;
  
  EXP_CHK_E(is_init_, return(-1))
  EXP_CHK_EM((speed = GetSpeedVal(baud_rate)) != -1, return(-1),
             std::string("invalid baud rate: ") + std::to_string(baud_rate));
  EXP_CHK_EM(cfgetispeed(&termios_new_) != speed, return(0), "already at requested baud rate, doing nothing")
  ERRNO_CHK_E(cfsetispeed(&termios_new_, speed) == 0, return(-1))
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}
  

int SerialCom::SetCharSize(const unsigned int char_size){
  EXP_CHK_E(is_init_, return(-1))
  assert(char_size >= 5 || char_size <= 8);
  
  termios_new_.c_cflag &= ~CSIZE;
  
  switch(char_size){
    case 5:
      termios_new_.c_cflag |= CS5; break;
    case 6:
      termios_new_.c_cflag |= CS6; break;
    case 7:
      termios_new_.c_cflag |= CS7; break;
    case 8:
      termios_new_.c_cflag |= CS8; break;
  }
  
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::GetCharSize(unsigned int &char_size){
  EXP_CHK_E(is_init_, return(-1))
  char_size = 0;
  
  //the 5th and 6th bits of c_cflag are for character size, the order below is important
  if( (termios_new_.c_cflag & CS8) == CS8 ) //48
    char_size = 8;
  else if( (termios_new_.c_cflag & CS7) == CS7 ) //32
    char_size = 7;
  else if( (termios_new_.c_cflag & CS6) == CS6 ) //16
    char_size = 6;
  else if( (termios_new_.c_cflag & CS5) == CS5) //0
    char_size = 5;

  EXP_CHK_EM(char_size != 0, return(-1), "could not determine char size")
    
  //printf("CS5: %d\nCS6: %d\nCS7: %d\nCS8: %d\n", CS5, CS6, CS7, CS8);
  //printf("CSTOPB: %d\nCSIZE: %d\nPARENB: %d\nPARODD: %d\n", CSTOPB, CSIZE, PARENB, PARODD);
  return 0;
}


int SerialCom::SetParity(const unsigned int parity_type){
  EXP_CHK_E(is_init_, return(-1))

  switch(parity_type){
    case PARITY_NONE:
      termios_new_.c_cflag &= ~PARENB;
      break;
    case PARITY_EVEN:
      termios_new_.c_cflag |= PARENB;
      termios_new_.c_cflag &= ~PARODD;
      break;
    case PARITY_ODD:
      termios_new_.c_cflag |= (PARENB | PARODD);
      break;
#ifdef CMSPAR
    case PARITY_SPACE:
      termios_new_.c_cflag |= (PARENB | CMSPAR);
      termios_new_.c_cflag &= ~PARODD;
      break;
    case PARITY_MARK:
      termios_new_.c_cflag |= (PARENB | CMSPAR | PARODD);
#endif
    default:
      printf("SerialCom::SetParity(): invalid parity option: %d\n", parity_type);
      return(-1);
  }
  
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::SetStopBits(unsigned int num_stop_bits){
  EXP_CHK_E(is_init_, return(-1))
  assert(num_stop_bits == 1 || num_stop_bits == 2);

  if(num_stop_bits == 1)
    termios_new_.c_cflag &= ~CSTOPB;
  else //num_stop_bits == 2
    termios_new_.c_cflag |= CSTOPB;

  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::GetStopBits(unsigned int &num_stop_bits){
  EXP_CHK_E(is_init_, return(-1))
  num_stop_bits = ( (termios_new_.c_cflag & CSTOPB) == CSTOPB ) ? 2 : 1;
  return 0;
}


int SerialCom::SetHardwareFlowControl(const bool hardware_flow_control){
  EXP_CHK_E(is_init_, return(-1))
  if(hardware_flow_control)
    termios_new_.c_cflag |= CRTSCTS;
  else
    termios_new_.c_cflag &= ~CRTSCTS;
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::GetHardwareFlowControl(bool &hardware_flow_control){
  EXP_CHK_E(is_init_, return(-1))
  hardware_flow_control = ( (termios_new_.c_cflag & CRTSCTS) == CRTSCTS );
  return 0;
}


int SerialCom::SetSoftwareFlowControl(const bool software_flow_control){
  EXP_CHK_E(is_init_, return(-1))
  if(software_flow_control)
    termios_new_.c_iflag |= (IXON | IXOFF | IXANY);
  else
    termios_new_.c_iflag &= ~(IXON | IXOFF | IXANY);
  ERRNO_CHK_E(tcsetattr(port_fd_, TCSANOW, &termios_new_) == 0, return(-1))
  return 0;
}


int SerialCom::GetSoftwareFlowControl(bool &software_flow_control){
  EXP_CHK_E(is_init_, return(-1))
  software_flow_control = ( ( termios_new_.c_iflag & (IXON | IXOFF | IXANY) ) == (IXON | IXOFF | IXANY) );
  return 0;
}


int SerialCom::Write(const void *data_buf, const unsigned int data_buf_len, const bool drain_buffer, 
                        const unsigned int time_out_sec, const unsigned int time_out_limit){
  EXP_CHK_E(is_init_, return(-1))
  EXP_CHK_E(data_buf_len > 0, return(0))
  
  int num_byte, num_active_fd;
  unsigned int num_timeout = 0, num_byte_written = 0;
  struct timeval tv, tv_temp;

  // Set timeout struct
  tv.tv_sec = time_out_sec;
  tv.tv_usec = 0;
  
  while(data_buf_len > num_byte_written){
    do{
      tv_temp = tv; //select may be modifying tv_temp so reset
      num_active_fd = select(port_fd_ + 1, NULL, &write_fd_set_, NULL, &tv_temp); //getdtablesize()
      // This error is sometimes thrown, so I check separately 
      ERRNO_CHK_EM(num_active_fd != -1 && errno != EINTR, return(-1), "select() EINTR error")
      ERRNO_CHK_EM(num_active_fd != -1, return(-1), "select() error")
      if(num_active_fd == 0){
        num_timeout++;
        printf("%s - timeout occurence %d\n", CURRENT_FUNC, num_timeout);
        EXP_CHK_E(num_timeout >= time_out_limit, return(-1))
      }
      else{
        ERRNO_CHK_E((num_byte = write(port_fd_, static_cast<const uint8_t*>(data_buf)+num_byte_written,
                                      data_buf_len-num_byte_written)) != -1, return(-1))
      }
    }while(num_active_fd == 0); //loop for timeout check instances
    num_byte_written += num_byte;
  }
  
  //tcdrain() function blocks until all output data is written to fildes
  if(drain_buffer)
    ERRNO_CHK_E(tcdrain(port_fd_) != -1, return(-1));
  return 0;
}


int SerialCom::SendByte(void *single_byte){
  EXP_CHK_E(is_init_, return(-1))
  int num_byte_written;
  ERRNO_CHK_E((num_byte_written = write(port_fd_, single_byte, 1)) != -1, return(-1))
  EXP_CHK_E(num_byte_written == 1, return(-1))
  return 0;
}


/*
unsigned char *pucData = input buffer data (MUST BE LARGE ENOUGH!)
int nReqLen            = attempt to read up to nReqLen number of bytes from port_fd_
int &nActualLen        = number of bytes actually received from port
*/
int SerialCom::Read(void *data_buf, const unsigned int req_buffer_len, unsigned int &num_read_byte,
                       const unsigned int time_out_sec, const unsigned int time_out_limit){
  EXP_CHK_E(is_init_, return(-1))
  EXP_CHK_E(req_buffer_len > 0, return(0)) 
//  int nNumBytes;
//  ioctl(port_fd_, FIONREAD, &nNumBytes);
//  printf("input bytes available: %d\n", nNumBytes);
  
  int num_byte, num_active_fd;
  unsigned int num_timeout = 0;
  struct timeval tv, tv_temp;

  //Wait up to five seconds.
  tv.tv_sec = time_out_sec;
  tv.tv_usec = 0;

  num_read_byte = 0;
  while(req_buffer_len > num_read_byte){
    do{
      tv_temp = tv; //select may be modifying tv_temp so reset
      num_active_fd = select(port_fd_ + 1, &read_fd_set_, NULL, NULL, &tv_temp); //getdtablesize()
      //this error is sometimes thrown, so I check separately 
      ERRNO_CHK_EM(num_active_fd != -1 && errno != EINTR, return(-1), "select() EINTR error")
      ERRNO_CHK_EM(num_active_fd != -1, return(-1), "select() error")
      if(num_active_fd == 0){
        num_timeout++;
        printf("%s - timeout occurence %d\n", CURRENT_FUNC, num_timeout);
        EXP_CHK_E(num_timeout >= time_out_limit, return(-1))
      }
      else{
        ERRNO_CHK_E((num_byte = read(port_fd_, static_cast<uint8_t*>(data_buf)+num_read_byte, 
                    req_buffer_len-num_read_byte)) != -1, return(-1))
      }
    } while(num_active_fd == 0); //loop for timeout check instances
    num_read_byte += num_byte;
  }
  
  return 0;
}


//use a pull-up resistor on this line to logic 0 (voltage high) so it's not floating
int SerialCom::CheckCTS(bool &state){
  EXP_CHK_E(is_init_, return(-1))
  int status;
  //Read terminal status line: Clear To Send
  ERRNO_CHK_E(ioctl(port_fd_, TIOCMGET, &status) != -1, return(-1))
  state = ( (status & TIOCM_CTS) != 0 );
  return 0;
}


//typically rests at logic 0 (voltage high)
int SerialCom::SetRTS(const bool state){
  EXP_CHK_E(is_init_, return(-1))
  int status;
  ERRNO_CHK_E(ioctl(port_fd_, TIOCMGET, &status) != -1, return(-1))
  if(state)
    status |= TIOCM_RTS;
  else 
    status &= ~TIOCM_RTS;
  ERRNO_CHK_E(ioctl(port_fd_, TIOCMSET, &status) != -1, return(-1))
  return 0;
}


//typically rests at logic 0 (voltage high)
int SerialCom::SetDTR(const bool state){
  EXP_CHK_E(is_init_, return(-1))
  int status;
  ERRNO_CHK_E(ioctl(port_fd_, TIOCMGET, &status) != -1, return(-1))
  if(state) 
    status |= TIOCM_DTR;
  else 
    status &= ~TIOCM_DTR;
  ERRNO_CHK_E(ioctl(port_fd_, TIOCMSET, &status) != -1, return(-1))
  return 0;
}


/*
Clear any bytes that may be queued for input on device without 
reading these bytes; this is a DESTRUCTIVE input flush
*/
int SerialCom::FlushInput(){
  EXP_CHK_E(is_init_, return(-1))
  ERRNO_CHK_E(tcflush(port_fd_, TCIFLUSH) != -1, return(-1))
  return 0;
}


/*
Clear any bytes that may be queued for output on device without
writing these bytes; this is a DESTRUCTIVE output flush
*/
int SerialCom::FlushOutput(){
  EXP_CHK_E(is_init_, return(-1))
  ERRNO_CHK_E(tcflush(port_fd_, TCOFLUSH) != -1, return(-1))
  return 0;
}


int SerialCom::FlushIO(){
  EXP_CHK_E(is_init_, return(-1))
  ERRNO_CHK_E(tcflush(port_fd_, TCIOFLUSH) != -1, return(-1))
  return 0;
}


/*
int &num_in_bytes, &num_out_bytes : references to load with values of bytes in input 
			    queue and output queue respectively
*/
int SerialCom::InQueue(int &num_in_bytes, int &num_out_bytes){
  EXP_CHK_E(is_init_, return(-1))
  // at least this many bytes have to be available
  ERRNO_CHK_E(ioctl(port_fd_, FIONREAD, num_in_bytes) != -1, return(-1))
  ERRNO_CHK_E(ioctl(port_fd_, TIOCOUTQ, num_out_bytes) != -1, return(-1))
  return 0;
}
