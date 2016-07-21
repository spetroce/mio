#include "advImageDisplay.h"

constexpr std::array<const char*, 4> Roi::roi_type_str;


AdvImageDisplay::AdvImageDisplay(QWidget *parent) : QWidget(parent), id_(0), normalize_img_(false),
    normalize_roi_(false), convert_to_false_colors_(false), layout_(NULL), label_(NULL), show_image_(false),
    is_init_(false), limit_view_(false), show_roi_(false), auto_convert_img_(false){
  prev_src_img_size_ = cv::Point(-1, -1);
  ResetZoom();
}


AdvImageDisplay::~AdvImageDisplay(){
  src_img_ = cv::Mat();
  UpdateDisplay();
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
  const int type = img.type();
  src_img_mtx_.lock();
  bool locked = true;
  if(type == CV_8UC1 || type == CV_8UC3 || type == CV_32FC1 || type == CV_32FC3)
    src_img_ = clone ? img.clone() : img;
  else if(auto_convert_img_)
    img.convertTo(src_img_, CV_8U);
  else{
    src_img_mtx_.unlock();
    locked = false;
    ShowStripes();
  }
  if(locked)
    src_img_mtx_.unlock();

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
            UpdateDisplay();
          }
          else{
            disp_roi_.mutex.unlock();
            RemoveRoi();
          }
        }
        break;
      case QEvent::Wheel:
        if(!show_roi_){
          QWheelEvent *wheel_event = mio::StaticCastPtr<QWheelEvent>(event);
          scroll_wheel_count_ += wheel_event->delta()/120;
          if(scroll_wheel_count_ < 0)
            scroll_wheel_count_ = 0;
          pixmap_mouse_pos_ = cv::Point2f(wheel_event->x(), wheel_event->y());
          UpdateZoom();
          UpdateDisplay();
          break;
        }
      default:
        break;
    }
  }

  return QWidget::eventFilter(target, event);
}


/*this functions updates the following vairables based on the scroll wheel count:
bool is_zoom_
float zoom_scaler_ - scale factor ranging from 0 to 1
cv::Point2f zoom_region_size_ - lower right ROI coordinate
cv::Point2f origin_ - upper left ROI coordinate
float prev_zoom_ - the previous zoom_scaler_
*/
void AdvImageDisplay::UpdateZoom(){
  src_img_mtx_.lock();
  const cv::Size kSrcImgSize = src_img_.size();
  src_img_mtx_.unlock();
  const float kZoomScale = 0.025f;
  if((is_zoom_ = scroll_wheel_count_ > 0)){
    //check for a minimum of 20 horizontal pixels
    while(kSrcImgSize.width * (1.0f - static_cast<float>(scroll_wheel_count_)*kZoomScale) < 20)
      if(scroll_wheel_count_-1 > 0)
        --scroll_wheel_count_;
      else{
        //could not find a zoom factor yielding the 20 pixel minimum; don't zoom
        ResetZoom();
        break;
      }
  }
  else
    ResetZoom();

  if(is_zoom_){
    zoom_scaler_ = 1.0f - static_cast<float>(scroll_wheel_count_)*kZoomScale;
    cv::Size2f frame_size(kSrcImgSize.width, kSrcImgSize.height);
    zoom_region_size_ = cv::Point2f(frame_size.width*zoom_scaler_, frame_size.height*zoom_scaler_);

    //scale the mouse position by the last zoom_scaler_
    cv::Point2f scaled_mouse = cv::Point2f(pixmap_mouse_pos_.x*prev_zoom_,
                                           pixmap_mouse_pos_.y*prev_zoom_);
    //find the new origin within the last scaled image and add it to the last origin_
    origin_ = cv::Point2f( origin_.x + ( scaled_mouse.x - scaled_mouse.x*(zoom_scaler_/prev_zoom_) ),
                           origin_.y + ( scaled_mouse.y - scaled_mouse.y*(zoom_scaler_/prev_zoom_) ) );

    prev_zoom_ = zoom_scaler_;
  }
}


void AdvImageDisplay::ResetZoom(){
  zoom_scaler_ = prev_zoom_ = max_disp_img_dim_scale_inv_ = 1.0;
  origin_ = origin_bounded_ = pixmap_mouse_pos_ = cv::Point2f(0, 0);
  scroll_wheel_count_ = 0;
  is_zoom_ = false;
}


// label_ to src_img_ coordinates
cv::Point2f AdvImageDisplay::View2Image(const cv::Point2f &view_pnt){
  return cv::Point2f((view_pnt.x*prev_zoom_ + origin_.x) * max_disp_img_dim_scale_inv_,
                     (view_pnt.y*prev_zoom_ + origin_.y) * max_disp_img_dim_scale_inv_ );
}


//src_img_ to label_ coordinates
cv::Point2f AdvImageDisplay::Image2View(const cv::Point2f &img_pnt){
  return cv::Point2f( (img_pnt.x - origin_bounded_.x) / zoom_scaler_ / max_disp_img_dim_scale_inv_,
                      (img_pnt.y - origin_bounded_.y) / zoom_scaler_ / max_disp_img_dim_scale_inv_ );
}


void AdvImageDisplay::UpdateDisplay(){
  src_img_mtx_.lock();
  const cv::Size kSrcImgSize = src_img_.size();
  resize_total_mtx_.lock();
  if(kSrcImgSize != prev_src_img_size_){
    resize_fx_total_ *= static_cast<float>(kSrcImgSize.width) / static_cast<float>(prev_src_img_size_.width);
    resize_fy_total_ *= static_cast<float>(kSrcImgSize.height) / static_cast<float>(prev_src_img_size_.height);
  }
  resize_total_mtx_.unlock();
  prev_src_img_size_ = kSrcImgSize;


  try{
    cv::Size max_disp_img_size = src_img_.size();
    float max_disp_img_dim_scale;
    const bool is_scaled = limit_view_ ? mio::GetMaxSize(src_img_, max_disp_img_dim_, max_disp_img_size,
                                                         max_disp_img_dim_scale) : false;
    max_disp_img_dim_scale_inv_ = is_scaled ? 1.0f/max_disp_img_dim_scale : 1.0f;

    if(is_zoom_){ //ROI with original size or maxSize resizing
      origin_bounded_ = cv::Point2f(origin_.x*max_disp_img_dim_scale_inv_, origin_.y*max_disp_img_dim_scale_inv_);
      //check that the ROI is not out of bounds on src_img_, move the origin if necessary
      if(origin_bounded_.x + zoom_region_size_.width > src_img_.cols)
         origin_bounded_.x = src_img_.cols - zoom_region_size_.width;
      if(origin_bounded_.y + zoom_region_size_.height > src_img_.rows)
         origin_bounded_.y = src_img_.rows - zoom_region_size_.height;
      //chcek that we didn't move our origin too far
      if(origin_bounded_.x < 0)
         origin_bounded_.x = 0;
      if(origin_bounded_.y < 0)
         origin_bounded_.y = 0;
      //perform slight adjustment on the ROI size if necessary (ie. any error from float-int rounding)
      if(origin_bounded_.x + zoom_region_size_.width > src_img_.cols)
        zoom_region_size_.width = static_cast<float>(src_img_.cols) - origin_bounded_.x;
      if(origin_bounded_.y + zoom_region_size_.height > src_img_.rows)
        zoom_region_size_.height = static_cast<float>(src_img_.rows) - origin_bounded_.y;

      zoom_img_ = cv::Mat(src_img_, cv::Rect(origin_bounded_.x, origin_bounded_.y,
                                             zoom_region_size_.width, zoom_region_size_.height));
      if(max_disp_img_size == zoom_img_.size()) //don't call resize if we don't have to
        disp_img_ = zoom_img_.clone();
      else
        cv::resize(zoom_img_, disp_img_, max_disp_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else if(is_scaled){
      if(max_disp_img_size == src_img_.size()) //don't call resize if we don't have to
        disp_img_ = src_img_.clone();
      else
        cv::resize(src_img_, disp_img_, max_disp_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else //original size
      disp_img_ = src_img_.clone();

    //perform any backend conversions, normalizing, false colors
    if(normalize_img_)
      cv::normalize(disp_img_, disp_img_, 0, 255, cv::NORM_MINMAX);
    else if(normalize_roi_){
      src_roi_.mutex.lock();
      if(src_roi_.vertices.size() > 0 && !create_roi_){
        if(src_roi_.vertices.size() > 1 && src_roi_.type == Roi::ROI_RECT){
          resize_total_mtx_.lock();
          cv::Point2f p1 = Image2View( cv::Point2f(src_roi_.vertices[0].x * resize_fx_total_,
                                                   src_roi_.vertices[0].y * resize_fy_total_) ),
                      p2 = Image2View( cv::Point2f(src_roi_.vertices[1].x * resize_fx_total_,
                                                   src_roi_.vertices[1].y * resize_fy_total_) );
          resize_total_mtx_.unlock();
          mio::SetBound<float>(p1.x, 0, disp_img_.cols);
          mio::SetBound<float>(p1.y, 0, disp_img_.rows);
          mio::SetBound<float>(p2.x, 0, disp_img_.cols);
          mio::SetBound<float>(p2.y, 0, disp_img_.rows);
          if(fabs(p1.x - p2.x) > 2 && fabs(p1.y - p2.y) > 2){
            cv::Mat roi = cv::Mat( disp_img_, cv::Rect(p1, p2) );
            cv::normalize(roi, roi, 0, 255, cv::NORM_MINMAX);
          }
        }
      }
      src_roi_.mutex.unlock();
    }
    if(convert_to_false_colors_)
      cv::applyColorMap(disp_img_, disp_img_, cv::COLORMAP_JET);

    if(show_roi_){
      roi_mask_mtx_.lock();
      if(is_scaled)
        cv::resize(roi_mask_, disp_roi_mask_, max_disp_img_size, 0, 0, cv::INTER_NEAREST);
      else
        disp_roi_mask_ = roi_mask_;
      roi_mask_mtx_.unlock();
      cv::Mat tmp_img(disp_img_.size(), disp_img_.type());
      disp_img_.copyTo(tmp_img, disp_roi_mask_);
      disp_img_ *= 0.4;
      tmp_img.copyTo(disp_img_, disp_roi_mask_);
    }
    DrawRoi(disp_img_); //draw the ROI if it exists and display the image

    OpenCVMat2QImage(disp_img_, qt_src_img_, true); //TODO - check for error here
    label_->setPixmap( QPixmap::fromImage(qt_src_img_) );
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
    usleep(100000);
  }
  src_img_mtx_.unlock();
}


void AdvImageDisplay::Image2View(const std::vector<cv::Point2f> &src, std::vector<cv::Point> &dst, const bool scale_vertices){
  const size_t src_size = src.size();
  dst.resize( src.size() );
  if(src_size > 0){
    if(scale_vertices){
      resize_total_mtx_.lock();
      for(size_t i = 0; i < src_size; ++i)
        dst[i] = Image2View( cv::Point2f(src[i].x * resize_fx_total_, src[i].y * resize_fy_total_) );
      resize_total_mtx_.unlock();
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
    resize_total_mtx_.lock();
    const bool scale_vertices = std::fabs(resize_fx_total_ - 1.0f) > 0.00001f &&
                                std::fabs(resize_fy_total_ - 1.0f) > 0.00001f;
    resize_total_mtx_.unlock();
    Image2View(disp_roi_.vertices, vertices, scale_vertices);

    switch(disp_roi_.type){
      case Roi::ROI_RECT:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::rectangle(img, vertices[0], vertices[1], color, 2);
        break;
      case Roi::ROI_POLY:
        if(vertices.size() > 1)
          cv::polylines(img, vertices, !create_roi_, color, 2);
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


void AdvImageDisplay::BeginCreateRoi(const int roi_type){
  printf("%s\n", CURRENT_FUNC);
  show_roi_ = false;
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
  UpdateRoiMask();
}


 void AdvImageDisplay::RemoveRoi(){
  disp_roi_.mutex.lock();
  create_roi_ = false;
  disp_roi_.vertices.clear();
  disp_roi_.mutex.unlock();
  UpdateRoiMask();
  UpdateDisplay();
}


void AdvImageDisplay::UpdateRoiMask(){
  printf("%s - id=%d\n", CURRENT_FUNC, id_);
  try{
    src_img_mtx_.lock();
    cv::Size disp_img_size = src_img_.size();
    src_img_mtx_.unlock();

    src_roi_.vertices = disp_roi_.vertices;
    src_roi_.type = disp_roi_.type;
    roi_mask_mtx_.lock();
    {
      if(src_roi_.vertices.size() > 0){
        roi_mask_ = cv::Mat::zeros(disp_img_size, CV_8UC1);
#if ICV_OPENCV_VERSION_MAJOR < 3
        const int fill_flag = CV_FILLED;
#else
        const int fill_flag = cv::FILLED;
#endif
        switch(src_roi_.type){
          case Roi::ROI_RECT:
            STD_INVALID_ARG_E(src_roi_.vertices.size() == 2)
            cv::rectangle(roi_mask_, src_roi_.vertices[0], src_roi_.vertices[1], cv::Scalar(255), fill_flag);
            break;
          case Roi::ROI_POLY:
            {
            const size_t roi_vertices_size = src_roi_.vertices.size();
            std::vector< std::vector<cv::Point> > roi_vertices;
            roi_vertices.resize(1);
            roi_vertices.front().resize(roi_vertices_size);
            for(size_t i = 0; i < roi_vertices_size; ++i)
              roi_vertices[0][i] = src_roi_.vertices[i];
            cv::fillPoly( roi_mask_, roi_vertices, cv::Scalar(255) );
            }
            break;
          case Roi::ROI_CENTER_CIRCLE:
            {
            STD_INVALID_ARG_E(src_roi_.vertices.size() == 2)
            const float radius = sm::VerDist2(src_roi_.vertices[0], src_roi_.vertices[1]);
            cv::circle(roi_mask_, src_roi_.vertices[0], radius, cv::Scalar::all(255), fill_flag);
            }
            break;
          case Roi::ROI_DRAG_CIRCLE:
            {
            STD_INVALID_ARG_E(src_roi_.vertices.size() == 2)
            const cv::Point2f center = sm::MidPoint2(src_roi_.vertices[0], src_roi_.vertices[1]);
            const float radius = sm::VerDist2(src_roi_.vertices[0], src_roi_.vertices[1]) / 2.0f;
            cv::circle(roi_mask_, center, radius, cv::Scalar::all(255), fill_flag);
            }
            break;
        }
      }
      else
        roi_mask_ = cv::Mat();
    }
    roi_mask_mtx_.unlock();
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
  }
}


void AdvImageDisplay::GetRoiMask(cv::Mat &roi, cv::Size resize_to){
  roi_mask_mtx_.lock();
  if(resize_to == cv::Size())
    roi = roi_mask_.clone();
  else
    cv::resize(roi_mask_, roi, resize_to, 0, 0, cv::INTER_NEAREST);
  roi_mask_mtx_.unlock();
}


void AdvImageDisplay::ResetResizeTotal(){
  resize_total_mtx_.lock();
  resize_fx_total_ = resize_fy_total_ = 1.0;
  resize_total_mtx_.unlock();
}


void AdvImageDisplay::ShowStripes(){
  STD_RT_ERR_E(mio::FileExists(ADV_IMG_DISP_STRIPES_JPEG))
  src_img_mtx_.lock();
  src_img_ = cv::imread(ADV_IMG_DISP_STRIPES_JPEG);
  src_img_mtx_.unlock();
  UpdateDisplay();
}


void AdvImageDisplay::ShowRoi(){
  EXP_CHK_EM(!create_roi_, return, "cannot modify 'show ROI' state while a ROI is being created")
  roi_mask_mtx_.lock();
  EXP_CHK_E(!roi_mask_.empty(), roi_mask_mtx_.unlock();return)
  roi_mask_mtx_.unlock();
  show_roi_ = !show_roi_;
  ResetZoom();
  UpdateDisplay();
}


void AdvImageDisplay::SetLimitView(const bool state, const int max_disp_img_dim){
  limit_view_ = state;
  max_disp_img_dim_ = max_disp_img_dim;
  if(state){
    ResetZoom();
    UpdateDisplay();
  }
}


bool AdvImageDisplay::GetLimitView(){
  return limit_view_;
}


int AdvImageDisplay::GetID(){
  return id_;
}


void AdvImageDisplay::SetShowImage(const bool state){
  show_image_ = state;
}


bool AdvImageDisplay::GetShowImage(){
  return show_image_;
}


void AdvImageDisplay::SetAutoConvertImage(const bool state){
  auto_convert_img_ = state;
}


bool AdvImageDisplay::GetAutoConvertImage(){
  return auto_convert_img_;
}


QLabel* AdvImageDisplay::GetImageQLabel(){
  return label_;
}


void AdvImageDisplay::SetNormalizeImage(const bool state){
  normalize_img_ = state;
}

bool AdvImageDisplay::GetNormalizeImage(){
  return normalize_img_;
}

void AdvImageDisplay::SetNormalizeRoi(const bool state){
  normalize_roi_ = state;
}

bool AdvImageDisplay::GetNormalizeRoi(){
  return normalize_roi_;
}

void AdvImageDisplay::SetConvertToFalseColors(const bool state){
  convert_to_false_colors_ = state;
}

bool AdvImageDisplay::GetConvertToFalseColors(){
  return convert_to_false_colors_;
}

