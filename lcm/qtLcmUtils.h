#ifndef __QT_LCM_UTILS_H__
#define __QT_LCM_UTILS_H__

#include <lcm/lcm.h>
#include <QSocketNotifier> //need to link against Qt5::Core


//#define QT_LCM_SOCKET_NOTIFIER_SLOT(class, lcm_object, lcm_fd) \
//void DataReady(int fd){lcm_handle_to(class::lcm_object, class::lcm_fd, 1);}

#define QT_LCM_SOCKET_NOTIFIER_SLOT(class, lcm_object) void DataReady(int fd){ lcm_handle(class::lcm_object); }


// 1. A Qt socket notifier watches the lcm file descriptor
// 2. When something changes in the file descriptor, the socket notifier calls DataReady(int)
// 3. DataReady(int) calls lcm_handle(), which calls the respective lcm subscriptions
#define INIT_QT_LCM_SOCKET_NOTIFIER(class, lcm_object, lcm_fd, socket_notifier, numeric_id)       \
if(class::lcm_object == NULL)                                                                     \
  class::lcm_object = lcm_create(NULL);                                                           \
EXP_CHK_EM( class::lcm_object != NULL, return, std::string("id=") + std::to_string(numeric_id) ); \
class::lcm_fd = lcm_get_fileno(class::lcm_object);                                                \
class::socket_notifier = new QSocketNotifier(class::lcm_fd, QSocketNotifier::Read, this);         \
connect( class::socket_notifier, SIGNAL( activated(int) ), this, SLOT( DataReady(int) ) );

#define UNINIT_QT_LCM_SOCKET_NOTIFIER(class, lcm_object, socket_notifier)                     \
disconnect( class::socket_notifier, SIGNAL( activated(int) ), this, SLOT( DataReady(int) ) ); \
delete class::socket_notifier;                                                                \
lcm_destroy(class::lcm_object);

#endif //__QT_LCM_UTILS_H__

