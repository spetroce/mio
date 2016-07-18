#include "advImageDisplay.h"


AdvImageDisplay::AdvImageDisplay(QWidget *parent) : QWidget(parent), id_(0), normalize_img_(false),
    normalize_roi_(false), convert_to_false_colors_(false), layout_(NULL), label_(NULL), show_image_(false),
    is_init_(false), limit_view_(false){
  // bec_.perform = false;
  // bec_.scale = 1.0f;
  prev_src_img_size_ = cv::Point(-1, -1);
  //init zoom variables
  zoom_info_.pixmap_mouse_pos = cv::Point2f(0, 0);
  zoom_info_.scroll_wheel_count = 0;
  zoom_info_tmp_.pixmap_mouse_pos = cv::Point2f(0, 0);
  zoom_info_tmp_.scroll_wheel_count = 0;
  ResetZoom();
}


AdvImageDisplay::~AdvImageDisplay(){
  cv_disp_img_ = cv::Mat();
  UpdateDisplay();
  //disconnect( this, SIGNAL( SignalUpdateDisplay(bool, bool) ), this, SLOT( UpdateDisplaySlot(bool, bool) ) );
  delete label_;
  label_ = NULL;
  if(layout_)
    delete layout_;
  layout_ = NULL;
}


void AdvImageDisplay::Init(const int id, const bool manage_layout){
  EXP_CHK_E(!is_init_, return)
  id_ = id;

  qRegisterMetaType< cv::Mat >("cv::Mat");
  label_ = new QLabel();
  if(manage_layout){
    layout_ = new QGridLayout();
    layout_->setSizeConstraint(QLayout::SetFixedSize);
    layout_->addWidget(label_);
    setLayout(layout_);
  }

  // Handle mouse events from int he video display QLabel
  label_->installEventFilter(this);
  label_->setMouseTracking(true);
  // Handle key press events, you must click on the video display so it can receive KeyPress events
  label_->setFocusPolicy(Qt::StrongFocus);

  SetupMat2QImage();
  is_init_ = true;
}


void AdvImageDisplay::SetImage(const cv::Mat &img, const bool clone){
  cv_disp_img_ = clone ? img.clone() : img;
  UpdateDisplay();
}


/*
All ROI vertices are saved in pixel coordinates in reference to the original unprocessed image.
The parameter mouse_pos is a pixel map coordinate and is repective to the final image displayed to the screen.
View2Image() removes effects caused by imgProcQueue resizing, view size limiting, and zooming.
*/
bool AdvImageDisplay::eventFilter(QObject *target, QEvent *event){
  if(target == label_){
    switch( event->type() ){
      case QEvent::MouseMove:
        {
          QMouseEvent *mouseEvent = mio::StaticCastPtr<QMouseEvent>(event);
          int x = mouseEvent->pos().x(),
              y = mouseEvent->pos().y();
          cv::Point2f processed_img_mouse_pos = View2Image( cv::Point2f(x, y) );
          bool update_display = false;
          disp_roi_.mutex.lock();
          const size_t roi_vertices_size = disp_roi_.vertices.size();
          if(create_roi_ && roi_vertices_size > 1){
            if(disp_roi_.type == Roi::ROI_RECT ||
               disp_roi_.type == Roi::ROI_CENTER_CIRCLE || disp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              STD_INVALID_ARG_E(roi_vertices_size == 2)
              disp_roi_.vertices[1] = processed_img_mouse_pos;
            }
            else if(disp_roi_.type == Roi::ROI_POLY)
              disp_roi_.vertices.back() = processed_img_mouse_pos;
            update_display = true;
          }
          disp_roi_.mutex.unlock();
          if(update_display) //this is to update the mouse_pos_value in statistics
            UpdateDisplay();
          break;
        }
      case QEvent::MouseButtonPress:
        {
          bool update_display = false;
          if(create_roi_){
            printf("QEvent::MouseButtonPress\n");
            QMouseEvent *mouseEvent = mio::StaticCastPtr<QMouseEvent>(event);
            const cv::Point2f mouse_pos = cv::Point2f( mouseEvent->pos().x(), mouseEvent->pos().y() );
            disp_roi_.mutex.lock();
            if(disp_roi_.type == Roi::ROI_RECT ||
               disp_roi_.type == Roi::ROI_CENTER_CIRCLE || disp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              disp_roi_.vertices.resize(2);
              disp_roi_.vertices[0] = disp_roi_.vertices[1] = View2Image(mouse_pos);
              update_display = true;
            }
            disp_roi_.mutex.unlock();
          }
          if(update_display)
            UpdateDisplay();
          break;
        }
      case QEvent::MouseButtonRelease:
        {
          printf("QEvent::MouseButtonRelease\n");
          bool update_display = false;
          if(create_roi_){
            disp_roi_.mutex.lock();
            if(disp_roi_.type == Roi::ROI_RECT ||
               disp_roi_.type == Roi::ROI_CENTER_CIRCLE || disp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              disp_roi_.mutex.unlock();
              AddRoi();
              disp_roi_.mutex.lock();
            }
            else if(disp_roi_.type == Roi::ROI_POLY){
              QMouseEvent *mouseEvent = mio::StaticCastPtr<QMouseEvent>(event);
              const cv::Point2f mouse_pos = cv::Point2f( mouseEvent->pos().x(), mouseEvent->pos().y() );
              if( disp_roi_.vertices.empty() ){
                disp_roi_.vertices.push_back( View2Image(mouse_pos) );
                disp_roi_.vertices.push_back( View2Image(mouse_pos) );
              }
              else
                disp_roi_.vertices.push_back( View2Image(mouse_pos) );
              update_display = true;
            }
            disp_roi_.mutex.unlock();
          }
          if(update_display)
            UpdateDisplay();
          break;
        }
      case QEvent::KeyPress:
        if(create_roi_){
          QKeyEvent *keyEvent = mio::StaticCastPtr<QKeyEvent>(event);
          const Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
          disp_roi_.mutex.lock();
          if(disp_roi_.type == Roi::ROI_POLY && disp_roi_.vertices.size() > 3 &&
             keyEvent->key() == Qt::Key_Space && modifiers == Qt::NoButton){
            disp_roi_.vertices.pop_back();
            disp_roi_.mutex.unlock();
            AddRoi();
          }
          else{
            disp_roi_.mutex.unlock();
            RemoveRoi();
          }
        }
        break;
      case QEvent::Wheel:
        {
          QWheelEvent *wheel_event = mio::StaticCastPtr<QWheelEvent>(event);
          zoom_info_tmp_.scroll_wheel_count += wheel_event->delta()/120;
          if(zoom_info_tmp_.scroll_wheel_count < 0)
            zoom_info_tmp_.scroll_wheel_count = 0;
          zoom_info_tmp_.pixmap_mouse_pos = cv::Point2f(wheel_event->x(), wheel_event->y());
          UpdateZoom(zoom_info_tmp_);
          UpdateDisplay();
          break;
        }
      default:
        break;
    }
  }
//  else if( (target == ui->scrollAreaWidgetContents_cv || target == ui->scrollAreaWidgetContents_misc) &&
//           event->type() == QEvent::Resize ){ //Scroll Widget Resize (only happens once on program startup)
//    ui->scrollArea_cv->setMinimumWidth( ui->scrollAreaWidgetContents_cv->minimumSizeHint().width() +
//                                        ui->scrollArea_cv->verticalScrollBar()->width() );
//    ui->scrollArea_misc->setMinimumWidth( ui->scrollAreaWidgetContents_misc->minimumSizeHint().width() +
//                                          ui->scrollArea_misc->verticalScrollBar()->width() );
//  }

  return QWidget::eventFilter(target, event);
}


cv::Point2f AdvImageDisplay::View2Image(const cv::Point2f &view_pnt){
  return cv::Point2f( (view_pnt.x*prev_zoom_ + origin_.x) * max_scale_inv_,
                      (view_pnt.y*prev_zoom_ + origin_.y) * max_scale_inv_ );
}


cv::Point2f AdvImageDisplay::Image2View(const cv::Point2f &img_pnt){
  return cv::Point2f( (img_pnt.x - origin_bounded_.x) / zoom_scaler_ / max_scale_inv_,
                      (img_pnt.y - origin_bounded_.y) / zoom_scaler_ / max_scale_inv_ );
}


void AdvImageDisplay::ResetZoom(){
  zoom_scaler_ = prev_zoom_ = max_scale_inv_ = 1.0;
  origin_ = cv::Point2f(0, 0);
  origin_bounded_ = cv::Point2f(0, 0);
  is_zoom_ = false;
}


/*this functions updates the following vairables based on the scroll wheel count:
bool is_zoom_
float zoom_scaler_ - scale factor ranging from 0 to 1
cv::Point2f zoom_region_size_ - lower right ROI coordinate
cv::Point2f origin_ - upper left ROI coordinate
float prev_zoom_ - the previous zoom_scaler_
*/
void AdvImageDisplay::UpdateZoom(zoomInfo_t &zoom_info){
  //m_mtx.lock();
    zoom_info_ = zoom_info;
    const float zoom_factor_step_size = 0.025f;
    if((is_zoom_ = zoom_info_.scroll_wheel_count > 0)) //if true, check for a minimum of 20 horizontal pixels
      while( cv_disp_img_.cols * (1.0f - static_cast<float>(zoom_info_.scroll_wheel_count)*zoom_factor_step_size) < 20 )
        if(zoom_info_.scroll_wheel_count-1 > 0)
          zoom_info_.scroll_wheel_count--;
        else{ //could not find a zoom factor yielding the 20 pixel minimum; don't zoom
          ResetZoom();
          break;
        }
    else
      ResetZoom();

    if(is_zoom_){
      //if zoom_info wheel count should take the updated value if it was too large
      zoom_info.scroll_wheel_count = zoom_info_.scroll_wheel_count;

      zoom_scaler_ = 1.0f - static_cast<float>(zoom_info_.scroll_wheel_count)*zoom_factor_step_size;
      cv::Size2f frameSize(cv_disp_img_.cols, cv_disp_img_.rows);
      zoom_region_size_ = cv::Point2f(frameSize.width*zoom_scaler_, frameSize.height*zoom_scaler_);

      //scale the mouse position by the last zoom_scaler_
      cv::Point2f scaledMouse = cv::Point2f(zoom_info_.pixmap_mouse_pos.x*prev_zoom_,
                                            zoom_info_.pixmap_mouse_pos.y*prev_zoom_);
      //find the new origin within the last scaled image and add it to the last origin_
      origin_ = cv::Point2f( origin_.x + ( scaledMouse.x - scaledMouse.x*(zoom_scaler_/prev_zoom_) ),
                              origin_.y + ( scaledMouse.y - scaledMouse.y*(zoom_scaler_/prev_zoom_) ) );

      prev_zoom_ = zoom_scaler_;
    }
  //m_mtx.unlock();
}


void AdvImageDisplay::UpdateDisplay(){
  //m_mtx.lock();
  resize_total_mutex_.lock();
  cv::Size src_img_size = cv_disp_img_.size();
  if(src_img_size != prev_src_img_size_){
    resize_fx_total_ *= static_cast<float>(src_img_size.width) / static_cast<float>(prev_src_img_size_.width);
    resize_fy_total_ *= static_cast<float>(src_img_size.height) / static_cast<float>(prev_src_img_size_.height);
  }
  prev_src_img_size_ = src_img_size;
  resize_total_mutex_.unlock();

  try{
    cv::Size max_img_size = cv_disp_img_.size();
    float max_dim_scale_factor;
    const bool is_scaled = limit_view_ ? mio::GetMaxSize(cv_disp_img_, 640, max_img_size, max_dim_scale_factor) : false;
    max_scale_inv_ = is_scaled ? 1.0f/max_dim_scale_factor : 1.0f;

    if(is_zoom_){ //ROI with original size or maxSize resizing
      origin_bounded_ = cv::Point2f(origin_.x*max_scale_inv_, origin_.y*max_scale_inv_);
      //check that the ROI is not out of bounds on cv_disp_img_, move the origin if necessary
      if(origin_bounded_.x + zoom_region_size_.width > cv_disp_img_.cols)
         origin_bounded_.x = cv_disp_img_.cols - zoom_region_size_.width;
      if(origin_bounded_.y + zoom_region_size_.height > cv_disp_img_.rows)
         origin_bounded_.y = cv_disp_img_.rows - zoom_region_size_.height;
      //chcek that we didn't move our origin too far
      if(origin_bounded_.x < 0)
         origin_bounded_.x = 0;
      if(origin_bounded_.y < 0)
         origin_bounded_.y = 0;
      //perform slight adjustment on the ROI size if necessary (ie. any error from float-int rounding)
      if(origin_bounded_.x + zoom_region_size_.width > cv_disp_img_.cols)
        zoom_region_size_.width = static_cast<float>(cv_disp_img_.cols) - origin_bounded_.x;
      if(origin_bounded_.y + zoom_region_size_.height > cv_disp_img_.rows)
        zoom_region_size_.height = static_cast<float>(cv_disp_img_.rows) - origin_bounded_.y;

      zoom_img_ = cv::Mat(cv_disp_img_, cv::Rect(origin_bounded_.x, origin_bounded_.y,
                                                 zoom_region_size_.width, zoom_region_size_.height));
      if(max_img_size == zoom_img_.size()) //don't call resize if we don't have to
        final_img_ = zoom_img_.clone();
      else
        cv::resize(zoom_img_, final_img_, max_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else if(is_scaled){
      if(max_img_size == cv_disp_img_.size()) //don't call resize if we don't have to
        final_img_ = cv_disp_img_.clone();
      else
        cv::resize(cv_disp_img_, final_img_, max_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else //original size
      final_img_ = cv_disp_img_.clone();

    //perform any backend conversions, normalizing, false colors
    if(normalize_img_)
      cv::normalize(final_img_, final_img_, 0, 255, cv::NORM_MINMAX);
    else if(normalize_roi_){
      roi_data_.mutex.lock();
      if(roi_data_.vertices.size() > 0 && !roi_data_.creating_flag){
        if(roi_data_.vertices.size() > 1 && roi_data_.type == Roi::ROI_RECT){
          cv::Point2f p1 = Image2View( cv::Point2f(roi_data_.vertices[0].x * resize_fx_total_,
                                                   roi_data_.vertices[0].y * resize_fy_total_) ),
                      p2 = Image2View( cv::Point2f(roi_data_.vertices[1].x * resize_fx_total_,
                                                   roi_data_.vertices[1].y * resize_fy_total_) );
          mio::SetBound<float>(p1.x, 0, final_img_.cols);
          mio::SetBound<float>(p1.y, 0, final_img_.rows);
          mio::SetBound<float>(p2.x, 0, final_img_.cols);
          mio::SetBound<float>(p2.y, 0, final_img_.rows);
          if(fabs(p1.x - p2.x) > 2 && fabs(p1.y - p2.y) > 2){
            cv::Mat roi = cv::Mat( final_img_, cv::Rect(p1, p2) );
            cv::normalize(roi, roi, 0, 255, cv::NORM_MINMAX);
          }
        }
      }
      roi_data_.mutex.unlock();
    }
    if(convert_to_false_colors_)
      cv::applyColorMap(final_img_, final_img_, cv::COLORMAP_JET);

    DrawRoi(final_img_); //draw the ROI if it exists and display the image

    OpenCVMat2QImage(final_img_, qt_disp_img_, true); //TODO - check for error here
    label_->setPixmap( QPixmap::fromImage(qt_disp_img_) );
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
    usleep(100000);
  }
  //m_mtx.unlock();
}


void AdvImageDisplay::Image2View(const std::vector<cv::Point2f> &src, std::vector<cv::Point> &dst, const bool scale_vertices){
  const size_t src_size = src.size();
  dst.resize( src.size() );
  if(src_size > 0){
    if(scale_vertices){
      for(size_t i = 0; i < src_size; ++i)
        dst[i] = Image2View( cv::Point2f(src[i].x * resize_fx_total_, src[i].y * resize_fy_total_) );
    }
    else
      for(size_t i = 0; i < src_size; ++i)
        dst[i] = Image2View(src[i]);
  }
}


//TODO - there could be a race between the image processing loop and the data used to draw the ROI's, that is,
//maybe the Roi should be emitted with each frame.
void AdvImageDisplay::DrawRoi(cv::Mat &img){
  cv::Scalar color = img.channels() > 1 ? cv::Scalar(50, 50, 50) : cv::Scalar(255, 255, 255);

  disp_roi_.mutex.lock();
  if(disp_roi_.vertices.size() > 0){
    std::vector<cv::Point> vertices;
    const bool scale_vertices = std::fabs(resize_fx_total_ - 1.0f) > 0.00001f &&
                                std::fabs(resize_fy_total_ - 1.0f) > 0.00001f ;
    Image2View(disp_roi_.vertices, vertices, scale_vertices);

    switch(disp_roi_.type){
      case Roi::ROI_RECT:
        STD_INVALID_ARG_E(vertices.size() == 2)
        std::cout << vertices[0] << ", " << vertices[1] << std::endl;
        cv::rectangle(img, vertices[0], vertices[1], color, 2);
        break;
      case Roi::ROI_POLY:
        if(vertices.size() > 1)
          cv::polylines(img, vertices, create_roi_, color, 2);
        break;
      case Roi::ROI_CENTER_CIRCLE:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::circle(img, vertices[0], sm::VerDist2(vertices[0], vertices[1]), color, 2);
        break;
      case Roi::ROI_DRAG_CIRCLE:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::circle(img, sm::MidPoint2(vertices[0], vertices[1]),
                   sm::VerDist2(vertices[0], vertices[1]) / 2.0f, color, 2);
        break;
    }
  }
  disp_roi_.mutex.unlock();
}


//create will remove the drawn ROI, but leaves the image processing ROI in the background
//to remove the background processing ROI, you must call Remove ROI
//void AdvImageDisplay::InitCreateRoi(){
  //create_roi_ = true;
  //roi_data_.mutex.lock();
    //roi_data_.vertices.clear();
//    roi_data_.type = ui->comboBox_ROIType->currentIndex();
    //ResetResizeTotal();
    //roi_data_.creating_flag = true;
  //roi_data_.mutex.unlock();

//  ui->comboBox_ROIType->setEnabled(false);
//  m_func_cv_add_button[VCV_RESIZE]->setEnabled(false); //no resizing allowed during Roi creation
//}


// void AdvImageDisplay::CreateRoi(){
//   roi_data_.mutex.lock();
//   create_roi_ = false;
//   roi_data_.creating_flag = false;

//  CImgProcQueue::roi_data_.vertices = roi_data_.vertices;
//  CImgProcQueue::roi_data_.type = roi_data_.type;
//  roi_data_.mutex.unlock();

//  CImgProcQueue::UpdateRoiMask();

//  ui->comboBox_ROIType->setEnabled(true);
//  m_func_cv_add_button[VCV_RESIZE]->setEnabled(true);
//  AdvImageDisplay::SendPptFrameLcm();
//}


//void AdvImageDisplay::ShowRoi(){
//  m_show_roi = ui->checkBox_showROI->isChecked();
//  AdvImageDisplay::SendPptFrameLcm();
//}


 void AdvImageDisplay::RemoveRoi(){
  //ShowRoi();
  disp_roi_.mutex.lock();
  create_roi_ = false;
  disp_roi_.vertices.clear();
  disp_roi_.mutex.unlock();
  UpdateDisplay();
}


void AdvImageDisplay::BeginCreateRoi(const int roi_type){
  printf("%s\n", CURRENT_FUNC);
  disp_roi_.mutex.lock();
  create_roi_ = true;
  disp_roi_.type = roi_type;
  disp_roi_.vertices.clear();
  ResetResizeTotal();
  disp_roi_.mutex.unlock();
}


void AdvImageDisplay::AddRoi(){
  // TODO callback to imagelistviewer so it knows when add roi is finished (enable add button)
  disp_roi_.mutex.lock();
  create_roi_ = false;
  disp_roi_.mutex.unlock();
}


void AdvImageDisplay::UpdateRoiMask(){
  printf("%s - id=%d\n", CURRENT_FUNC, id_);
  try{
//    m_out_img.lock();
//      cv::Size out_img_size = m_out_img.obj_.size();
//    m_out_img.unlock();

    roi_mask_mtx_.lock();
    {
      if(roi_data_.vertices.size() > 0){
//        roi_mask_ = cv::Mat::zeros(out_img_size, CV_8UC1);
#if ICV_OPENCV_VERSION_MAJOR < 3
        const int fill_flag = CV_FILLED;
#else
        const int fill_flag = cv::FILLED;
#endif
        switch(roi_data_.type){
          case Roi::ROI_RECT:
            STD_INVALID_ARG_E(roi_data_.vertices.size() == 2)
            cv::rectangle(roi_mask_, roi_data_.vertices[0], roi_data_.vertices[1], cv::Scalar(255), fill_flag);
            break;
          case Roi::ROI_POLY:
            {
            const size_t roi_vertices_size = roi_data_.vertices.size();
            std::vector< std::vector<cv::Point> > roi_vertices;
            roi_vertices.resize(1);
            roi_vertices.front().resize(roi_vertices_size);
            for(size_t i = 0; i < roi_vertices_size; ++i)
              roi_vertices[0][i] = roi_data_.vertices[i];
            cv::fillPoly( roi_mask_, roi_vertices, cv::Scalar(255) );
            }
            break;
          case Roi::ROI_CENTER_CIRCLE:
            {
            STD_INVALID_ARG_E(roi_data_.vertices.size() == 2)
            const float radius = sm::VerDist2(roi_data_.vertices[0], roi_data_.vertices[1]);
            cv::circle(roi_mask_, roi_data_.vertices[0], radius, cv::Scalar::all(255), fill_flag);
            }
            break;
          case Roi::ROI_DRAG_CIRCLE:
            {
            STD_INVALID_ARG_E(roi_data_.vertices.size() == 2)
            const cv::Point2f center = sm::MidPoint2(roi_data_.vertices[0], roi_data_.vertices[1]);
            const float radius = sm::VerDist2(roi_data_.vertices[0], roi_data_.vertices[1]) / 2.0f;
            cv::circle(roi_mask_, center, radius, cv::Scalar::all(255), fill_flag);
            }
            break;
        }
      }
      else
        roi_mask_ = cv::Mat();
      roi_mask_resize_ = roi_mask_;

      cv::Mat roi_mask_copy = roi_mask_.clone();
    }
    roi_mask_mtx_.unlock();
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
  }
}


void AdvImageDisplay::ResetResizeTotal(){
  resize_total_mutex_.lock();
  resize_fx_total_ = resize_fy_total_ = 1.0;
  resize_total_mutex_.unlock();
}


void AdvImageDisplay::SetBackEndNorm(bool state){
  normalize_img_ = state;
}

bool AdvImageDisplay::GetBackEndNorm(){
  return normalize_img_;
}

// void AdvImageDisplay::SetBackEndNormROI(bool state){
//   normalize_roi_ = state;
// }

// bool AdvImageDisplay::GetBackEndNormROI(){
//   return normalize_roi_;
// }

void AdvImageDisplay::SetBackEndFalseColors(bool state){
  convert_to_false_colors_ = state;
}

bool AdvImageDisplay::GetBackEndFalseColors(){
  return convert_to_false_colors_;
}

void AdvImageDisplay::SetLimitView(bool state){
  limit_view_ = state;
  if(state){
    zoom_info_tmp_.scroll_wheel_count = 0;
    zoom_info_tmp_.pixmap_mouse_pos = cv::Point2f(0, 0);
    UpdateZoom(zoom_info_tmp_);
  }
}

bool AdvImageDisplay::GetLimitView(){
  return limit_view_;
}

int AdvImageDisplay::GetID(){
  return id_;
}

void AdvImageDisplay::SetShowImage(bool state){
  show_image_ = state;
}

bool AdvImageDisplay::GetShowImage(){
  return show_image_;
}

// void AdvImageDisplay::SetBackEndConvert(bool state){
//   bec_.perform = state;
// }

// bool AdvImageDisplay::GetBackEndConvert(){
//   return bec_.perform;
// }

// void AdvImageDisplay::SetBackEndScale(float val){
//   bec_.scale = val;
// }

// float AdvImageDisplay::GetBackEndScale(){
//   return bec_.scale;
// }

QLabel* AdvImageDisplay::GetImageQLabel(){
  return label_;
}


inline bool BackEndConvert(const cv::Mat &src_img, cv::Mat &dst_img, const SBackEndConvert &bec){
  const int type = src_img.type();
  if(type == CV_8UC1 || type == CV_8UC3 || type == CV_32FC1 || type == CV_32FC3){
    dst_img = src_img.clone();
    return false;
  }
  else if(bec.perform){
    src_img.convertTo(dst_img, CV_8U, bec.scale);
    return false;
  }

  return true;
}


void AdvImageDisplay::ShowStripes(){
  STD_RT_ERR_E(mio::FileExists(ADV_IMG_DISP_STRIPES_JPEG))
  cv_disp_img_ = cv::imread(ADV_IMG_DISP_STRIPES_JPEG);
  UpdateDisplay();
}

