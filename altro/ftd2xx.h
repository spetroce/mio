#ifndef __MIO_FTD2XX_H__
#define __MIO_FTD2XX_H__

//these are the error strings fod the FTDI D2XX library: libftd2xx-x86_64-1.3.6
//NOTE: include this file after you include ftd2xx.h

namespace mio{

inline std::string ftd2xx_err_str(const int err_idx){
  EXP_CHK(err_idx >= 0 && err_idx <= 18, return(std::string("d2xx_err_str() - invalid error index")))
  const char *error_string[] = {"FT_OK",
                                "FT_INVALID_HANDLE",
                                "FT_DEVICE_NOT_FOUND",
                                "FT_DEVICE_NOT_OPENED",
                                "FT_IO_ERROR",
                                "FT_INSUFFICIENT_RESOURCES",
                                "FT_INVALID_PARAMETER",
                                "FT_INVALID_BAUD_RATE",
                                "FT_DEVICE_NOT_OPENED_FOR_ERASE",
                                "FT_DEVICE_NOT_OPENED_FOR_WRITE",
                                "FT_FAILED_TO_WRITE_DEVICE",
                                "FT_EEPROM_READ_FAILED",
                                "FT_EEPROM_WRITE_FAILED",
                                "FT_EEPROM_ERASE_FAILED",
                                "FT_EEPROM_NOT_PRESENT",
                                "FT_EEPROM_NOT_PROGRAMMED",
                                "FT_INVALID_ARGS",
                                "FT_NOT_SUPPORTED",
                                "FT_OTHER_ERROR"};
  return std::string(error_string[err_idx]);
}


//returns true if a timeout occured
bool d2xx_timeout(FT_STATUS ftStatus, DWORD dwBytesToRead, DWORD dwBytesReturned){
  return (ftStatus == FT_OK && dwBytesToRead != dwBytesReturned);
}

}


#endif //__MIO_FTD2XX_H__

