#ifndef __MIO_GALIL_COMMON_H__
#define __MIO_GALIL_COMMON_H__

#include <bitset>
#include "gclib.h"
#include "gclibo.h"
#include "gclib_errors.h"
#include "mio/altro/io.hpp" //ResolveUdevSymlink()


class GalilException : std::exception {
  public:
    GalilException(const GReturn gr) : _gr(gr) {}
    GalilException(const GReturn gr, const std::string err_msg) : _gr(gr), _err_msg(err_msg) {}

    const char* what() const throw(){
      char buf[1024];
      GError(_gr, buf, 1024);
      std::string str = _err_msg.empty() ? buf : _err_msg + " - " + buf;
      return str.c_str();
    }

    GReturn code() const{
      return _gr;
    }

    ~GalilException() throw() {}

  private:
    GReturn _gr;
    std::string _err_msg;
};


#define G_ERR(func_call)             \
{                                    \
  const GReturn gr = func_call;      \
  if( !!(gr == G_NO_ERROR) ) ; else{ \
    std::ostringstream stream;       \
    stream << __FILE__ << " - " << CURRENT_FUNC << ":" << __LINE__ << " - (" << #func_call << ") returned an error";\
    throw GalilException( gr, stream.str() ); \
  } \
}   \


inline void OpenGalil(GCon &galil_con, std::string dev_str){
  EXP_CHK(galil_con == 0, return)
  char buf[1024];
  std::string cmd_str, res_symlink;
  ResolveUdevSymlink(dev_str, res_symlink);
  std::cout << "OpenGalil() attempting to open controller at " << res_symlink << std::endl;
  cmd_str = res_symlink + " --baud 19200 --timeout 5000 --subscribe MG --direct";
  try{
    G_ERR( GOpen(cmd_str.c_str(), &galil_con) );
    G_ERR( GInfo(galil_con, buf, 1024) ); //grab connection string
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
    return;
  }
  std::cout << buf << "\n";
}


inline void CloseGalil(GCon &galil_con){
  EXP_CHK(galil_con != 0, return)
  try{
    G_ERR( GCmd(galil_con, "MO") );
    G_ERR( GClose(galil_con) );
    galil_con = 0;
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
    return;
  }
}


inline void GetOutputStatus(const GCon &galil_con){
  try{
    int digital_out_bit_mask_int;
    G_ERR( GCmdI(galil_con, "MG _OP0", &digital_out_bit_mask_int) );
    std::bitset<8> bs( static_cast<uint8_t>(digital_out_bit_mask_int) );
    std::cout << "digital_out_bit_mask: [" << bs << "]\n\n";
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}

namespace icv{

// Set ouputs 1-4 high (relays are active low, so 1 will turn them off) and set ouputs 5-8 low
inline void InitDgitalOutputs(const GCon &galil_con){
  try{
    G_ERR( GCmd(galil_con, "OP $0F") ); // Clear all bits
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}


// true sets all outputs to 1 (high); false sets all outputs to 0 (low)
inline void SetAllDigitalOutputs(const GCon &galil_con, const bool flag){
  try{
    if(flag){
      G_ERR( GCmd(galil_con, "OP $FF") ); // Set all bits
    }
    else{
      G_ERR( GCmd(galil_con, "OP $00") ); // Clear all bits
    }
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}


// state = true, turn relay on; state = false, turn replay off
inline void SetRelayState(const GCon &galil_con, const int relay_num, const bool state){
  STD_INVALID_ARG_E(galil_con != 0)
  STD_INVALID_ARG_E(relay_num >= 1 && relay_num <= 4)
  try{
    int digital_out_bit_mask_temp;
    G_ERR( GCmdI(galil_con, "MG _OP0", &digital_out_bit_mask_temp) );
    uint8_t digital_out_bit_mask = digital_out_bit_mask_temp;

    enum RelayBitFlag {Relay1 = 1 << 0, Relay2 = 1 << 1, Relay3 = 1 << 2, Relay4 = 1 << 3};
    const uint8_t relay_bit_flags[] = {Relay1, Relay2, Relay3, Relay4};
    if(state)
      digital_out_bit_mask &= ~relay_bit_flags[relay_num-1]; //clear bit; low turns relay on
    else
      digital_out_bit_mask |= relay_bit_flags[relay_num-1]; //set bit; high turns relay off

    char cmd_str[32];
    snprintf(cmd_str, 32, "OP %d", digital_out_bit_mask);
    G_ERR( GCmd(galil_con, cmd_str) );
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}


inline void TriggerCams(const GCon &galil_con){
  STD_INVALID_ARG_E(galil_con != 0)
  int digital_out_bit_mask_temp;
  G_ERR( GCmdI(galil_con, "MG _OP0", &digital_out_bit_mask_temp) );
  const uint8_t digital_out_bit_mask = digital_out_bit_mask_temp;

  enum CameraTriggerBitFlag {Camera1 = 1 << 4, Camera2 = 1 << 5};
  const uint8_t cam_trig_bit_flag[] = {Camera1, Camera2};
  uint8_t camera_trig_on_bit_mask = digital_out_bit_mask | cam_trig_bit_flag[0] | cam_trig_bit_flag[1],
          camera_trig_off_bit_mask = digital_out_bit_mask & ~cam_trig_bit_flag[0] & ~cam_trig_bit_flag[1];

  char cmd_str[256];
  snprintf(cmd_str, 256, "OP %d;WT 5;OP %d", camera_trig_on_bit_mask, camera_trig_off_bit_mask);
  try{
    G_ERR( GCmd(galil_con, cmd_str) );
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}


class TriggerCamLoop{
  public:
    bool exit_thread_, started_;
    std::thread thread_;
    std::mutex *gc_mutex_;
    GCon gc_ = 0;

    TriggerCamLoop(GCon gc, std::mutex *gc_mutex) : exit_thread_(false), started_(false){
      STD_INVALID_ARG_E(gc != nullptr)
      STD_INVALID_ARG_E(gc_mutex != nullptr)
      gc_ = gc;
      gc_mutex_ = gc_mutex;
    }

    void Thread(){
      const double frequency = 8;
      const double period_ms = 1000.0/frequency;
      for(;;){
        if(exit_thread_)
          break;
        gc_mutex_->lock();
        icv::TriggerCams(gc_);
        gc_mutex_->unlock();
        std::this_thread::sleep_for( std::chrono::milliseconds(static_cast<size_t>(period_ms)) );
      }
    }

    void Start(){
      EXP_CHK(started_ == false, return)
      exit_thread_ = false;
      thread_ = std::thread(&TriggerCamLoop::Thread, this);
      printf("%s - started\n", CURRENT_FUNC);
      started_ = true;
    }

    void Stop(){
      EXP_CHK(started_ == true, return)
      exit_thread_ = true;
      thread_.join();
      printf("%s - stopped\n", CURRENT_FUNC);
      started_ = false;
    }
};


/*
//jogs motor at 1/4 rotation per second for driver tuning
void CRobot::TuneMotor(){
  EXP_CHK(m_gc != 0, return)

  G_ERR( GCmd(m_gc, "SH C") );
  G_ERR( GCmd(m_gc, "JG ,,500") );
  G_ERR( GCmd(m_gc, "BG C") );
  for(;;){
    char key;
    std::cout << "press q to stop\n";
    std::cin >> key;
    if(key == 'q')
      break;
  }
  G_ERR( GCmd(m_gc, "ST") );
}


void CRobot::LoadProgram(){
  try{
    STD_INVALID_ARG_E(m_gc != 0)
    std::string trig_cam_routine = "#TriggerCam;SB 1,2;WT 1;CB 1,2;EN;";
    std::string program = trig_cam_routine;

    G_ERR( GProgramDownload(m_gc, program.c_str(), NULL) );
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}

void CRobot::RunProgram(){
  try{
    STD_INVALID_ARG_E(m_gc != 0)
    G_ERR( GCmd(m_gc, "XQ #TriggerCam,1") );
  }
  catch(const GalilException &ge){
    std::cout << CURRENT_FUNC << "(): caught an error: "  << ge.what() << std::endl;
  }
}
*/

} //namespace icv

#endif //__MIO_GALIL_COMMON_H__

