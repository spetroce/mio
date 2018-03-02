#ifndef __META_TYPES_H__
#define __META_TYPES_H__

#include <QMetaType>
#if ICV_OPENCV_VERSION_MAJOR < 3
#include "opencv2/core/core.hpp"
#else
#include "opencv2/core.hpp"
#endif


typedef struct ZoomInfo{
  int scroll_wheel_count;
  cv::Point2f pixmap_mouse_pos;
} zoomInfo_t;

Q_DECLARE_METATYPE(zoomInfo_t)

Q_DECLARE_METATYPE(cv::Mat)

#endif //__META_TYPES_H__
