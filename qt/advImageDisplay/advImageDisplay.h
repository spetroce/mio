#ifndef __MIO_ADV_IMAGE_DISPLAY_H__
#define __MIO_ADV_IMAGE_DISPLAY_H__

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QSocketNotifier> //need to link against Qt5::Core
//#include <QMessageBox>
//#include <QFileDialog>
//#include <QScrollBar>

#include <queue>
#include <array>
#include <unistd.h>
#include <mutex>

#include "mio/altro/algorithm.h"
#include "mio/altro/opencv.h"
#include "mio/math/math.h"
#include "mio/altro/error.h"
#include "mio/qt/cvMatToQImage.h"
#ifdef HAVE_LCM
#include "mio/lcm/lcmTypes.h"
#endif

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#if CV_MAJOR_VERSION < 3
#include "opencv2/contrib/contrib.hpp" //applyColorMap()
#endif


struct Roi{
  static const int ROI_RECT = 0,
                   ROI_POLY = 1,
                   ROI_CENTER_CIRCLE = 2,
                   ROI_DRAG_CIRCLE = 3;
  static constexpr std::array<const char*, 4> roi_type_str = { {"rect", "poly", "cent-circ", "drag-circ"} };

  std::mutex mutex;
  int type;
  std::vector<cv::Point2f> vertices;

  Roi() : type(-1) {};
};


class AdvImageDisplay : public QWidget{
  Q_OBJECT

  public:
    AdvImageDisplay(QWidget *parent = NULL);
    ~AdvImageDisplay();

    void Init(const int id, const bool manage_layout = true);
    void SetImage(const cv::Mat &kImg, const bool kClone, const bool kCalledFromExternalThread = false);
    //void SetResizeScale(const float height_scale, const float width_scale);
    void BeginCreateRoi(const int roi_type);
    void AddRoi();
    void RemoveRoi();
    void GetRoiMask(cv::Mat &roi, cv::Size resize_to = cv::Size());
    void ShowStripes();
    void ShowRoi();
    void SetNormalizeImage(const bool state);
    bool GetNormalizeImage();
    void SetNormalizeRoi(const bool state);
    bool GetNormalizeRoi();
    void SetConvertToFalseColors(const bool state);
    bool GetConvertToFalseColors();
    void SetLimitView(const bool state, const int max_disp_img_dim = 512);
    bool GetLimitView();
    int GetID();
    void SetShowImage(const bool state);
    bool GetShowImage();
    QLabel* GetImageQLabel();
    void SetAutoConvertImage(const bool state);
    bool GetAutoConvertImage();
    void SetDrawClicks(const bool kSet);
    bool GetDrawClicks();
    void GetClickPnts(std::vector<cv::Point2f> &pnt_vec);
    void ClearClickPntBuffer();
    void SaveRoi(QString file_full_qstr);
    void LoadRoi(const QString file_full_qstr);
    bool SetZoomingEnabled(const bool kEnabled);
    void SetupLcm(const std::string kNewFrameLcmChanNamePrefix);

  protected:
    bool eventFilter(QObject *target, QEvent *event);

  private:
    bool is_init_, show_image_, limit_view_, normalize_img_, convert_to_false_colors_, auto_convert_img_;
    int id_, max_disp_img_dim_;
    std::mutex src_img_mtx_;
    QGridLayout *layout_;
    QLabel *label_;
    QImage qt_src_img_;
    cv::Mat src_img_, disp_img_;
    cv::Size prev_src_img_size_;
    std::vector<cv::Point2f> mouse_click_pnt_vec_;
    bool draw_mouse_clicks_;
#ifdef HAVE_LCM
    lcm_t *lcm_;
    lcm_opencv_mat_t_subscription_t *new_frame_lcm_sub_;
    QSocketNotifier *socket_notifier_;
    int lcm_fd_;
    bool lcm_is_init_;
#endif

    cv::Point2f ViewToImage(const cv::Point2f &view_coord);
    cv::Point2f ImageToView(const cv::Point2f &img_coord);
    void ImageToView(const std::vector<cv::Point2f> &src, std::vector<cv::Point> &dst, const bool scale_vertices);

    bool create_roi_, normalize_roi_, show_roi_;
    float resize_fx_total_, resize_fy_total_;
    Roi src_roi_, disp_roi_;
    std::mutex resize_total_mtx_, roi_mask_mtx_;
    //std::vector<Roi> roi_vec_;
    //std::vector< std::vector<cv::Point> > roi_polygons_tmp_;
    cv::Mat roi_mask_, disp_roi_mask_;
    void DrawRoi(cv::Mat &img);
    void CreateRoiMask();
    void InitCreateRoi();
    void UpdateRoiMask();
    void ResetResizeTotal();

    bool is_zoom_, zooming_enabled_;
    int scroll_wheel_count_;
    float zoom_scaler_, prev_zoom_, max_disp_img_dim_scale_inv_;
    cv::Mat zoom_img_;
    cv::Point2f origin_, origin_bounded_, pixmap_mouse_pos_;
    cv::Size2f zoom_region_size_;
    void UpdateZoom();
    void ResetZoom();
#ifdef HAVE_LCM
    static void NewFrameLCM(const lcm_recv_buf_t *rbuf, const char * channel, 
                            const lcm_opencv_mat_t * msg, void * userdata);
#endif

  private slots:
    void UpdateDisplay();
#ifdef HAVE_LCM
    void DataReady(int fd){ lcm_handle(AdvImageDisplay::lcm_); }
#endif

  signals:
    void ExternalDisplayUpdate(void);
};

#endif //__MIO_ADV_IMAGE_DISPLAY_H__

