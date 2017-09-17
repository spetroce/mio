#include "imageListViewer.h"
#include "ui_imageListViewer.h"
#include <QMessageBox>
#include <QFileDialog>

using namespace mio;

ImageListViewer::ImageListViewer(const bool show_earth, QWidget *parent) :
    QWidget(parent), ui(new Ui::ImageListViewer), normalize_roi_(false){
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
  connect(ui->pb_roi_norm, SIGNAL(clicked()), this, SLOT(RoiNorm()));
  connect(ui->pb_roi_save, SIGNAL(clicked()), this, SLOT(SaveRoi()));
  connect(ui->pb_roi_load, SIGNAL(clicked()), this, SLOT(LoadRoi()));
  connect(ui->comboBox_select_roi, SIGNAL(currentIndexChanged(int)), this, SLOT(SetRoiIndex(int)));

  adv_img_disp_->SetLimitView(true);
  if(show_earth)
    ShowEarth();
  ui->comboBox_select_roi->addItem("Show All");
}


ImageListViewer::~ImageListViewer(){
  delete adv_img_disp_;
  delete ui;
}


void ImageListViewer::SetImageList(std::string file_path, const std::vector<std::string> &img_file_name_vec){
  EXP_CHK(img_file_name_vec.size() > 0, adv_img_disp_->ShowStripes();return)
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
    EXP_CHK_M(mio::FileExists(file_full), continue, "file_full=" + file_full)
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

  SetImgIdxGui();
}


void ImageListViewer::SetImageList(const std::vector<std::string> &img_file_name_vec,
                                   const std::vector<cv::Mat> &img_vec, const bool clone_images){
  EXP_CHK(img_file_name_vec.size() > 0, return)
  EXP_CHK(img_file_name_vec.size() == img_vec.size(), return)

  img_file_name_vec_ = img_file_name_vec;
  if(clone_images){
    const size_t img_vec_size = img_vec.size();
    img_vec_.resize(img_vec_size);
    for(size_t i = 0; i < img_vec_size; ++i)
      img_vec_[i] = img_vec[i].clone();
  }
  else
    img_vec_ = img_vec;

  SetImgIdxGui();
}


void ImageListViewer::SetImgIdxGui(){
  STD_INVALID_ARG_E(img_vec_.size() == img_file_name_vec_.size())
  const size_t num_valid_img = img_vec_.size();

  ui->slider_img_idx->setMaximum(num_valid_img <= 1 ? 1 : num_valid_img-1);
  ui->slider_img_idx->setEnabled(num_valid_img > 1);
  ui->pb_prev_img->setEnabled(num_valid_img > 1);
  ui->pb_next_img->setEnabled(num_valid_img > 1);

  if(num_valid_img == 0)
    adv_img_disp_->ShowStripes();
  else{
    ui->slider_img_idx->setValue(0);
    SetImage(0);
  }
}


void ImageListViewer::SetImage(const int kIndex){
  EXP_CHK(kIndex >= 0 && kIndex < img_vec_.size(), return)
  adv_img_disp_->SetImage(img_vec_[kIndex], true);
  ui->lineEdit_img_name->setText(QString(img_file_name_vec_[kIndex].c_str()));
}


void ImageListViewer::SetImage(const cv::Mat &kImg, const std::string &kStr,
                               const bool kClone, const bool kCalledFromExternalThread){ 
  adv_img_disp_->SetImage(kImg, kClone, kCalledFromExternalThread);
  ui->lineEdit_img_name->setText(QString(kStr.c_str()));
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
  const size_t kRoiTypeIdx = ui->comboBox_roi_type->currentIndex();
  adv_img_disp_->CreateRoi(kRoiTypeIdx);
  ui->comboBox_select_roi->addItem(QString::number(ui->comboBox_select_roi->count()-1) +
                                   " - " + QString(Roi::roi_type_str[kRoiTypeIdx]));
}


void ImageListViewer::RemoveRoi(){
  if(ui->comboBox_select_roi->count() > 1){
    const int kComboBoxCurrentIdx = ui->comboBox_select_roi->currentIndex();
    if(kComboBoxCurrentIdx == 0 && ui->comboBox_select_roi->count() > 2){
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "Remove ROI Prompt", "Remove all ROI?",
                                    QMessageBox::Yes | QMessageBox::No);
      if(reply == QMessageBox::Yes){
        for(size_t i = ui->comboBox_select_roi->count()-1; i > 0; --i)
          ui->comboBox_select_roi->removeItem(i);
        adv_img_disp_->RemoveRoi(-1);
      }
    }
    else{
      adv_img_disp_->RemoveRoi(kComboBoxCurrentIdx == 0 ? 0 : kComboBoxCurrentIdx-1);
      ui->comboBox_select_roi->removeItem(kComboBoxCurrentIdx == 0 ? 1 : kComboBoxCurrentIdx);
    }
  }
  else
    std::cout << FL_STRM << "there aren't any ROIs to remove\n";
}


void ImageListViewer::ShowRoi(){
  adv_img_disp_->ShowRoi();
}


void ImageListViewer::RoiNorm(){
  normalize_roi_ = !normalize_roi_;
  adv_img_disp_->SetNormalizeRoi(normalize_roi_);
}


void ImageListViewer::SaveRoi(){
  QString filter = "*.xml";
  QString file_full = QFileDialog::getSaveFileName(this, tr("Save ROI"), "/home/", filter);
  if(file_full != ""){
    std::cout << FL_STRM << file_full.toStdString() << std::endl;
    adv_img_disp_->SaveRoi(file_full);
  }
}


void ImageListViewer::LoadRoi(){
  QString filter = "*.xml";
  QString file_full_qt = QFileDialog::getOpenFileName(this, tr("Open ROI"), "/home/", filter);
  std::string file_full = file_full_qt.toStdString();
  EXP_CHK_M(mio::FileExists(file_full), return, std::string("file_full=") + file_full)
  std::vector<int> loaded_roi_types;
  adv_img_disp_->LoadRoi(file_full_qt, loaded_roi_types);
  for(const int kRoiType : loaded_roi_types)
    ui->comboBox_select_roi->addItem(QString::number(ui->comboBox_select_roi->count()-1) +
                                     " - " + QString(Roi::roi_type_str[kRoiType]));
}


void ImageListViewer::SetRoiIndex(const int kIndex){
  adv_img_disp_->SetRoiIndex(kIndex-1);
}


void ImageListViewer::ShowRoiControl(const bool kShow){
  if(kShow)
    ui->widget_roiControl->show();
  else
    ui->widget_roiControl->hide();
}


void ImageListViewer::HideShowRoiControl(){
  ShowRoiControl(!ui->widget_roiControl->isVisible());
}

