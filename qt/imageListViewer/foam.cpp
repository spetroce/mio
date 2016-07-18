#include <stdio.h>
#include "mio/altro/io.hpp"


class Foam {
  std::vector<mio::FileDescript> img_file_desc_vec_[3];
  std::vector<cv::Mat> img_vec_[3];
};


Foam::LoadImagesFromDisk(std::string file_ext){
  QString load_dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), 
                                                       QString(image_dir_), QFileDialog::ShowDirsOnly);
  mio::FormatFilePath(dir_path);
  mio::FormatFileExt(file_ext);
  const size_t file_ext_size = file_ext.size();

  //get directory listing
  std::vector<std::string> dir_list_vec;
  mio::GetDirList(dir_path, dir_list_vec, true);
  for(size_t i = 0; i < 3; ++i)
    img_file_desc_vec_[i].reserve(dir_list_vec.size());

  std::vector<std::string> file_prefix_vec = {"R__", "G__", "B__"};

  const size_t num_prefix = file_prefix_vec.size();
  std::vector< std::vector<std::string> > img_file_name_vec(num_prefix);
  for(auto &dir_item : dir_list_vec)
    for(size_t j = 0; j < num_prefix; ++j)
      if( (dir_item.compare(0, file_prefix_vec[j].size(), file_prefix_vec[j]) == 0) && //check file prefix
          (dir_item.compare(dir_item.size() - file_ext_size, file_ext_size, file_ext) == 0) ){ //check file extension
        mio::FileDescript file_desc;
        file_desc.file_name_ = dir_item;
        mio::TimeStamp ts;
        sscanf(dir_item.c_str(), "%*s__%d-%d-%d__%d-%d-%d.%*s", ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
        file_desc.epoch_time_ = mio::EpochTime(ts);
        img_file_desc_vec_[j].push_back(file_desc);
      }

  for(size_t i = 0; i < 3; ++i){
    std::stable_sort(img_file_desc_vec_.begin(), img_file_desc_vec_.end(), mio::FileDescript::epoch_time_less_than);
    const size_t img_file_desc_vec_size = img_file_desc_vec_[i].size();
    img_vec_[i].resize(img_file_desc_vec_size);
    for(size_t j = 0; j < img_file_desc_vec_size; ++j)
      cv::imread(load_dir + "/" + img_file_desc_vec_[i][j].file_name, img_vec_[i][j]);
  }

  if(m_cal_img_vec.size() > 0){
    m_img_size = m_cal_img_vec[0].image.size();
    ui->pushButton_prev->setEnabled(true);
    ui->pushButton_next->setEnabled(true);
    ui->slider_display->setEnabled(true);
    ui->slider_display->setValue(0);
    ui->slider_display->setMaximum(m_cal_img_vec.size() - 1);
    UpdateDisplay(0);
  }
}


void Foam::UpdateDisplay(int idx){
  const double PIXEL_SCALE = g_pixel_scale[ ui->comboBox_pixDepth->currentIndex() ];
  if( idx <= m_cal_img_vec.size() ){
    ui->lineEdit_display->setText( QString( m_cal_img_vec[idx].file_descript.file_name.c_str() ) );
    m_cal_img_vec[idx].image.convertTo(m_disp_img, CV_8U, PIXEL_SCALE);
    //cv::rectangle(m_disp_img, m_mouse_pos_start, m_mouse_pos_end, cv::Scalar::all(255), 3);
    //cv::circle(m_disp_img, m_mouse_pos_start, sm::VerDist2(m_mouse_pos_start, m_mouse_pos_end), cv::Scalar::all(255), 3);
    m_circle_roi_center = sm::MidPoint2(m_mouse_pos_start, m_mouse_pos_end);
    m_circle_roi_radius = sm::VerDist2(m_mouse_pos_start, m_mouse_pos_end) / 2.0f;
    cv::circle(m_disp_img, m_circle_roi_center, m_circle_roi_radius, cv::Scalar::all(255), 3);
    OpenCVMat2QImage(m_disp_img, m_qt_img, true);
    ui->label_display->setPixmap( QPixmap::fromImage(m_qt_img) );
    qApp->processEvents();
  }
  else
    printf("Foam::UpdateDisplay() - invalid index\n");
}



bool Foam::eventFilter(QObject *target, QEvent *event){
  if(target == ui->label_display){
    switch( event->type() ){
      case QEvent::MouseMove:
        if(m_create_roi){
          QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
          m_mouse_pos_end = cv::Point2f( mouseEvent->pos().x(), mouseEvent->pos().y() );
          UpdateDisplay( ui->slider_display->value() );
        }
        break;
      case QEvent::MouseButtonPress:
        {
          QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
          //if(mouseEvent->button() == Qt::RightButton)
          m_create_roi = true;
          m_mouse_pos_start = m_mouse_pos_end = cv::Point2f( mouseEvent->pos().x(), mouseEvent->pos().y() );
          UpdateDisplay( ui->slider_display->value() );
          break;
        }
      case QEvent::MouseButtonRelease:
        {
          QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
          m_create_roi = false;
          m_mouse_pos_end = cv::Point2f( mouseEvent->pos().x(), mouseEvent->pos().y() );
          UpdateDisplay( ui->slider_display->value() );
          BuildRoiMask();
          break;
        }
    }
  }

  return QWidget::eventFilter(target, event);
}


void Foam::BuildRoiMask(){
  m_roi_mask = cv::Mat::zeros(m_img_size, CV_8U);
  //cv::Rect roi(pix_x, pix_y, roi_width, roi_height);
  //cv::rectangle(m_roi_mask, roi, cv::Scalar::all(255), cv::FILLED);
  cv::circle(m_roi_mask, m_circle_roi_center, m_circle_roi_radius, cv::Scalar::all(255), 
#if ICV_OPENCV_VERSION_MAJOR < 3
CV_FILLED);
#else
cv::FILLED);
#endif
}


void Foam::DecrementDisplaySlider(){
  ui->slider_display->setValue(ui->slider_display->value() - 1);
}

void Foam::IncrementDisplaySlider(){
  ui->slider_display->setValue(ui->slider_display->value() + 1);
}


int main (){
  char sentence []="Rudolph is 12 years old";
  char str [20];
  int i;

  sscanf (sentence,"%s %*s %d",str,&i); 
  printf ("%s -> %d\n",str,i);
  
  return 0;
}

