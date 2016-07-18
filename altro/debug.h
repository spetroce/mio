#include "mio/altro/error.h"

#ifndef MIO_DBG_PRINT_CNTR_VAR
static size_t MIO_DBG_PRINT_CNTR = 0;
#define MIO_DBG_PRINT_CNTR_VAR
#endif

#undef DBG_PRINT
#undef DBG_PRINT_M

#ifdef MIO_DEBUG
#define DBG_PRINT                                                                                                       \
{                                                                                                                       \
const std::string file_full(__FILE__);                                                                                  \
const std::string file_name = file_full.substr(file_full.find_last_of("/\\") + 1);                                      \
std::cout << "DBG[" << MIO_DBG_PRINT_CNTR << "] : " << file_name << " : " << __LINE__ << " : " << CURRENT_FUNC << "\n"; \
++MIO_DBG_PRINT_CNTR;                                                                                                   \
}

#define DBG_PRINT_M(msg)                                                                 \
{                                                                                        \
const std::string file_full(__FILE__);                                                   \
const std::string file_name = file_full.substr(file_full.find_last_of("/\\") + 1);       \
std::cout << "DBG[" << MIO_DBG_PRINT_CNTR << "] : " << file_name << " : " << __LINE__ << \
             " : " << CURRENT_FUNC << " : " << msg << "\n";                              \
++MIO_DBG_PRINT_CNTR;                                                                    \
}

#else
#define DBG_PRINT ;
#define DBG_PRINT_M ;
#endif

#ifdef MIO_DEBUG
#undef MIO_DEBUG
#endif

