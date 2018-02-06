#ifndef __MIO_SERIAL_COM_H__
#define __MIO_SERIAL_COM_H__

#include <sys/termios.h>
#include <time.h>
#include <string>
#include <unistd.h> //getdtablesize()
#include <stdint.h>
#include <fcntl.h>
#include "mio/altro/error.h"

#define PARITY_NONE 0
#define PARITY_EVEN 1
#define PARITY_ODD 2
#ifdef CMSPAR
  #define PARITY_SPACE 3
  #define PARITY_MARK 4
#endif

enum inputType{
  CANONICAL_INPUT,
  RAW_INPUT
};


enum outputType{
  PROCESSED_OUTPUT, 
  RAW_OUTPUT
};


class SerialCom{

  private:
    bool is_init_;
    int port_fd_;

    struct termios termios_orig_, termios_new_;
    fd_set write_fd_set_, read_fd_set_;

  public:
    SerialCom();
    ~SerialCom();

    int Init(const char *path_name, int nFlags = O_RDWR | O_NOCTTY | O_NDELAY); //must have O_RDONLY, O_WRONLY, or O_RDWR flag
    int Uninit(const bool kRestoreSettings);

    int GetPortFD();
    bool IsInit();

    int SetInputType(const inputType type);
    int SetOutputType(const outputType type);
    int SetDefaultControlFlags();

    static speed_t GetSpeedVal(const unsigned int baud_rate);
    int SetOutBaudRate(const unsigned int baud_rate);
    int SetInBaudRate(const unsigned int baud_rate);

    int SetCharSize(const unsigned int char_size);
    int GetCharSize(unsigned int &char_size);

    int SetParity(const unsigned int parity_type);
    int SetStopBits(unsigned int num_stop_bits);
    int GetStopBits(unsigned int &num_stop_bits);
    int SetHardwareFlowControl(const bool hardware_flow_control);
    int GetHardwareFlowControl(bool &hardware_flow_control);
    int SetSoftwareFlowControl(const bool software_flow_control);
    int GetSoftwareFlowControl(bool &software_flow_control);
    
    int Write(const void *data_buf, const unsigned int data_buf_len, const bool drain_buffer, 
               const unsigned int time_out_sec = 3, const unsigned int time_out_limit = 3);
    int SendByte(void *single_byte);
    int Read(void *pvDataBuf, const unsigned int req_buffer_len, unsigned int &num_read_byte,
              const unsigned int time_out_sec = 3, const unsigned int time_out_limit = 3);
    
    int CheckCTS(bool &state);
    int SetRTS(const bool state);
    int SetDTR(const bool state);
    
    int FlushInput();
    int FlushOutput();
    int FlushIO();
    int InQueue(int &num_in_bytes, int &num_out_bytes);
};

#endif //__MIO_SERIAL_COM_H__

