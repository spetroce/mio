#include "advImageDisplay.h"
#include <QMessageBox>
#include <QXmlStreamWriter>
#include "mio/qt/qtXml.h"

using namespace mio;

constexpr std::array<const char*, 4> Roi::roi_type_str;


AdvImageDisplay::AdvImageDisplay(QWidget *parent) : QWidget(parent), id_(0), normalize_img_(false),
    normalize_roi_(false), convert_to_false_colors_(false), layout_(NULL), label_(NULL), show_image_(false),
    is_init_(false), limit_view_(false), show_roi_(false), auto_convert_img_(false), zooming_enabled_(true),
    draw_mouse_clicks_(false), mouse_button_pressed_(false), roi_idx_(-1), create_roi_(false){
#ifdef HAVE_LCM
  lcm_is_init_ = false;
#endif
  prev_src_img_size_ = cv::Size2d(-1, -1);
  mouse_drag_ = cv::Point2d(0, 0);
  ResetZoom();
}


AdvImageDisplay::~AdvImageDisplay(){
#ifdef HAVE_LCM
  if(lcm_is_init_){
    disconnect(AdvImageDisplay::socket_notifier_, SIGNAL(activated(int)), this, SLOT(DataReady(int)));
    delete AdvImageDisplay::socket_notifier_;
    lcm_destroy(AdvImageDisplay::lcm_);
  }
#endif
  src_img_ = cv::Mat();
  UpdateDisplay();
  delete label_;
  label_ = NULL;
  if(layout_)
    delete layout_;
  layout_ = NULL;
}


void AdvImageDisplay::Init(const int id, const bool kManageLayout){
  EXP_CHK(!is_init_, return)
  id_ = id;

  qRegisterMetaType< cv::Mat >("cv::Mat");
  label_ = new QLabel();
  if(kManageLayout){
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
  connect(this, SIGNAL(ExternalDisplayUpdate()), this, SLOT(UpdateDisplay()), Qt::QueuedConnection);
  is_init_ = true;
}


void AdvImageDisplay::SetImage(const cv::Mat &kImg, const bool kClone, const bool kCalledFromExternalThread){
  const int kType = kImg.type();
  src_img_mtx_.lock();
  bool locked = true;
  if(kType == CV_8UC1 || kType == CV_8UC3 || kType == CV_32FC1 || kType == CV_32FC3)
    src_img_ = kClone ? kImg.clone() : kImg;
  else if(auto_convert_img_)
    kImg.convertTo(src_img_, CV_8U);
  else{
    src_img_mtx_.unlock();
    locked = false;
    ShowStripes();
  }
  if(locked)
    src_img_mtx_.unlock();

  if(kCalledFromExternalThread){
    emit ExternalDisplayUpdate();
    //QCoreApplication::processEvents();
  }
  else
    UpdateDisplay();
}


/*
All ROI vertices are saved in pixel coordinates in reference to the original unprocessed image.
The parameter mouse_pos is a pixel map coordinate and is repective to the final image displayed to the screen.
ViewToImage() removes effects caused by imgProcQueue resizing, view size limiting, and zooming.
*/
bool AdvImageDisplay::eventFilter(QObject *target, QEvent *event){
  if(target == label_){
    switch( event->type() ){
      case QEvent::MouseMove:
        {
          QMouseEvent *mouse_event = dynamic_cast<QMouseEvent*>(event);
          cv::Point2d mouse_pos(mouse_event->pos().x(), mouse_event->pos().y());
          cv::Point2d processed_img_mouse_pos = ViewToImage(mouse_pos);
          bool update_display = true;
          temp_roi_mtx_.lock();
          const size_t kRoiVerticesSize = temp_roi_.vertices.size();
          if(create_roi_ && kRoiVerticesSize > 1){
            if(temp_roi_.type == Roi::ROI_RECT ||
               temp_roi_.type == Roi::ROI_CENTER_CIRCLE || temp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              STD_INVALID_ARG_E(kRoiVerticesSize == 2)
              temp_roi_.vertices[1] = processed_img_mouse_pos;
            }
            else if(temp_roi_.type == Roi::ROI_POLY)
              temp_roi_.vertices.back() = processed_img_mouse_pos;
          }
          else if(mouse_button_pressed_){
            mouse_drag_ = mouse_button_press_init_pos_ - cv::Point2d(sm::RoundToMultiple(mouse_pos.x, 3),
                                                                     sm::RoundToMultiple(mouse_pos.y, 3));
            UpdateZoom();
            mouse_button_press_init_pos_ -= mouse_drag_;
            mouse_drag_ = cv::Point2d(0, 0);
          }
          else
            update_display = false;
          temp_roi_mtx_.unlock();
          if(update_display)
            UpdateDisplay();
          break;
        }
      case QEvent::MouseButtonPress:
        {
          mouse_button_pressed_ = true;
          mouse_drag_ = cv::Point2d(0, 0);
          bool update_display = false;
          QMouseEvent *mouse_event = dynamic_cast<QMouseEvent*>(event);
          const cv::Point2d mouse_pos = cv::Point2d( mouse_event->pos().x(), mouse_event->pos().y() );
          mouse_button_press_init_pos_ = mouse_pos;
          if(create_roi_){
            temp_roi_mtx_.lock();
            if(temp_roi_.type == Roi::ROI_RECT ||
               temp_roi_.type == Roi::ROI_CENTER_CIRCLE || temp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              temp_roi_.vertices.resize(2);
              temp_roi_.vertices[0] = temp_roi_.vertices[1] = ViewToImage(mouse_pos);
              update_display = true;
            }
            temp_roi_mtx_.unlock();
          }
          mouse_click_pnt_vec_.resize(1);
          mouse_click_pnt_vec_[0] = ViewToImage(mouse_pos);
          if(update_display || draw_mouse_clicks_)
            UpdateDisplay();
          break;
        }
      case QEvent::MouseButtonRelease:
        {
          mouse_button_pressed_ = false;
          bool update_display = false;
          if(create_roi_){
            temp_roi_mtx_.lock();
            if(temp_roi_.type == Roi::ROI_RECT ||
               temp_roi_.type == Roi::ROI_CENTER_CIRCLE || temp_roi_.type == Roi::ROI_DRAG_CIRCLE){
              temp_roi_mtx_.unlock();
              AddRoi();
              temp_roi_mtx_.lock();
            }
            else if(temp_roi_.type == Roi::ROI_POLY){
              QMouseEvent *mouse_event = dynamic_cast<QMouseEvent*>(event);
              const cv::Point2d mouse_pos = cv::Point2d( mouse_event->pos().x(), mouse_event->pos().y() );
              if( temp_roi_.vertices.empty() ){
                temp_roi_.vertices.push_back( ViewToImage(mouse_pos) );
                temp_roi_.vertices.push_back( ViewToImage(mouse_pos) );
              }
              else
                temp_roi_.vertices.push_back( ViewToImage(mouse_pos) );
              update_display = true;
            }
            temp_roi_mtx_.unlock();
          }
          if(update_display)
            UpdateDisplay();
          break;
        }
      case QEvent::KeyPress:
        if(create_roi_){
          QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
          const Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
          temp_roi_mtx_.lock();
          if(temp_roi_.type == Roi::ROI_POLY && temp_roi_.vertices.size() > 3 &&
             keyEvent->key() == Qt::Key_Space && modifiers == Qt::NoButton){
            temp_roi_.vertices.pop_back();
            temp_roi_mtx_.unlock();
            AddRoi();
            UpdateDisplay();
          }
          else{
            temp_roi_mtx_.unlock();
            CancelCreateRoi();
          }
        }
        break;
      case QEvent::Wheel:
        if(!show_roi_ && zooming_enabled_ && !mouse_button_pressed_){
          QWheelEvent *wheel_event = dynamic_cast<QWheelEvent*>(event);
          scroll_wheel_count_ += wheel_event->delta()/120;
          if(scroll_wheel_count_ < 0)
            scroll_wheel_count_ = 0;
          pixmap_mouse_pos_ = cv::Point2d(wheel_event->x(), wheel_event->y());
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


double GetZoomScalar(const int kIndex){
  const double kZoomScale = 0.025;
  return (1.0 - static_cast<double>(kIndex)*kZoomScale);
}

/*this functions updates the following vairables based on the scroll wheel count:
bool is_zoom_
double zoom_scalar_ - scale factor ranging from 0 to 1
cv::Point2d zoom_region_size_ - lower right ROI coordinate
cv::Point2d origin_ - upper left ROI coordinate
double prev_zoom_ - the previous zoom_scalar_
*/
void AdvImageDisplay::UpdateZoom(){
  src_img_mtx_.lock();
  const cv::Size2d kSrcImgSize = src_img_.size();
  src_img_mtx_.unlock();
  is_zoom_ = scroll_wheel_count_ > 0;
  if(is_zoom_){
    //check for a minimum of 20 horizontal pixels
    while(kSrcImgSize.width*GetZoomScalar(scroll_wheel_count_) < 20)
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
    zoom_scalar_ = GetZoomScalar(scroll_wheel_count_);
    //find the new origin within the last scaled image and add it to the last origin_
    origin_ = cv::Point2d( origin_.x + (pixmap_mouse_pos_.x*prev_zoom_ - pixmap_mouse_pos_.x*zoom_scalar_),
                           origin_.y + (pixmap_mouse_pos_.y*prev_zoom_ - pixmap_mouse_pos_.y*zoom_scalar_) );
    origin_ += mouse_drag_*zoom_scalar_;
    zoom_region_size_ = cv::Size2d(kSrcImgSize.width*zoom_scalar_, kSrcImgSize.height*zoom_scalar_);
    mio::SetClamp<double>(origin_.x, 0, kSrcImgSize.width-zoom_region_size_.width);
    mio::SetClamp<double>(origin_.y, 0, kSrcImgSize.height-zoom_region_size_.height);
    prev_zoom_ = zoom_scalar_;
  }
}


void AdvImageDisplay::ResetZoom(){
  zoom_scalar_ = prev_zoom_ = display_img_dim_scalar_inv_ = 1.0;
  origin_ = clamped_origin_ = pixmap_mouse_pos_ = cv::Point2d(0, 0);
  scroll_wheel_count_ = 0;
  is_zoom_ = false;
}


// label_ to src_img_ coordinates
cv::Point2d AdvImageDisplay::ViewToImage(const cv::Point2d &kViewPnt){
  return cv::Point2d((kViewPnt.x*prev_zoom_ + origin_.x) * display_img_dim_scalar_inv_,
                     (kViewPnt.y*prev_zoom_ + origin_.y) * display_img_dim_scalar_inv_ );
}


// src_img_ to label_ coordinates
cv::Point2d AdvImageDisplay::ImageToView(const cv::Point2d &kImgPnt){
  return cv::Point2d((kImgPnt.x - clamped_origin_.x) / prev_zoom_ / display_img_dim_scalar_inv_,
                     (kImgPnt.y - clamped_origin_.y) / prev_zoom_ / display_img_dim_scalar_inv_);
}


void AdvImageDisplay::UpdateDisplay(){
  src_img_mtx_.lock();
  const cv::Size2d kSrcImgSize = src_img_.size();
  resize_total_mtx_.lock();
  if(kSrcImgSize != prev_src_img_size_){
    resize_fx_total_ *= kSrcImgSize.width/prev_src_img_size_.width;
    resize_fy_total_ *= kSrcImgSize.height/prev_src_img_size_.height;
  }
  resize_total_mtx_.unlock();
  prev_src_img_size_ = kSrcImgSize;

  try{
    cv::Size display_img_size = src_img_.size();
    double display_img_dim_scale;
    const bool kIsScaled = limit_view_ ? mio::GetMaxSize(src_img_, limit_view_max_img_dim_, display_img_size,
                                                         display_img_dim_scale) : false;
    display_img_dim_scalar_inv_ = kIsScaled ? 1.0/display_img_dim_scale : 1.0;

    if(is_zoom_){ //ROI with original size or maxSize resizing
      clamped_origin_ = origin_*display_img_dim_scalar_inv_;
      mio::SetClamp(clamped_origin_.x, 0.0, src_img_.cols - zoom_region_size_.width);
      mio::SetClamp(clamped_origin_.y, 0.0, src_img_.rows - zoom_region_size_.height);
      zoom_img_ = cv::Mat(src_img_, cv::Rect(clamped_origin_.x, clamped_origin_.y,
                                             zoom_region_size_.width, zoom_region_size_.height));
      if(display_img_size == zoom_img_.size()) //don't call resize if we don't have to
        disp_img_ = zoom_img_.clone();
      else
        cv::resize(zoom_img_, disp_img_, display_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else if(kIsScaled){
      if(display_img_size == src_img_.size()) //don't call resize if we don't have to
        disp_img_ = src_img_.clone();
      else
        cv::resize(src_img_, disp_img_, display_img_size, 0, 0, cv::INTER_NEAREST);
    }
    else //original size
      disp_img_ = src_img_.clone();

    //perform any backend conversions, normalizing, false colors
    if(normalize_img_){
      cv::normalize(disp_img_, disp_img_, 0, 255, cv::NORM_MINMAX);
    }
    else if(normalize_roi_ && roi_vec_.size() > 0){
      Roi &roi = roi_vec_[roi_idx_ == -1 ? 0 : roi_idx_];
      roi_vec_mtx_.lock();
      if(roi.vertices.size() > 1 && roi.type == Roi::ROI_RECT && !create_roi_){
        resize_total_mtx_.lock();
        cv::Point2d p1 = ImageToView( cv::Point2d(roi.vertices[0].x * resize_fx_total_,
                                                  roi.vertices[0].y * resize_fy_total_) ),
                    p2 = ImageToView( cv::Point2d(roi.vertices[1].x * resize_fx_total_,
                                                  roi.vertices[1].y * resize_fy_total_) );
        resize_total_mtx_.unlock();
        mio::SetClamp<double>(p1.x, 0, disp_img_.cols-1);
        mio::SetClamp<double>(p1.y, 0, disp_img_.rows-1);
        mio::SetClamp<double>(p2.x, 0, disp_img_.cols-1);
        mio::SetClamp<double>(p2.y, 0, disp_img_.rows-1);
        if(std::fabs(p1.x - p2.x) > 2 && std::fabs(p1.y - p2.y) > 2){
          cv::Mat roi = cv::Mat( disp_img_, cv::Rect(p1, p2) );
          cv::normalize(roi, roi, 0, 255, cv::NORM_MINMAX);
        }
      }
      roi_vec_mtx_.unlock();
    }
    if(convert_to_false_colors_)
      cv::applyColorMap(disp_img_, disp_img_, cv::COLORMAP_JET);

    if(show_roi_ && ((roi_idx_ == -1 && roi_vec_.size() == 1) || roi_idx_ > -1)){
      roi_mask_mtx_.lock();
      if(kIsScaled)
        cv::resize(roi_mask_, temp_roi_mask_, display_img_size, 0, 0, cv::INTER_NEAREST);
      else
        temp_roi_mask_ = roi_mask_;
      roi_mask_mtx_.unlock();
      cv::Mat tmp_img(disp_img_.size(), disp_img_.type());
      disp_img_.copyTo(tmp_img, temp_roi_mask_);
      disp_img_ *= 0.4;
      tmp_img.copyTo(disp_img_, temp_roi_mask_);
    }

    DrawRoi(disp_img_); //draw the ROI if it exists

    if(draw_mouse_clicks_){
      for(const auto &pnt : mouse_click_pnt_vec_)
#if CV_MAJOR_VERSION < 3
        cv::circle(disp_img_, ImageToView(pnt), 5, cv::Scalar::all(255), 1);
#else
        cv::drawMarker(disp_img_, ImageToView(pnt), cv::Scalar::all(255), cv::MARKER_CROSS, 10);
#endif
    }

    OpenCVMat2QImage(disp_img_, qt_src_img_, true); //TODO - check for error here
    label_->setPixmap( QPixmap::fromImage(qt_src_img_) );
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
    usleep(100000);
  }
  src_img_mtx_.unlock();
}


void AdvImageDisplay::ImageToView(const std::vector<cv::Point2d> &kSrcPnt, std::vector<cv::Point> &dst, const bool kScaleVertices){
  const size_t kSrcSize = kSrcPnt.size();
  dst.resize(kSrcPnt.size());
  if(kSrcSize > 0){
    if(kScaleVertices){
      resize_total_mtx_.lock();
      for(size_t i = 0; i < kSrcSize; ++i)
        dst[i] = ImageToView( cv::Point2d(kSrcPnt[i].x * resize_fx_total_, kSrcPnt[i].y * resize_fy_total_) );
      resize_total_mtx_.unlock();
    }
    else
      for(size_t i = 0; i < kSrcSize; ++i)
        dst[i] = ImageToView(kSrcPnt[i]);
  }
}


void AdvImageDisplay::DrawRoi(const Roi &kRoi, cv::Mat &img){
  cv::Scalar color = img.channels() > 1 ? cv::Scalar(50, 50, 50) : cv::Scalar(255, 255, 255);
  if(kRoi.vertices.size() > 0){
    std::vector<cv::Point> vertices;
    resize_total_mtx_.lock();
    const bool kScaleVertices = std::fabs(resize_fx_total_ - 1.0) > 0.00001 &&
                                std::fabs(resize_fy_total_ - 1.0) > 0.00001;
    resize_total_mtx_.unlock();
    ImageToView(kRoi.vertices, vertices, kScaleVertices);

    switch(kRoi.type){
      case Roi::ROI_RECT:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::rectangle(img, vertices[0], vertices[1], color, 1);
        break;
      case Roi::ROI_POLY:
        if(vertices.size() > 1)
          cv::polylines(img, vertices, !create_roi_, color, 1);
        break;
      case Roi::ROI_CENTER_CIRCLE:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::circle(img, vertices[0], sm::VerDist2(vertices[0], vertices[1]), color, 1);
        break;
      case Roi::ROI_DRAG_CIRCLE:
        STD_INVALID_ARG_E(vertices.size() == 2)
        cv::circle(img, sm::MidPoint2(vertices[0], vertices[1]),
                   sm::VerDist2(vertices[0], vertices[1]) / 2.0f, color, 1);
        break;
    }
  }
}


void AdvImageDisplay::DrawRoi(cv::Mat &img){
  if(create_roi_){
    temp_roi_mtx_.lock();
    DrawRoi(temp_roi_, img);
    temp_roi_mtx_.unlock();
  }

  roi_vec_mtx_.lock();
  if(roi_vec_.size() > 0)
    if(roi_idx_ == -1)
      for(const auto roi : roi_vec_)
        DrawRoi(roi, img);
    else if(roi_idx_ < static_cast<int>(roi_vec_.size()))
      DrawRoi(roi_vec_[roi_idx_], img);
  roi_vec_mtx_.unlock();
}


void AdvImageDisplay::CancelCreateRoi(){
  if(create_roi_){
    temp_roi_mtx_.lock();
    temp_roi_.vertices.clear();
    create_roi_ = false;
    temp_roi_mtx_.unlock();
  }
}


void AdvImageDisplay::CreateRoi(const int kRoiType){
  printf("%s\n", CURRENT_FUNC);
  temp_roi_mtx_.lock();
  temp_roi_.type = kRoiType;
  temp_roi_.vertices.clear();
  show_roi_ = false;
  create_roi_ = true;
  temp_roi_mtx_.unlock();
  ResetResizeTotal();
}


void AdvImageDisplay::AddRoi(){
  // TODO callback to imagelistviewer so it knows when add roi is finished (enable add button)
  temp_roi_mtx_.lock();
  create_roi_ = false;
  roi_vec_mtx_.lock();
  roi_vec_.push_back(temp_roi_);
  roi_vec_mtx_.unlock();
  temp_roi_mtx_.unlock();
  UpdateRoiMask();
  UpdateDisplay();
}


void AdvImageDisplay::RemoveRoi(const int kRoiIdx){
  if(create_roi_){
    temp_roi_mtx_.lock();
    create_roi_ = false;
    temp_roi_.vertices.clear();
    temp_roi_mtx_.unlock();
  }
  else{
    roi_vec_mtx_.lock();
    if(kRoiIdx == -1)
      roi_vec_.clear();
    else if(roi_vec_.size() > 0 && kRoiIdx > 0 && kRoiIdx < roi_vec_.size())
      roi_vec_.erase(roi_vec_.begin()+kRoiIdx);
    roi_vec_mtx_.unlock();
  }
  UpdateRoiMask();
  UpdateDisplay();
}


void AdvImageDisplay::UpdateRoiMask(){
  try{
    src_img_mtx_.lock();
    cv::Size disp_img_size = src_img_.size();
    src_img_mtx_.unlock();

    roi_mask_mtx_.lock();
    roi_vec_mtx_.lock();
    {
      if(roi_vec_.size() > 0 && roi_idx_ < static_cast<int>(roi_vec_.size())){
        Roi &roi = roi_vec_[(roi_idx_ == -1 ? 0 : roi_idx_)];
        roi_mask_ = cv::Mat::zeros(disp_img_size, CV_8UC1);
#if CV_MAJOR_VERSION < 3
        const int kFillFlag = CV_FILLED;
#else
        const int kFillFlag = cv::FILLED;
#endif
        switch(roi.type){
          case Roi::ROI_RECT:
            STD_INVALID_ARG_E(roi.vertices.size() == 2)
            cv::rectangle(roi_mask_, roi.vertices[0], roi.vertices[1], cv::Scalar(255), kFillFlag);
            break;
          case Roi::ROI_POLY:
            {
            const size_t kRoiVerticesSize = roi.vertices.size();
            std::vector< std::vector<cv::Point> > roi_vertices;
            roi_vertices.resize(1);
            roi_vertices.front().resize(kRoiVerticesSize);
            for(size_t i = 0; i < kRoiVerticesSize; ++i)
              roi_vertices[0][i] = roi.vertices[i];
            cv::fillPoly( roi_mask_, roi_vertices, cv::Scalar(255) );
            }
            break;
          case Roi::ROI_CENTER_CIRCLE:
            {
            STD_INVALID_ARG_E(roi.vertices.size() == 2)
            const double kRadius = sm::VerDist2(roi.vertices[0], roi.vertices[1]);
            cv::circle(roi_mask_, roi.vertices[0], kRadius, cv::Scalar::all(255), kFillFlag);
            }
            break;
          case Roi::ROI_DRAG_CIRCLE:
            {
            STD_INVALID_ARG_E(roi.vertices.size() == 2)
            const cv::Point2d center = sm::MidPoint2(roi.vertices[0], roi.vertices[1]);
            const double kRadius = sm::VerDist2(roi.vertices[0], roi.vertices[1]) / 2.0f;
            cv::circle(roi_mask_, center, kRadius, cv::Scalar::all(255), kFillFlag);
            }
            break;
        }
      }
      else
        roi_mask_ = cv::Mat();
    }
    roi_vec_mtx_.unlock();
    roi_mask_mtx_.unlock();
  }
  catch(cv::Exception &e){
    printf("%s - caught error: %s, id=%d\n", CURRENT_FUNC, e.what(), id_);
  }
}


void AdvImageDisplay::GetRoiMask(cv::Mat &roi, cv::Size resize_to){
  roi_mask_mtx_.lock();
  EXP_CHK(!roi_mask_.empty(), roi_mask_mtx_.unlock();return)
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
  src_img_mtx_.lock();
  src_img_ = cv::imread(ADV_IMG_DISP_STRIPES_JPEG);
  src_img_mtx_.unlock();
  UpdateDisplay();
}


void AdvImageDisplay::ShowRoi(){
  EXP_CHK_M(!create_roi_, return, "cannot modify 'show ROI' state while a ROI is being created")
  roi_mask_mtx_.lock();
  EXP_CHK(!roi_mask_.empty(), roi_mask_mtx_.unlock();return)
  roi_mask_mtx_.unlock();
  show_roi_ = !show_roi_;
  ResetZoom();
  UpdateDisplay();
}


void AdvImageDisplay::SetLimitView(const bool kState, const int kMaxDispImgDim){
  limit_view_ = kState;
  limit_view_max_img_dim_ = kMaxDispImgDim;
  if(kState){
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


void AdvImageDisplay::SetShowImage(const bool kState){
  show_image_ = kState;
}


bool AdvImageDisplay::GetShowImage(){
  return show_image_;
}


void AdvImageDisplay::SetAutoConvertImage(const bool kState){
  auto_convert_img_ = kState;
}


bool AdvImageDisplay::GetAutoConvertImage(){
  return auto_convert_img_;
}


QLabel* AdvImageDisplay::GetImageQLabel(){
  return label_;
}


void AdvImageDisplay::SetNormalizeImage(const bool kState){
  normalize_img_ = kState;
  UpdateDisplay();
}

bool AdvImageDisplay::GetNormalizeImage(){
  return normalize_img_;
}

void AdvImageDisplay::SetNormalizeRoi(const bool kState){
  normalize_roi_ = kState;
  UpdateDisplay();
}

bool AdvImageDisplay::GetNormalizeRoi(){
  return normalize_roi_;
}

void AdvImageDisplay::SetConvertToFalseColors(const bool kState){
  convert_to_false_colors_ = kState;
}

bool AdvImageDisplay::GetConvertToFalseColors(){
  return convert_to_false_colors_;
}

bool AdvImageDisplay::SetZoomingEnabled(const bool kEnabled){
  zooming_enabled_ = kEnabled;
  ResetZoom();
}

void AdvImageDisplay::SetDrawClicks(const bool kSet){
  draw_mouse_clicks_ = kSet;
  UpdateDisplay();
}

bool AdvImageDisplay::GetDrawClicks(){
  return draw_mouse_clicks_;
}

void AdvImageDisplay::GetClickPnts(std::vector<cv::Point2d> &pnt_vec){
  pnt_vec = mouse_click_pnt_vec_;
}

void AdvImageDisplay::ClearClickPntBuffer(){
  mouse_click_pnt_vec_.clear();
  UpdateDisplay();
}

// Set the index of the ROI you want to display. Setting to -1 will display all ROIs
void AdvImageDisplay::SetRoiIndex(const int kRoiIdx){
  roi_idx_ = kRoiIdx;
  UpdateRoiMask();
  UpdateDisplay();
}

int AdvImageDisplay::GetRoiIndex(){
  return roi_idx_;
}


void AdvImageDisplay::SaveRoi(QString file_full_qstr){
  if(roi_vec_.size() > 0){
    EXP_CHK(!file_full_qstr.isEmpty(), return)
    std::string file_full = file_full_qstr.toStdString(), file_path;
    mio::FileNameExpand(file_full, ".", &file_path, NULL, NULL, NULL);
    EXP_CHK_M(mio::DirExists(file_path), return, file_path + "is not an existing directory")
    mio::ForceFileExtension(file_full, "xml");
    printf("%s - saving ROI to %s\n", CURRENT_FUNC, file_full.c_str());
    file_full_qstr = QString::fromStdString(file_full);
    QFile file(file_full_qstr);
    EXP_CHK(file.open(QIODevice::WriteOnly),
            QMessageBox::warning(0, "Read only", "The file is in read only mode");return)

    QXmlStreamWriter xml_writer(&file);
    xml_writer.setAutoFormatting(true);
    xml_writer.writeStartDocument(); // write XML version number

    xml_writer.writeStartElement("roi_list");
    for(const auto &kRoi : roi_vec_){
      if(kRoi.vertices.size() >= 2){
        xml_writer.writeStartElement("roi");
        xml_writer.writeAttribute("type", INT_TO_QSTR(kRoi.type));
        xml_writer.writeStartElement("point_list");
        for(const auto &pnt : kRoi.vertices){
          xml_writer.writeStartElement("point");
          xml_writer.writeAttribute("x", FLT_TO_QSTR(pnt.x));
          xml_writer.writeAttribute("y", FLT_TO_QSTR(pnt.y));
          xml_writer.writeEndElement(); // end point
        }
        xml_writer.writeEndElement(); // end point_list
        xml_writer.writeEndElement(); // end roi
      }
    }
    xml_writer.writeEndElement(); // end roi_list

    xml_writer.writeEndDocument();
    file.close();
  }
}


void AdvImageDisplay::LoadRoi(const QString kFileFullQStr, std::vector<int> &loaded_roi_types){
  EXP_CHK(!kFileFullQStr.isEmpty(), return)
  std::string file_full = kFileFullQStr.toStdString();
  EXP_CHK(mio::FileExists(file_full), return)
  printf("%s - loading ROI from %s\n", CURRENT_FUNC, file_full.c_str());
  loaded_roi_types.clear();

  QFile file(kFileFullQStr);
  EXP_CHK(file.open(QIODevice::ReadOnly | QIODevice::Text),
          QMessageBox::warning(0, "AdvImageDisplay::LoadRoi", "Couldn't open xml file");return)

  QXmlStreamAttributes attr;
  QXmlStreamReader xml_reader(&file);
  Roi roi;
  while(!xml_reader.atEnd() && !xml_reader.hasError()){
    xml_reader.readNext();
    if(xml_reader.isStartDocument()) // skip StartDocument tokens
      continue;

    if(xml_reader.isStartElement() && xml_reader.name() == "roi_list"){
      // iterate through children of roi_list
      while(!xml_reader.atEnd() && !xml_reader.hasError()){
        xml_reader.readNext();
        if(xml_reader.isStartElement() && xml_reader.name() == "roi"){
          roi.vertices.clear();
          attr = xml_reader.attributes();
          if(attr.hasAttribute("type") )
            roi.type = ATTR_TO_INT(attr, "type");

          // iterate through children of roi
          while(!xml_reader.atEnd() && !xml_reader.hasError()){
            xml_reader.readNext();
            if(xml_reader.isStartElement() && xml_reader.name() == "point_list"){
              // iterate through children of point_list
              while(!xml_reader.atEnd() && !xml_reader.hasError()){
                xml_reader.readNext();
                if(xml_reader.isStartElement() && xml_reader.name() == "point"){
                  attr = xml_reader.attributes();
                  cv::Point2d pnt;
                  if(attr.hasAttribute("x"))
                    pnt.x = ATTR_TO_FLT(attr, "x");
                  if(attr.hasAttribute("y"))
                    pnt.y = ATTR_TO_FLT(attr, "y");
                  roi.vertices.push_back(pnt);
                  std::cout << pnt << std::endl;
                } // "point"
                else if(xml_reader.isEndElement() && xml_reader.name() == "point_list")
                  break;
              }
            } // "point_list"
            else if(xml_reader.isEndElement() && xml_reader.name() == "roi")
              break;
          }
          if(roi.vertices.size() > 1){
            roi_vec_.push_back(roi);
            loaded_roi_types.push_back(roi.type);
          }
        } // "roi"
        else if(xml_reader.isEndElement() && xml_reader.name() == "roi_list")
          break;
      }
    } // "roi_list"
  }

  if(xml_reader.hasError())
    QMessageBox::critical(this, "QXSRExample::parseXML", xml_reader.errorString(), QMessageBox::Ok);
  xml_reader.clear(); // removes any device() or data from the reader and resets its internal state to the initial state

  file.close();
  UpdateRoiMask();
  UpdateDisplay();
}


void AdvImageDisplay::SetupLcm(const std::string kNewFrameLcmChanNamePrefix){
#ifdef HAVE_LCM
  if(AdvImageDisplay::lcm_ == NULL)
    AdvImageDisplay::lcm_ = lcm_create(NULL);
  EXP_CHK_M(AdvImageDisplay::lcm_ != NULL, return, std::string("id=") + std::to_string(id_));
  AdvImageDisplay::lcm_fd_ = lcm_get_fileno(AdvImageDisplay::lcm_);
  AdvImageDisplay::socket_notifier_ = new QSocketNotifier(AdvImageDisplay::lcm_fd_, QSocketNotifier::Read, this);
  connect(AdvImageDisplay::socket_notifier_, SIGNAL(activated(int)), this, SLOT(DataReady(int)));

  std::string new_frame_lcm_chan_name = kNewFrameLcmChanNamePrefix + "_" + std::to_string(id_);
  new_frame_lcm_sub_ = lcm_opencv_mat_t_subscribe(AdvImageDisplay::lcm_, new_frame_lcm_chan_name.c_str(),
                                                  &NewFrameLCM, static_cast<void*>(this));
  lcm_opencv_mat_t_subscription_set_queue_capacity(new_frame_lcm_sub_, 2);
  lcm_is_init_ = true;
#endif
}


#ifdef HAVE_LCM
void AdvImageDisplay::NewFrameLCM(const lcm_recv_buf_t *rbuf, const char *channel, 
                                  const lcm_opencv_mat_t *msg, void *userdata){
  AdvImageDisplay *w = static_cast<AdvImageDisplay*>(userdata);
  const cv::Mat kImg(msg->rows, msg->cols, msg->openCvType, msg->data);
  w->SetImage(kImg, true);
}
#endif

