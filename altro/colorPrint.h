#ifndef __MIO_COLOR_PRINT_H__
#define __MIO_COLOR_PRINT_H__

#include <stdio.h>

namespace mio{

/*
22, 30 - black
22, 31 - red
22, 32 - green
22, 33 - brown
22, 34 - blue
22, 35 - magenta
22, 36 - cyan
22, 37 - gray
01, 30 - dark gray
01, 31 - light red
01, 32 - light green
01, 33 - yellow
01, 34 - light blue
01, 35 - light magenta
01, 36 - light cyan
01, 37 - white
*/
inline void printf_color(const char *str, const int color_attr_a, const int color_attr_b){
  printf("%c[%d;%dm%s%c[%dm\n", 0x1B, color_attr_a, color_attr_b, str, 0x1B, 0);
}

inline void printf_color(const std::string str, const int color_attr_a, const int color_attr_b){
  printf("%c[%d;%dm%s%c[%dm\n", 0x1B, color_attr_a, color_attr_b, str.c_str(), 0x1B, 0);
}

} //namespace mio

#endif //__MIO_COLOR_PRINT_H__

