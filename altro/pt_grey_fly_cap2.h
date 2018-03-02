#ifndef __MIO_PT_GREY_FLY_CAP__
#define __MIO_PT_GREY_FLY_CAP__

#if defined(LINUX32) || defined(LINUX64)
#define LINUX
#endif

#ifdef LINUX
#include <unistd.h>
#include <time.h>
#endif
#include <iostream>
#include <sstream>
#include "FlyCapture2.h"
#if CV_MAJOR_VERSION < 3
#include "opencv2/core/core.hpp"
#else
#include "opencv2/core.hpp"
#endif
#include "mio/altro/error.h"


namespace mio{

#define PGR_ERR_VAR FlyCapture2::Error _ptgrey_error;

#undef EXP_CHK
#define EXP_CHK(exp, exit_function)                                  \
if( !!(exp) ) ; else{                                                  \
  std::cout << __FILE__ << " - " << CURRENT_FUNC << ":" << __LINE__ << \
               " - (" << #exp << ") is false.\n";                      \
  exit_function;                                                       \
}

#define PGR_ERR_OK(ptgrey_func_call, exit_function)\
EXP_CHK(ptgrey_func_call == FlyCapture2::PGRERROR_OK, _ptgrey_error.PrintErrorTrace(); exit_function)

#define PGR_EXP_CHK(exp, exit_function)\
EXP_CHK(exp, _ptgrey_error.PrintErrorTrace(); exit_function);


inline void PrintProperty(const FlyCapture2::Property &prop){
#define BOOL_STR(flag) flag ? "true" : "false"
  printf("type=%d, present=%s, absControl=%s, onePush=%s, onOff=%s, "
"autoManualMode=%s, valueA=%u, valueB=%u, absValue=%.6f\n",
         static_cast<int>(prop.type), BOOL_STR(prop.present), BOOL_STR(prop.absControl), BOOL_STR(prop.onePush),
         BOOL_STR(prop.onOff), BOOL_STR(prop.autoManualMode), prop.valueA, prop.valueB, prop.absValue);
#undef BOOL_STR
}


inline void FC2ImageToOpenCvMat(const FlyCapture2::Image &image, cv::Mat &mat,
                                FlyCapture2::Image *rgb_img = nullptr, const bool clone_data = false){
  FlyCapture2::Image _rgb_img;
  if(image.GetPixelFormat() == FlyCapture2::PIXEL_FORMAT_MONO8){
    if(clone_data)
      mat = cv::Mat(image.GetRows(), image.GetCols(), CV_8U, image.GetData()).clone();
    else
      mat = cv::Mat(image.GetRows(), image.GetCols(), CV_8U, image.GetData());
  }
  else{
    if(!rgb_img)
      rgb_img = &_rgb_img;
	  // convert to rgb
    image.Convert(FlyCapture2::PIXEL_FORMAT_BGR, rgb_img);
    // convert to OpenCV Mat
	  size_t row_bytes = static_cast<double>(rgb_img->GetReceivedDataSize())/static_cast<double>(rgb_img->GetRows());
    if(clone_data)
      mat = cv::Mat(rgb_img->GetRows(), rgb_img->GetCols(), CV_8UC3, rgb_img->GetData(), row_bytes).clone();
    else
      mat = cv::Mat(rgb_img->GetRows(), rgb_img->GetCols(), CV_8UC3, rgb_img->GetData(), row_bytes);
  }
}


inline void PrintBuildInfo(){
	FlyCapture2::FC2Version fc2Version;
	FlyCapture2::Utilities::GetLibraryVersion(&fc2Version);

	std::ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "."
          << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	std::cout << version.str() <<std::endl;
}


inline void PrintCameraInfo(FlyCapture2::CameraInfo *cam_info){
	std::cout << std::endl;
	std::cout << "Serial number -" << cam_info->serialNumber << std::endl;
	std::cout << "Camera model - " << cam_info->modelName << std::endl;
	std::cout << "Camera vendor - " << cam_info->vendorName << std::endl;
	std::cout << "Sensor - " << cam_info->sensorInfo << std::endl;
	std::cout << "Resolution - " << cam_info->sensorResolution << std::endl;
	std::cout << "Firmware version - " << cam_info->firmwareVersion << std::endl;
	std::cout << "Firmware build time - " << cam_info->firmwareBuildTime << std::endl << std::endl;
}


inline void PrintImageInfo(const FlyCapture2::Image &img){
  std::cout << FL_STRM << std::endl;
  std::cout << "Rows: " << img.GetRows() << std::endl;
  std::cout << "Cols: " << img.GetCols() << std::endl;
  std::cout << "BitsPerPixel: " << img.GetBitsPerPixel() << std::endl << std::endl;
}


inline bool PollForTriggerReady(FlyCapture2::Camera &cam){
  PGR_ERR_VAR
	const unsigned int kSoftwareTrigger = 0x62C;
	unsigned int register_val = 0;
	do{
    PGR_ERR_OK(cam.ReadRegister(kSoftwareTrigger, &register_val), return(false))
	}while((register_val >> 31) != 0);

	return true;
}


inline bool WaitForPtGreyPowerUp(FlyCapture2::Camera &cam, const unsigned int kCameraPower = 0x610,
                           const unsigned int kPowerVal = 0x80000000){
  PGR_ERR_VAR
  // Power up the camera
  PGR_ERR_OK(cam.WriteRegister(kCameraPower, kPowerVal), return(false))
  // Wait for camera to complete power-up
  const unsigned int millisec_to_sleep = 100;
  unsigned int register_val = 0;
  unsigned int retries = 10;
  FlyCapture2::Error read_register_error;
  do{
#if defined(WIN32) || defined(WIN64)
	  Sleep(millisec_to_sleep);
#elif defined(LINUX)
	  struct timespec nanosec_delay;
	  nanosec_delay.tv_sec = 0;
	  nanosec_delay.tv_nsec = (long)millisec_to_sleep * 1000000L;
	  nanosleep(&nanosec_delay, NULL);
#endif
	  read_register_error = cam.ReadRegister(kCameraPower, &register_val);
    // Ignore timeout errors, camera may not be responding to register reads during power-up
	  if (read_register_error == FlyCapture2::PGRERROR_TIMEOUT){}
    else{
      PGR_ERR_OK(read_register_error, return(false))
	  }

	  retries--;
  } while ((register_val & kPowerVal) == 0 && retries > 0);
  // Check for timeout errors after retrying
  PGR_EXP_CHK(read_register_error == FlyCapture2::PGRERROR_TIMEOUT, return(false))
  return true;
}


inline bool SetTriggerMode(FlyCapture2::Camera &cam, const bool onOff){
  PGR_ERR_VAR
  // Check for external trigger support
  FlyCapture2::TriggerModeInfo trigger_mode_info;
  PGR_ERR_OK(cam.GetTriggerModeInfo(&trigger_mode_info), return(false))
  EXP_CHK_ERRNO_M(trigger_mode_info.present == true, return(false), "trigger mode not supported")

  // Get current trigger settings
	FlyCapture2::TriggerMode trigger_mode;
  PGR_ERR_OK(cam.GetTriggerMode(&trigger_mode), return(false))
  trigger_mode.onOff = onOff;
  if(onOff){
    trigger_mode.mode = 0;
    trigger_mode.source = 0; // GPIO 0 as trigger input
    trigger_mode.parameter = 0;
    trigger_mode.polarity = 1; // Trigger on rising edge
    printf("rising edge trigger GPIO: %d\n", trigger_mode.source);
  }
  PGR_ERR_OK(cam.SetTriggerMode(&trigger_mode), return(false))

  //if(onOff)
    //PollForTriggerReady(cam);
  return true;
}


inline void PrintFormat7Capabilities(FlyCapture2::Format7Info fmt7Info){
	std::cout << "Max image pixels: (" << fmt7Info.maxWidth << ", " << fmt7Info.maxHeight << ")" << std::endl;
	std::cout << "Image Unit size: (" << fmt7Info.imageHStepSize << ", " << fmt7Info.imageVStepSize << ")" << std::endl;
	std::cout << "Offset Unit size: (" << fmt7Info.offsetHStepSize << ", " << fmt7Info.offsetVStepSize << ")" << std::endl;
	std::cout << "Pixel format bitfield: 0x" << fmt7Info.pixelFormatBitField << std::endl;
}


/*
  If absControl is available for the property it is used, otherwise, valueA is used.
  {FlyCapture2::BRIGHTNESS,
   FlyCapture2::AUTO_EXPOSURE,
   FlyCapture2::SHARPNESS,
   FlyCapture2::WHITE_BALANCE,
   FlyCapture2::HUE,
   FlyCapture2::SATURATION,
   FlyCapture2::GAMMA,
   FlyCapture2::IRIS,
   FlyCapture2::FOCUS,
   FlyCapture2::ZOOM,
   FlyCapture2::PAN,
   FlyCapture2::TILT,
   FlyCapture2::SHUTTER,
   FlyCapture2::GAIN,
   FlyCapture2::TRIGGER_MODE,
   FlyCapture2::TRIGGER_DELAY,
   FlyCapture2::FRAME_RATE,
   FlyCapture2::TEMPERATURE,
   FlyCapture2::UNSPECIFIED_PROPERTY_TYPE,
   FlyCapture2::PROPERTY_TYPE_FORCE_32BITS};
*/
inline bool SetProperty(FlyCapture2::Camera &cam, const FlyCapture2::PropertyType prop_type, const double value,
                        const bool enable_auto, const bool on_off, const bool one_push){
  PGR_ERR_VAR
  FlyCapture2::PropertyInfo prop_info;
  prop_info.type = prop_type;
  PGR_ERR_OK(cam.GetPropertyInfo(&prop_info), return(false))
  FlyCapture2::Property prop;
  prop.type = prop_type;
  prop.absValue = prop.valueA = 0;
  if(prop_info.absValSupported){
    prop.absControl = true;
    prop.absValue = value;
  }
  else{
    prop.absControl = false;
    prop.valueA = value;
  }
  prop.autoManualMode = enable_auto;
  prop.onOff = on_off;
  prop.onePush = one_push;
  PGR_ERR_OK(cam.SetProperty(&prop), return(false))
  return true;
}


inline bool SetFormat7Mode(FlyCapture2::Camera &cam,
                           const FlyCapture2::Mode k_fmt7Mode = FlyCapture2::MODE_0,
                           const FlyCapture2::PixelFormat k_fmt7PixFmt = FlyCapture2::PIXEL_FORMAT_RAW8){
  PGR_ERR_VAR
	FlyCapture2::Format7Info fmt7Info;
	fmt7Info.mode = k_fmt7Mode;
	bool supported;
	PGR_ERR_OK(cam.GetFormat7Info(&fmt7Info, &supported), return(false))
	PrintFormat7Capabilities(fmt7Info);
	EXP_CHK_M((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) != 0, return(false), "Pixel format is not supported")

	FlyCapture2::Format7ImageSettings fmt7ImageSettings;
	fmt7ImageSettings.mode = k_fmt7Mode;
	fmt7ImageSettings.offsetX = 0;
	fmt7ImageSettings.offsetY = 0;
	fmt7ImageSettings.width = fmt7Info.maxWidth;
	fmt7ImageSettings.height = fmt7Info.maxHeight;
	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;

	bool valid;
	FlyCapture2::Format7PacketInfo fmt7PacketInfo;
	// Validate the settings to make sure that they are valid
	PGR_ERR_OK(cam.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo), return(false))
  EXP_CHK_M(valid, return(false), "Format7 settings are not valid")

	// Set the settings to the camera
	PGR_ERR_OK(cam.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket), return(false))
  return true;
}


inline bool InitPtGreyFlyCap2Cam(FlyCapture2::PGRGuid &guid, FlyCapture2::Camera &cam,
                                 const bool wait_for_power_up = false, const bool with_ext_trig = true,
                                 const bool start_capture = true){
  PGR_ERR_VAR
  PGR_ERR_OK(cam.Connect(&guid), return(false))
  if(wait_for_power_up)
    EXP_CHK(WaitForPtGreyPowerUp(cam), return(false))

	// Get the camera information
	FlyCapture2::CameraInfo cam_info;
  PGR_ERR_OK(cam.GetCameraInfo(&cam_info), return(false))
	PrintCameraInfo(&cam_info);

  if(with_ext_trig)
    EXP_CHK(SetTriggerMode(cam, with_ext_trig), return(false))

  // Get the camera configuration
  FlyCapture2::FC2Config config;
  PGR_ERR_OK(cam.GetConfiguration(&config), return(false))
  config.grabTimeout = 5000;	// Set the grab timeout to 5 seconds
  PGR_ERR_OK(cam.SetConfiguration(&config), return(false))

  // Camera is ready, start capturing images
  if(start_capture){
    PGR_ERR_OK(cam.StartCapture(), return(false))
  }
  return true;
}


inline bool UninitPtGreyFlyCap2Cam(FlyCapture2::Camera &cam, const bool with_ext_trig = false){
  PGR_ERR_VAR

  if(with_ext_trig){
  	FlyCapture2::TriggerMode trigger_mode;
    PGR_ERR_OK(cam.GetTriggerMode(&trigger_mode), return(false))
  	trigger_mode.onOff = false;
    PGR_ERR_OK(cam.SetTriggerMode(&trigger_mode), return(false))
  }
  PGR_ERR_OK(cam.StopCapture(), return(false))
  PGR_ERR_OK(cam.Disconnect(), return(false))
  return true;
}

} //namespace mio

#endif //__MIO_PT_GREY_FLY_CAP__

