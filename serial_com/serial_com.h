#ifndef __MIO_SERIAL_COM_H__
#define __MIO_SERIAL_COM_H__

#include <sys/termios.h>
#include <time.h>
#include <string>
#include <unistd.h> //getdtablesize()
#include <stdint.h>
#include <fcntl.h>

enum ParityType {
  NoneParity = 0,
  EvenParity,
  OddParity
#ifdef CMSPAR
  , SpaceParity,
  MarkParity
#endif
};

enum InputType {
  CanonicalInput = 0,
  RawInput
};

enum OutputType {
  ProcessedOutput = 0, 
  RawOutput
};


class SerialCom {

  private:
    bool is_init_;
    int port_fd_;
    std::string dev_path_;

    struct termios termios_orig_, termios_new_;

  public:
    SerialCom();
    ~SerialCom();

    int Init(std::string dev_path, int flags = O_RDWR | O_NOCTTY | O_NDELAY); //must have O_RDONLY, O_WRONLY, or O_RDWR flag
    int Init(const char *dev_path, int flags = O_RDWR | O_NOCTTY | O_NDELAY);
    int Uninit(const bool kRestoreSettings = true);

    int GetPortFD();
    std::string GetDevPath();
    bool IsInit();

    int SetInputType(const InputType type);
    int SetOutputType(const OutputType type);
    int SetDefaultControlFlags();

    static speed_t GetSpeedVal(const unsigned int baud_rate);
    int SetOutBaudRate(const unsigned int baud_rate);
    int SetInBaudRate(const unsigned int baud_rate);

    int SetCharSize(const unsigned int char_size);
    int GetCharSize(unsigned int &char_size);

    int SetParity(const ParityType parity_type);
    int SetParityChecking(const bool enable, const bool ignore, const bool mark, const bool strip);
    int SetIgnoreBreakCondition(const bool ignore);
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

