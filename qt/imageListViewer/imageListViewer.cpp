#include "imageListViewer.h"
#include "ui_imageListViewer.h"


ImageListViewer::ImageListViewer(QWidget *parent) : QWidget(parent), ui(new Ui::ImageListViewer){
  ui->setupUi(this);
  adv_img_disp_ = new AdvImageDisplay();
  adv_img_disp_->Init(0, false);
  ui->grid_lay_disp_img->addWidget(adv_img_disp_->GetImageQLabel());

  ui->slider_img_idx->setMinimum(0);
  ui->slider_img_idx->setMaximum(0);

  connect(ui->slider_img_idx, SIGNAL(valueChanged(int)), this, SLOT(SetImage(int)));
  connect(ui->pb_prev_img, SIGNAL(clicked()), this, SLOT(DecrementImgIdxSlider()));
  connect(ui->pb_next_img, SIGNAL(clicked()), this, SLOT(IncrementImgIdxSlider()));

  STD_RT_ERR_E(mio::FileExists(IMG_LIST_VIEWER_EARTH_JPEG_DIR"/earth.jpg"))
  std::vector<std::string> img_file_name_vec = {"earth_.jpg"};
//  SetImageList(IMG_LIST_VIEWER_EARTH_JPEG_DIR, img_file_name_vec);
//  mio::GetDirList("/home/spetroce/code/src/testImages", std::vector<std::string>(), "tiff", img_file_name_vec);
  SetImageList("/home/spetroce/code/src/testImages", img_file_name_vec);
}


ImageListViewer::~ImageListViewer(){
  delete ui;
}


//  QString load_dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), 
                                                       //QString(image_dir_), QFileDialog::ShowDirsOnly);
//  std::vector<std::string> file_prefix_vec = {"R__", "G__", "B__"};

//        mio::FileDescript file_desc;
//        file_desc.file_name_ = file_name;
//        mio::TimeStamp ts;
//        sscanf(file_name.c_str(), "%*s__%d-%d-%d__%d-%d-%d.%*s", ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
//        file_desc.epoch_time_ = mio::EpochTime(ts);
//        img_file_desc_vec_[j].push_back(file_desc);


void ImageListViewer::SetImageList(std::string file_path, const std::vector<std::string> &img_file_name_vec){
  EXP_CHK_E(img_file_name_vec.size() > 0, return)
  const size_t img_file_name_vec_size = img_file_name_vec.size();
  img_vec_.resize(img_file_name_vec_size);
  img_file_name_vec_.resize(img_file_name_vec_size);
  size_t num_valid_img = 0;
  mio::FormatFilePath(file_path);
  std::string file_full;
  for(std::string file_name : img_file_name_vec){
    try{
      file_full = file_path + "/" + file_name;
      EXP_CHK_EM(mio::FileExists(file_full), continue, "file_full=" + file_full)
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
}


void ImageListViewer::DecrementImgIdxSlider(){
  ui->slider_img_idx->setValue(ui->slider_img_idx->value() - 1);
}


void ImageListViewer::IncrementImgIdxSlider(){
  ui->slider_img_idx->setValue(ui->slider_img_idx->value() + 1);
}

