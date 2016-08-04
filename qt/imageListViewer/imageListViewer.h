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
    AdvImageDisplay *adv_img_disp_;

    explicit ImageListViewer(const bool show_earth = false, QWidget *parent = 0);
    ~ImageListViewer();
    void SetImageList(const std::string file_path, const std::vector<std::string> &img_file_name_vec);
    void SetImageList(const std::vector<std::string> &img_file_name_vec,
                      const std::vector<cv::Mat> &img_vec, const bool clone_images = true);
    void ShowEarth();

  private:
    Ui::ImageListViewer *ui;
    cv::Mat cv_mat_;
    std::vector<cv::Mat> img_vec_;
    std::vector<std::string> img_file_name_vec_;
    void SetImgIdxGui();

  private slots:
    void SetImage(const int idx);
    void DecrementImgIdxSlider();
    void IncrementImgIdxSlider();
    void AddRoi();
    void RemoveRoi();
    void ShowRoi();
};

#endif // IMAGELISTVIEWER_H

