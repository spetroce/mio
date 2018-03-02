#ifndef __MIO_ADV_IMAGE_DISPLAY_H__
#define __MIO_ADV_IMAGE_DISPLAY_H__

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QSocketNotifier> //need to link against Qt5::Core

#include <array>
#include <unistd.h>
#include <mutex>

#include "mio/altro/algorithm.h"
#include "mio/altro/opencv.h"
#include "mio/math/math.h"
#include "mio/altro/error.h"
#include "mio/qt/cv_mat_to_qimage.h"
#ifdef HAVE_LCM
#include "mio/lcm/lcm_types.h"
#endif

#if CV_MAJOR_VERSION < 3
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp" //applyColorMap()
#else
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#endif

#if CV_MAJOR_VERSION < 3
namespace cv{
typedef Size_<double> Size2d;
}
#endif

namespace mio{

struct Roi{
  static const int ROI_RECT = 0,
                   ROI_POLY = 1,
                   ROI_CENTER_CIRCLE = 2,
                   ROI_DRAG_CIRCLE = 3;
  static constexpr std::array<const char*, 4> roi_type_str = { {"rect", "poly", "cent-circ", "drag-circ"} };

  int type;
  std::vector<cv::Point2d> vertices;

  Roi() : type(-1) {};
};


class AdvImageDisplay : public QWidget{
  Q_OBJECT

  public:
    AdvImageDisplay(QWidget *parent = NULL);
    ~AdvImageDisplay();

    void Init(const int id, const bool manage_layout = true);
    void SetImage(const cv::Mat &kImg, const bool kClone, const bool kCalledFromExternalThread = false);
    //void SetResizeScale(const double height_scale, const double width_scale);
    void CreateRoi(const int roi_type);
    void CancelCreateRoi();
    void RemoveRoi(const int kRoiIdx);
    void GetRoiMask(cv::Mat &roi, cv::Size resize_to = cv::Size());
    void ShowStripes();
    void ClearDisplay();
    void ShowRoi();
    void SetNormalizeImage(const bool kState);
    bool GetNormalizeImage();
    void SetNormalizeRoi(const bool kState);
    bool GetNormalizeRoi();
    void SetConvertToFalseColors(const bool kState);
    bool GetConvertToFalseColors();
    void SetLimitView(const bool kState, const int kMaxDispImgDim = 512);
    bool GetLimitView();
    int GetID();
    void SetShowImage(const bool kState);
    bool GetShowImage();
    QLabel* GetImageQLabel();
    void SetAutoConvertImage(const bool kState);
    bool GetAutoConvertImage();
    void SetDrawClicks(const bool kSet);
    bool GetDrawClicks();
    void GetClickPnts(std::vector<cv::Point2d> &pnt_vec);
    void ClearClickPntBuffer();
    void SetRoiIndex(const int kRoiIdx);
    int GetRoiIndex();
    void SaveRoi(QString file_full_qstr);
    void LoadRoi(const QString kFileFullQStr, std::vector<int> &loaded_roi_types);
    bool SetZoomingEnabled(const bool kEnabled);
    void SetupLcm(const std::string kNewFrameLcmChanNamePrefix);

  protected:
    bool eventFilter(QObject *target, QEvent *event);

  private:
    bool is_init_, show_image_, limit_view_, normalize_img_, convert_to_false_colors_, auto_convert_img_;
    int id_, limit_view_max_img_dim_;
    std::mutex src_img_mtx_;
    QGridLayout *layout_;
    QLabel *label_;
    QImage qt_src_img_;
    cv::Mat src_img_, disp_img_;
    cv::Size2d prev_src_img_size_;
    std::vector<cv::Point2d> mouse_click_pnt_vec_;
    bool draw_mouse_clicks_;
    bool mouse_button_pressed_;
    cv::Point2d mouse_button_press_init_pos_, mouse_drag_;
#ifdef HAVE_LCM
    lcm_t *lcm_;
    lcm_opencv_mat_t_subscription_t *new_frame_lcm_sub_;
    QSocketNotifier *socket_notifier_;
    int lcm_fd_;
    bool lcm_is_init_;
#endif

    cv::Point2d ViewToImage(const cv::Point2d &kViewPnt);
    cv::Point2d ImageToView(const cv::Point2d &kImgPnt);
    void ImageToView(const std::vector<cv::Point2d> &src, std::vector<cv::Point> &dst, const bool kScaleVertices);

    bool create_roi_, normalize_roi_, show_roi_;
    double resize_fx_total_, resize_fy_total_;
    Roi temp_roi_;
    std::mutex resize_total_mtx_, roi_mask_mtx_, temp_roi_mtx_, roi_vec_mtx_;
    std::vector<Roi> roi_vec_;
    int roi_idx_;
    cv::Mat roi_mask_, temp_roi_mask_;
    void AddRoi();
    void DrawRoi(const Roi &kRoi, cv::Mat &img, const bool kClosePoly);
    void DrawRoi(cv::Mat &img);
    void InitCreateRoi();
    void UpdateRoiMask();
    void ResetResizeTotal();

    bool is_zoom_, zooming_enabled_;
    int scroll_wheel_count_;
    double zoom_scalar_, prev_zoom_, display_img_dim_scalar_inv_;
    cv::Mat zoom_img_;
    cv::Point2d origin_, clamped_origin_, pixmap_mouse_pos_;
    cv::Size2d zoom_region_size_;
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

} //namespace mio

#endif //__MIO_ADV_IMAGE_DISPLAY_H__

