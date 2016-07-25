#include "imageListViewer.h"
#include "ui_imageListViewer.h"


ImageListViewer::ImageListViewer(const bool show_earth, QWidget *parent) : QWidget(parent), ui(new Ui::ImageListViewer){
  ui->setupUi(this);
  adv_img_disp_ = new AdvImageDisplay();
  adv_img_disp_->Init(0, false);
  ui->grid_lay_disp_img->addWidget(adv_img_disp_->GetImageQLabel());

  ui->slider_img_idx->setMinimum(0);
  ui->slider_img_idx->setMaximum(0);

  for(size_t i = 0; i < Roi::roi_type_str.size(); ++i)
    ui->comboBox_roi_type->addItem(Roi::roi_type_str[i]);

  connect(ui->slider_img_idx, SIGNAL(valueChanged(int)), this, SLOT(SetImage(int)));
  connect(ui->pb_prev_img, SIGNAL(clicked()), this, SLOT(DecrementImgIdxSlider()));
  connect(ui->pb_next_img, SIGNAL(clicked()), this, SLOT(IncrementImgIdxSlider()));
  connect(ui->pb_roi_add, SIGNAL(clicked()), this, SLOT(AddRoi()));
  connect(ui->pb_roi_rem, SIGNAL(clicked()), this, SLOT(RemoveRoi()));
  connect(ui->pb_show_roi, SIGNAL(clicked()), this, SLOT(ShowRoi()));

  adv_img_disp_->SetLimitView(true);
  if(show_earth)
    ShowEarth();

  //TODO - add support for multiple roi's
  ui->horizontalLayout_roi->removeWidget(ui->comboBox_select_roi);
  delete ui->comboBox_select_roi;
}


ImageListViewer::~ImageListViewer(){
  delete ui;
}


void ImageListViewer::SetImageList(std::string file_path, const std::vector<std::string> &img_file_name_vec){
  EXP_CHK_E(img_file_name_vec.size() > 0, adv_img_disp_->ShowStripes();return)
  const size_t img_file_name_vec_size = img_file_name_vec.size();
  img_vec_.resize(img_file_name_vec_size);
  img_file_name_vec_.resize(img_file_name_vec_size);
  size_t num_valid_img = 0;
  mio::FormatFilePath(file_path);
  std::string file_full;
  for(const std::string &file_name : img_file_name_vec){
    if(file_name.empty())
      continue;
    file_full = file_path + "/" + file_name;
    EXP_CHK_EM(mio::FileExists(file_full), continue, "file_full=" + file_full)
    try{
      img_vec_[num_valid_img] = cv::imread(file_full);
      img_file_name_vec_[num_valid_img] = file_name;
      ++num_valid_img;
    }
    catch(...){
      std::cout << CURRENT_FUNC << " - could not open file: " << file_full << std::endl;
      continue;
    }
  }
  img_vec_.resize(num_valid_img);
  img_file_name_vec_.resize(num_valid_img);

  ui->slider_img_idx->setMaximum(num_valid_img <= 1 ? 1 : num_valid_img-1);
  ui->slider_img_idx->setEnabled(num_valid_img > 1);
  ui->pb_prev_img->setEnabled(num_valid_img > 1);
  ui->pb_next_img->setEnabled(num_valid_img > 1);

  if(num_valid_img == 0)
    adv_img_disp_->ShowStripes();
  else
    SetImage(0);
}


void ImageListViewer::SetImageList(const std::vector<std::string> &img_file_name_vec,
                                   const std::vector<cv::Mat> &img_vec, const bool clone_images){
  EXP_CHK_E(img_file_name_vec.size() > 0, return)
  EXP_CHK_E(img_file_name_vec.size() == img_vec.size(), return)

  img_file_name_vec_ = img_file_name_vec;
  if(clone_images){
    const size_t img_vec_size = img_vec.size();
    img_vec_.resize(img_vec_size);
    for(size_t i = 0; i < img_vec_size; ++i)
      img_vec_[i] = img_vec[i].clone();
  }
  else
    img_vec_ = img_vec;

  ui->slider_img_idx->setMaximum(img_vec_.size()-1);
}


void ImageListViewer::SetImage(const int idx){
  EXP_CHK_E(idx >= 0 && idx < img_vec_.size(), return)
  adv_img_disp_->SetImage(img_vec_[idx], true);
  ui->lineEdit_img_name->setText(QString(img_file_name_vec_[idx].c_str()));
}


void ImageListViewer::ShowEarth(){
  STD_RT_ERR_E(mio::FileExists(IMG_LIST_VIEWER_EARTH_JPEG_DIR"/earth.jpg"))
  std::vector<std::string> img_file_name_vec = {"earth.jpg"};
  SetImageList(IMG_LIST_VIEWER_EARTH_JPEG_DIR, img_file_name_vec);
}


void ImageListViewer::DecrementImgIdxSlider(){
  ui->slider_img_idx->setValue(ui->slider_img_idx->value() - 1);
}


void ImageListViewer::IncrementImgIdxSlider(){
  ui->slider_img_idx->setValue(ui->slider_img_idx->value() + 1);
}


void ImageListViewer::AddRoi(){
  adv_img_disp_->BeginCreateRoi(ui->comboBox_roi_type->currentIndex());
}


void ImageListViewer::RemoveRoi(){
  adv_img_disp_->RemoveRoi();
}


void ImageListViewer::ShowRoi(){
  adv_img_disp_->ShowRoi();
}

