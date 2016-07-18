#ifndef __MIO_ADV_IMAGE_DISPLAY_H__
#define __MIO_ADV_IMAGE_DISPLAY_H__

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
//#include <QMessageBox>
//#include <QFileDialog>
//#include <QScrollBar>

#include <queue>
#include <array>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "mio/altro/algorithm.h"
#include "mio/altro/opencv.h"
#include "mio/math/math.h"
#include "mio/altro/io.h"
#include "mio/altro/casting.h"
#include "mio/altro/error.h"
#include "mio/altro/thread.h"
#include "mio/qt/cvMatToQImage.h"
#include "mio/qt/qtMetaTypes.h" //zoomInfo_t

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"

//#include "mio/lcm/lcmTypes.h"
//#include "mio/lcm/qtLcmUtils.h"
//#include "mio/lcm/lcmTypes.h"
//#include "mio/ipc/shMem.h"
//#include "mio/ipc/sem.h"
//#include "mio/ipc/macro.h"
//#include "mio/lcm/lcmUtils.h"

/*#include "videoDisplayWidget.h"*/
/*#include "imgProcQueue.h"*/
/*#include "videoCVWidget.h"*/
/*#ifdef HAVE_QWT*/
/*#include "histogram.h"*/
/*#endif*/
/*#include "videoCVWidgetCVFunc.h"*/
/*#include "icvLcmTypes.h"*/
/*#include "videoCVDisplay.h"*/
/*#include "settingEditor.h"*/
/*#include "infraCvConfig.h"*/
/*#include "ui_videoCVWidget.h"*/
/*#include "infraCvConfig.h"*/
/*#include "lcmChannels.h"*/


struct Roi{
  static const int ROI_RECT = 0,
                   ROI_POLY = 1,
                   ROI_CENTER_CIRCLE = 2,
                   ROI_DRAG_CIRCLE = 3;
  static constexpr std::array<const char*, 4> roi_type_str = {"rect", "poly", "cent-circ", "drag-circ"};

  std::mutex mutex;
  bool creating_flag;
  int type;
  std::vector<cv::Point2f> vertices;

  Roi() : creating_flag(false), type(-1) {};
};


struct SBackEndConvert{
  bool perform;
  float scale;
};


class AdvImageDisplay : public QWidget{
  Q_OBJECT
  QGridLayout *layout_;
  QLabel *label_;
  QImage qt_disp_img_;
  cv::Mat cv_disp_img_;

  Roi roi_data_;
  std::mutex roi_mask_mtx_;
  cv::Mat roi_mask_, roi_mask_resize_;
  std::vector<Roi> roi_vec_;
  std::vector< std::vector<cv::Point> > roi_polygons_tmp_;
  bool create_roi_, normalize_roi_;
  void DrawRoi(cv::Mat &img);
  void CreateRoiMask();

  cv::Point2f View2Image(const cv::Point2f &view_coord);
  cv::Point2f Image2View(const cv::Point2f &img_coord);
  void Image2View(const std::vector<cv::Point2f> &src, std::vector<cv::Point> &dst, const bool scale_vertices);

  void UpdateDisplay();


  zoomInfo_t zoom_info_, zoom_info_tmp_;
  cv::Mat zoom_img_, final_img_;
  float zoom_scaler_, prev_zoom_, max_scale_inv_;
  cv::Point2f origin_, origin_bounded_;
  cv::Size2f zoom_region_size_;
  bool is_zoom_;
  void UpdateZoom();
  void ResetZoom();

  float resize_fx_total_, resize_fy_total_;
  cv::Size prev_src_img_size_;
  std::mutex resize_total_mutex_;
  bool normalize_img_, convert_to_false_colors_;

  SBackEndConvert bec_;

  bool is_init_;

  void InitCreateRoi();
  void UpdateRoiMask();

  void ShowRoi();

  protected:
    int id_;
    std::string m_new_frame_lcm_chan_name;
    std::mutex cv_disp_img_mtx_;
    //TODO - there are two boolean flags to stop video display show_image_ and pause_display_
    bool show_image_, pause_display_, limit_view_;

  public:
    AdvImageDisplay(QWidget *parent = NULL);
    ~AdvImageDisplay();

    void SetImage(const cv::Mat &img, const bool clone);
    void SetResizeScale(const float height_scale, const float width_scale){};
    void SetMaxImgSize(cv::Size size){};
    void GetRoi(const size_t idx, Roi &roi){};
    void GetRoi(const size_t idx, cv::Mat &roi){};
    void CreateRoi(); //void AddRoi(const Roi &roi){};
    void ClearRoi(){};
    void RemoveRoi();
    void ShowStripes();

    void UpdateZoom(zoomInfo_t &zoomInfo);
    void SetBackEndNorm(bool);
    bool GetBackEndNorm();
    void SetBackEndNormROI(bool);
    bool GetBackEndNormROI();
    void SetBackEndFalseColors(bool);
    bool GetBackEndFalseColors();
    void SetLimitView(bool);
    bool GetLimitView();
    void ResetResizeTotal();

    void Init(const int id, const bool manage_layout = true);
    int GetID();
    void SetShowImage(bool);
    bool GetShowImage();
    void SetPauseDisplay(bool);
    bool GetPauseDisplay();
    void SetBackEndConvert(bool);
    bool GetBackEndConvert();
    void SetBackEndScale(float);
    float GetBackEndScale();
    QLabel* GetImageQLabel();

  protected:
    bool eventFilter(QObject *target, QEvent *event);
};

#endif //__MIO_ADV_IMAGE_DISPLAY_H__

