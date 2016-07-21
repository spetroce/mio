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


struct Roi{
  static const int ROI_RECT = 0,
                   ROI_POLY = 1,
                   ROI_CENTER_CIRCLE = 2,
                   ROI_DRAG_CIRCLE = 3;
  static constexpr std::array<const char*, 4> roi_type_str = {"rect", "poly", "cent-circ", "drag-circ"};

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
    void SetImage(const cv::Mat &img, const bool clone);
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

    void UpdateDisplay();
    cv::Point2f View2Image(const cv::Point2f &view_coord);
    cv::Point2f Image2View(const cv::Point2f &img_coord);
    void Image2View(const std::vector<cv::Point2f> &src, std::vector<cv::Point> &dst, const bool scale_vertices);

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

    bool is_zoom_;
    int scroll_wheel_count_;
    float zoom_scaler_, prev_zoom_, max_disp_img_dim_scale_inv_;
    cv::Mat zoom_img_;
    cv::Point2f origin_, origin_bounded_, pixmap_mouse_pos_;
    cv::Size2f zoom_region_size_;
    void UpdateZoom();
    void ResetZoom();
};

#endif //__MIO_ADV_IMAGE_DISPLAY_H__

