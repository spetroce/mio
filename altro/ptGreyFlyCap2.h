#ifndef __MIO_PT_GREY_FLY_CAP__
#define __MIO_PT_GREY_FLY_CAP__

#if defined(LINUX32) || defined(LINUX64)
#define LINUX
#endif

#ifdef LINUX
#include <unistd.h>
#include <time.h>
#endif

#include "FlyCapture2.h"
#include <iostream>
#include <sstream>


#define PGR_ERR_VAR FlyCapture2::Error _ptgrey_error;

#define EXP_CHK_E(exp, exit_function)                                  \
if( !!(exp) ) ; else{                                                  \
  std::cout << __FILE__ << " - " << CURRENT_FUNC << ":" << __LINE__ << \
               " - (" << #exp << ") is false.\n";                      \
  exit_function;                                                       \
}

#define PGR_ERR_OK(ptgrey_func_call, exit_function)\
EXP_CHK_E(ptgrey_func_call == PGRERROR_OK, _ptgrey_error.PrintErrorTrace(); exit_function)

#define PGR_EXP_CHK(exp, exit_function)\
EXP_CHK_E(exp, _ptgrey_error.PrintErrorTrace(); exit_function);

namespace mio{

void FC2ImageToOpenCvMat(const FlyCapture2::Image &image, cv::Mat &mat,
                         FlyCapture2::Image *rgb_img = nullptr, const bool clone_data = false){
  FlyCapture2::Image _rgb_img;
  if(!rgb_img)
    rgb_img = &_rgb_img;
	// convert to rgb
  raw_image.Convert(FlyCapture2::PIXEL_FORMAT_BGR, rgb_img);
  // convert to OpenCV Mat
	size_t row_bytes = static_cast<double>(rgb_img->GetReceivedDataSize())/static_cast<double>(rgb_img->GetRows());
  if(clone_data)
    mat = cv::Mat(rgb_img->GetRows(), rgb_img->GetCols(), CV_8UC3, rgb_img->GetData(), row_bytes).clone();
  else
    mat = cv::Mat(rgb_img->GetRows(), rgb_img->GetCols(), CV_8UC3, rgb_img->GetData(), row_bytes);
}


void PrintBuildInfo(){
	FlyCapture2::FC2Version fc2Version;
	FlyCapture2::Utilities::GetLibraryVersion(&fc2Version);

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "."
          << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	std::cout << version.str() <<std::endl;
}


void PrintCameraInfo(FlyCapture2::CameraInfo *cam_info){
	std::cout << std::endl;
	std::cout << "Serial number -" << cam_info->serialNumber << std::endl;
	std::cout << "Camera model - " << cam_info->modelName << std::endl;
	std::cout << "Camera vendor - " << cam_info->vendorName << std::endl;
	std::cout << "Sensor - " << cam_info->sensorInfo << std::endl;
	std::cout << "Resolution - " << cam_info->sensorResolution << std::endl;
	std::cout << "Firmware version - " << cam_info->firmwareVersion << std::endl;
	std::cout << "Firmware build time - " << cam_info->firmwareBuildTime << std::endl << std::endl;
}


bool PollForTriggerReady(FlyCapture2::Camera *cam){
  PGR_ERR_VAR
	const unsigned int kSoftwareTrigger = 0x62C;
	unsigned int register_val = 0;
	do{
    PGR_ERR_OK(cam->ReadRegister(kSoftwareTrigger, &register_val), return(false))
	}while((register_val >> 31) != 0);

	return true;
}


int InitPtGreyFlyCap2Cam(FlyCapture2::PGRGuid &guid, FlyCapture2::Camera &cam, const bool with_ext_trig = false){
  PGR_ERR_VAR

  PGR_ERR_OK(cam.Connect(&guid));

	// Power on the camera
	const unsigned int kCameraPower = 0x610;
	const unsigned int kPowerVal = 0x80000000;
  PGR_ERR_OK(cam.WriteRegister(kCameraPower, kPowerVal), return(-1))
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
		if (read_register_error == PGRERROR_TIMEOUT){}
    else{
      PGR_ERR_OK(read_register_error, return(-1))
		}

		retries--;
	} while ((register_val & kPowerVal) == 0 && retries > 0);
	// Check for timeout errors after retrying
  PGR_EXP_CHK(read_register_error == PGRERROR_TIMEOUT, return(-1))

	// Get the camera information
	FlyCapture2::CameraInfo cam_info;
  PGR_ERR_OK(cam.GetCameraInfo(&cam_info), return(-1))
	PrintCameraInfo(&cam_info);

  if(with_ext_trig){
	  // Check for external trigger support
	  FlyCapture2::TriggerModeInfo trigger_mode_info;
    PGR_ERR_OK(cam.GetTriggerModeInfo(&trigger_mode_info), return(-1))
    ERRNO_CHK_EM(trigger_mode_info.present == true, return(-1), "External trigger mode not supported")

	  // Get current trigger settings
	  FlyCapture2::TriggerMode trigger_mode;
    PGR_ERR_OK(cam.GetTriggerMode(&trigger_mode), return(-1))
	  // Set camera to trigger mode 0
	  trigger_mode.onOff = true;
	  trigger_mode.mode = 0;
	  trigger_mode.parameter = 0;
	  // Triggering the camera externally using source 0.
	  trigger_mode.source = 0;
    PGR_ERR_OK(cam.SetTriggerMode(&trigger_mode), return(-1))

	  // Poll to ensure camera is ready
    EXP_CHK_EM(PollForTriggerReady(&cam), return(-1), "error polling for trigger ready")
	  // Get the camera configuration
	  FC2Config config;
    PGR_ERR_OK(cam.GetConfiguration(&config), return(-1))
	  config.grabTimeout = 5000;	// Set the grab timeout to 5 seconds
	  // Set the camera configuration
    PGR_ERR_OK(cam.SetConfiguration(&config), return(-1))
  }

  // Camera is ready, start capturing images
  PGR_ERR_OK(cam.StartCapture(), return(-1));
  printf("trigger GPIO: %d\n", trigger_mode.source);
  return 0;
}


int UninitPtGreyFlyCap2Cam(FlyCapture2::Camera &cam, const bool with_ext_trig = false){
  PGR_ERR_VAR

  if(with_ext_trig){
  	FlyCapture2::TriggerMode trigger_mode;
    PGR_ERR_OK(cam.GetTriggerMode(&trigger_mode), return(-1))
  	trigger_mode.onOff = false;
    PGR_ERR_OK(cam.SetTriggerMode(&trigger_mode), return(-1))
  }
  PGR_ERR_OK(cam.StopCapture(), return(-1))
  PGR_ERR_OK(cam.Disconnect(), return(-1))
  return 0;
}

} //namespace mio

#endif //__MIO_PT_GREY_FLY_CAP__
