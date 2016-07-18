#ifndef IMAGELISTVIEWER_H
#define IMAGELISTVIEWER_H

#include <QWidget>
#include "mio/qt/advImageDisplay/advImageDisplay.h"


namespace Ui {
  class ImageListViewer;
}


class ImageListViewer : public QWidget {
  Q_OBJECT

  public:
    explicit ImageListViewer(const bool show_earth = false, QWidget *parent = 0);
    ~ImageListViewer();
    void SetImageList(const std::string file_path, const std::vector<std::string> &img_file_name_vec);
    void SetImageList(const std::vector<std::string> &img_file_name_vec,
                      const std::vector<cv::Mat> &img_vec, const bool clone_images = true);

  private:
    Ui::ImageListViewer *ui;
    //QImage qt_img_;
    cv::Mat cv_mat_;
    AdvImageDisplay *adv_img_disp_;
    std::vector<cv::Mat> img_vec_;
    std::vector<std::string> img_file_name_vec_;

  private slots:
    void SetImage(const int idx);
    void DecrementImgIdxSlider();
    void IncrementImgIdxSlider();
};

#endif // IMAGELISTVIEWER_H

