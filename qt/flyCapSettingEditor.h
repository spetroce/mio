#ifndef __FLY_CAP_SETTING_EDITOR_H__
#define __FLY_CAP_SETTING_EDITOR_H__

#include <map>
#include <mutex>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QFile>
#include "mio/qt/advSliderWidget/advSliderWidget.h"
#include "mio/altro/ptGreyFlyCap2.h"
#include "mio/altro/freqBuffer.h"
#include "mio/altro/io.h"
#include "mio/qt/qtXml.h"


namespace mio{

class UiPropertySetter : public QWidget{
  Q_OBJECT

  bool is_init_;
  QHBoxLayout *layout_;
  QLabel *label_;
  AdvSlider *val_a_slider_;
  DblAdvSlider *abs_val_slider_;
  QCheckBox *chb_auto_, *chb_on_off_, *chb_one_push_;
  FlyCapture2::PropertyInfo prop_info_;
  FlyCapture2::Property prop_;
  FlyCapture2::Camera *cam_;
  std::mutex *cam_mtx_;
  mio::CFreqBuffer<double> abs_val_freq_buf_;
  mio::CFreqBuffer<int> val_a_freq_buf_;
  int dbg_count_;
  bool from_update_ui_;

  static void AbsValFreqBufCallBack(double value, void *user_data){
    static_cast<mio::UiPropertySetter*>(user_data)->SetCameraProp();
  }

  static void ValAFreqBufCallBack(int value, void *user_data){
    static_cast<mio::UiPropertySetter*>(user_data)->SetCameraProp();
  }

  public:
    UiPropertySetter(int id) : is_init_(false), dbg_count_(0), from_update_ui_(false){
      layout_ = nullptr;
      label_ = nullptr;
      val_a_slider_ = nullptr;
      abs_val_slider_ = nullptr;
      chb_auto_ = chb_on_off_ = chb_one_push_ = nullptr;
    }

    ~UiPropertySetter(){
      if(label_) delete label_;
      if(abs_val_slider_){
        abs_val_freq_buf_.Uninit();
        disconnect(abs_val_slider_, SIGNAL(valueChanged(double)), this, SLOT(SetCameraProp()));
        delete abs_val_slider_;
      }
      if(val_a_slider_){
        val_a_freq_buf_.Uninit();
        disconnect(val_a_slider_, SIGNAL(valueChanged(int)), this, SLOT(SetCameraProp()));
        delete val_a_slider_;
      }
      if(chb_auto_){
        disconnect(chb_auto_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
        delete chb_auto_;
      }
      if(chb_on_off_){
        disconnect(chb_on_off_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
        delete chb_on_off_;
      }
      if(chb_one_push_){
        disconnect(chb_one_push_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
        delete chb_one_push_;
      }
      if(layout_) delete layout_;
    }

    void SetCamera(FlyCapture2::Camera *cam, std::mutex *cam_mtx){
      cam_ = cam;
      cam_mtx_ = cam_mtx;
    }

    FlyCapture2::PropertyInfo GetPropertyInfo(){
      return prop_info_;
    }

    FlyCapture2::Property GetProperty(){
      return prop_;
    }

    void Setup(const QString label, const FlyCapture2::PropertyInfo &prop_info, const FlyCapture2::Property &prop){
      STD_LOG_ERR_EM(!is_init_, "called this function twice")
      prop_info_ = prop_info;
      prop_ = prop;

      layout_ = new QHBoxLayout();
      label_ = new QLabel();
      label_->setText(label);
      layout_->addWidget(label_);

      if(prop_info.manualSupported){
        const size_t kFreq = 5, kBufSize = 5, kNumDecimal = 1;
        if(prop_info.absValSupported){
          abs_val_slider_ = new DblAdvSlider(prop_info.absMin, prop_info.absMax, prop.absValue,
                                             kNumDecimal, prop_info.absMin, prop_info.absMax);
          layout_->addWidget(abs_val_slider_);
          abs_val_freq_buf_.Init(AbsValFreqBufCallBack, kFreq, kBufSize, false, static_cast<void*>(this));
          connect(abs_val_slider_, SIGNAL(valueChanged(double)), this, SLOT(AbsValSliderValueChanged(double)));
        }
        else{
          val_a_slider_ = new AdvSlider(prop_info.min, prop_info.max, prop.valueA,
                                      prop_info.min, prop_info.max);
          layout_->addWidget(val_a_slider_);
          val_a_freq_buf_.Init(ValAFreqBufCallBack, kFreq, kBufSize, false, static_cast<void*>(this));
          connect(val_a_slider_, SIGNAL(valueChanged(int)), this, SLOT(ValASliderValueChanged(int)));
        }
      }
      if(prop_info.autoSupported){
        chb_auto_ = new QCheckBox();
        chb_auto_->setText("auto");
        chb_auto_->setCheckState(prop.autoManualMode ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_auto_);
        connect(chb_auto_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
      }
      if(prop_info.onOffSupported){
        chb_on_off_ = new QCheckBox();
        chb_on_off_->setText("on/off");
        chb_on_off_->setCheckState(prop.onOff ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_on_off_);
        connect(chb_on_off_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
        
      }
      if(prop_info.onePushSupported){
        chb_one_push_ = new QCheckBox();
        chb_one_push_->setText("onePush");
        chb_one_push_->setCheckState(prop.onePush ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_one_push_);
        connect(chb_one_push_, SIGNAL(clicked()), this, SLOT(SetCameraProp()));
      }

      setLayout(layout_);
      setFixedHeight( sizeHint().height() );
      is_init_ = true;
    }

    void UpdateUi(){
      EXP_CHK_E(is_init_, return)
      PGR_ERR_VAR
      cam_mtx_->lock();
      PGR_ERR_OK(cam_->GetProperty(&prop_), return)
      cam_mtx_->unlock();
      if(abs_val_slider_){
        abs_val_slider_->setValue(prop_.absValue);
        from_update_ui_ = true;
      }
      else if(val_a_slider_){
        val_a_slider_->setValue(prop_.valueA);
        from_update_ui_ = true;
      }
      if(chb_auto_)
        chb_auto_->setCheckState(prop_.autoManualMode ? Qt::Checked : Qt::Unchecked);
      if(chb_on_off_)
        chb_on_off_->setCheckState(prop_.onOff ? Qt::Checked : Qt::Unchecked);
      if(chb_one_push_)
        chb_one_push_->setCheckState(prop_.onePush ? Qt::Checked : Qt::Unchecked);
    }

  private slots:
    void AbsValSliderValueChanged(double value){
      if(!from_update_ui_)
        abs_val_freq_buf_.Push(value);
      else
        from_update_ui_ = false;
    }

    void ValASliderValueChanged(int value){
      if(!from_update_ui_)
        val_a_freq_buf_.Push(value);
      else
        from_update_ui_ = false;
    }

    void SetCameraProp(){
      //printf("%s - %d\n", CURRENT_FUNC, dbg_count_++);
      EXP_CHK_E(is_init_, return)
      PGR_ERR_VAR
      if(abs_val_slider_)
        prop_.absValue = abs_val_slider_->value();
      else if(val_a_slider_)
        prop_.valueA = val_a_slider_->value();
      if(chb_auto_)
        prop_.autoManualMode = (chb_auto_->checkState() == Qt::Checked);
      if(chb_on_off_)
        prop_.onOff = (chb_on_off_->checkState() == Qt::Checked);
      if(chb_one_push_)
        prop_.onePush = (chb_one_push_->checkState() == Qt::Checked);
      cam_mtx_->lock();
      PGR_ERR_OK(cam_->SetProperty(&prop_), return)
      cam_mtx_->unlock();
      emit ControlChanged(static_cast<int>(prop_.type));
    }

  signals:
    void ControlChanged(int type);
};


class FlyCapControl : public QWidget{
  Q_OBJECT

  public:
    FlyCapture2::Camera *cam_;
    std::mutex *cam_mtx_;
    std::map<FlyCapture2::PropertyType, UiPropertySetter*> ui_prop_setter_map_;
    bool absolute_mode_;
    QVBoxLayout *layout_;

    const std::vector<FlyCapture2::PropertyType> kPropTypeVec_ =
        {FlyCapture2::BRIGHTNESS,
         FlyCapture2::AUTO_EXPOSURE,
         FlyCapture2::SHARPNESS,
         FlyCapture2::WHITE_BALANCE,
         FlyCapture2::HUE,
         FlyCapture2::SATURATION,
         FlyCapture2::GAMMA,
         //FlyCapture2::IRIS,
         //FlyCapture2::FOCUS,
         //FlyCapture2::ZOOM,
         //FlyCapture2::PAN,
         //FlyCapture2::TILT,
         FlyCapture2::SHUTTER,
         FlyCapture2::GAIN,
         //FlyCapture2::TRIGGER_MODE,
         //FlyCapture2::TRIGGER_DELAY,
         FlyCapture2::FRAME_RATE};
         //FlyCapture2::TEMPERATURE,
         //FlyCapture2::UNSPECIFIED_PROPERTY_TYPE,
         //FlyCapture2::PROPERTY_TYPE_FORCE_32BITS};

    //TODO: the units given for some of the strings will be incorrect if absValSupported is false for a particular property
    const std::vector<std::string> kPropTypeNameVec_ =
        {"Brightness (%)",
         "Auto Exposure (EV)",
         "Sharpness",
         "White Balance",
         "Hue",
         "Saturation",
         "Gamma",
         //"Iris",
         //"Focus",
         //"Zoom",
         //"Pan",
         //"Tilt",
         "Shutter (ms)",
         "Gain (dB)",
         //"Trigger Mode",
         //"Trigger Delay",
         "Frame Rate"};
         //"Temperature",
         //"Unspecified Property Type",
         //"Property Type Force 32bits"};

    FlyCapControl(){
      STD_INVALID_ARG_E(kPropTypeVec_.size() == kPropTypeNameVec_.size())
      layout_ = nullptr;
      Qt::WindowFlags flags = windowFlags();
      flags |= Qt::CustomizeWindowHint;
      flags &= ~Qt::WindowCloseButtonHint;
      flags &= ~Qt::WindowMinimizeButtonHint;
      flags &= ~Qt::WindowMaximizeButtonHint;
      //flags |= Qt::Tool;
      setWindowFlags(flags);
    }

    ~FlyCapControl(){
      for(auto &pair : ui_prop_setter_map_)
        disconnect(pair.second, SIGNAL(ControlChanged(int)), this, SLOT(UpdateUi(int)));
      for(auto &pair : ui_prop_setter_map_)
        delete pair.second;
      if(layout_) delete layout_;
    }

    void SetCamera(FlyCapture2::Camera *cam, std::mutex *cam_mtx){
      cam_ = cam;
      cam_mtx_ = cam_mtx;
    }

    void Setup(){
      PGR_ERR_VAR
      layout_ = new QVBoxLayout();

      FlyCapture2::Property prop;
      FlyCapture2::PropertyInfo prop_info;
      int id = 0;
      for(size_t i = 0; i < kPropTypeVec_.size(); ++i){
        prop_info.type = kPropTypeVec_[i];
        PGR_ERR_OK(cam_->GetPropertyInfo(&prop_info), continue)

        if(prop_info.present){
          prop.type = kPropTypeVec_[i];
          PGR_ERR_OK(cam_->GetProperty(&prop), continue)
          ui_prop_setter_map_[ kPropTypeVec_[i] ] = new UiPropertySetter(id);
          ++id;
          UiPropertySetter *ui_prop_setter = ui_prop_setter_map_[ kPropTypeVec_[i] ];
          cam_mtx_->lock();
          ui_prop_setter->SetCamera(cam_, cam_mtx_);
          cam_mtx_->unlock();
          ui_prop_setter->Setup(QString::fromStdString(kPropTypeNameVec_[i]), prop_info, prop);
          layout_->addWidget(ui_prop_setter);
        }
      }

      for(auto &pair : ui_prop_setter_map_)
        connect(pair.second, SIGNAL(ControlChanged(int)), this, SLOT(UpdateUi(int)));

      setLayout(layout_);
      setFixedHeight( sizeHint().height() );
    }

    void SaveCameraSettings(QString file_full_qstr){
      EXP_CHK_E(!file_full_qstr.isEmpty(), return)
      std::string file_full = file_full_qstr.toStdString(), file_path, file_name_no_ext;
      mio::FileNameExpand(file_full, ".", &file_path, NULL, NULL, NULL);
      EXP_CHK_EM(mio::DirExists(file_path), return, file_path + "is not an existing directory")
      ForceXmlExtension(file_full_qstr);
      file_full = file_full_qstr.toStdString();
      printf("%s - saving to %s\n", CURRENT_FUNC, file_full.c_str());

      QFile file(file_full_qstr);
      EXP_CHK_E(file.open(QIODevice::WriteOnly),
                QMessageBox::warning(0, "Read only", "The file is in read only mode");return)

      QXmlStreamWriter xml_writer(&file);
      xml_writer.setAutoFormatting(true);
      xml_writer.writeStartDocument(); //write XML version number
      xml_writer.writeStartElement("FlyCaptureProperties");

      for(const auto &pair : ui_prop_setter_map_){
        FlyCapture2::Property prop = pair.second->GetProperty();

        xml_writer.writeStartElement("Property");
          xml_writer.writeAttribute("type", INT_TO_QSTR(static_cast<int>(prop.type)));
          xml_writer.writeAttribute("present", BOOL_TO_QSTR(prop.present));
          xml_writer.writeAttribute("absControl", BOOL_TO_QSTR(prop.absControl));
          xml_writer.writeAttribute("onePush", BOOL_TO_QSTR(prop.onePush));
          xml_writer.writeAttribute("onOff", BOOL_TO_QSTR(prop.onOff));
          xml_writer.writeAttribute("autoManualMode", BOOL_TO_QSTR(prop.autoManualMode));
          xml_writer.writeAttribute("valueA", UINT_TO_QSTR(prop.valueA));
          xml_writer.writeAttribute("valueB", UINT_TO_QSTR(prop.valueB));
          xml_writer.writeAttribute("absValue", FLT_TO_QSTR(prop.absValue));
          for(size_t i = 0; i < 8; ++i){
            char str[16];
            snprintf(str, 16, "reserved%lu", i);
            xml_writer.writeAttribute(str, UINT_TO_QSTR(prop.reserved[i]));
          }
        xml_writer.writeEndElement(); //end Property
      }

      xml_writer.writeEndElement(); //end FlyCaptureProperties
      xml_writer.writeEndDocument();
      file.close();
    }

    void LoadCameraSettings(const QString file_full_qstr){
      PGR_ERR_VAR
      EXP_CHK_E(!file_full_qstr.isEmpty(), return)
      std::string file_full = file_full_qstr.toStdString();
      EXP_CHK_E(mio::FileExists(file_full), return)
      printf("%s - loading from %s\n", CURRENT_FUNC, file_full.c_str());

      QFile file(file_full_qstr);
      EXP_CHK_E(file.open(QIODevice::ReadOnly | QIODevice::Text),
                QMessageBox::warning(0, "FlyCapControl::LoadCameraSettings", "Couldn't open xml file");return)

      QXmlStreamAttributes attr;
      QXmlStreamReader xml_reader(&file);
      while( !xml_reader.atEnd() && !xml_reader.hasError() ){
        xml_reader.readNext();
        if(xml_reader.isStartDocument()) //skip StartDocument tokens
          continue;
        if(xml_reader.isStartElement() && xml_reader.name() == "FlyCaptureProperties"){
          // iterate through children of FlyCaptureProperties
          while(!xml_reader.atEnd() && !xml_reader.hasError()){
            xml_reader.readNext();
            if(xml_reader.isStartElement() && xml_reader.name() == "Property"){
              attr = xml_reader.attributes();
              FlyCapture2::Property prop;
              if(attr.hasAttribute("type"))
                prop.type = static_cast<FlyCapture2::PropertyType>(ATTR_TO_INT(attr, "type"));
              if(attr.hasAttribute("present"))
                prop.present = ATTR_TO_BOOL(attr, "present");
              if(attr.hasAttribute("absControl"))
                prop.absControl = ATTR_TO_BOOL(attr, "absControl");
              if(attr.hasAttribute("onePush"))
                prop.onePush = ATTR_TO_BOOL(attr, "onePush");
              if(attr.hasAttribute("onOff"))
                prop.onOff = ATTR_TO_BOOL(attr, "onOff");
              if(attr.hasAttribute("autoManualMode"))
                prop.autoManualMode = ATTR_TO_BOOL(attr, "autoManualMode");
              if(attr.hasAttribute("valueA"))
                prop.valueA = ATTR_TO_UINT(attr, "valueA");
              if(attr.hasAttribute("valueB"))
                prop.valueB = ATTR_TO_UINT(attr, "valueB");
              if(attr.hasAttribute("absValue"))
                prop.absValue = ATTR_TO_FLT(attr, "absValue");
              for(size_t i = 0; i < 8; ++i){
                char str[16];
                snprintf(str, 16, "reserved%lu", i);
                if(attr.hasAttribute(str))
                  prop.reserved[i] = ATTR_TO_UINT(attr, str);
              }

              auto it = ui_prop_setter_map_.find(prop.type);
              if(it != ui_prop_setter_map_.end()){
                mio::PrintProperty(prop);
                cam_mtx_->lock();
                PGR_ERR_OK(cam_->SetProperty(&prop), continue)
                cam_mtx_->unlock();
                (*it).second->UpdateUi();
              }
            }
            else if(xml_reader.isEndElement() && xml_reader.name() == "FlyCaptureProperties")
              break;
          }
        }
      }

      //error handling
      if( xml_reader.hasError() )
        QMessageBox::critical(this, "QXSRExample::parseXML", xml_reader.errorString(), QMessageBox::Ok);
      xml_reader.clear(); //removes any device() or data from the reader and resets its internal state
      file.close();
    }

  private slots:
    void UpdateUi(int type){
      for(auto &pair : ui_prop_setter_map_)
        if(static_cast<int>(pair.second->GetProperty().type) != type)
          pair.second->UpdateUi();
    }
};

} //namespace mio

#endif //__FLY_CAP_SETTING_EDITOR_H__

