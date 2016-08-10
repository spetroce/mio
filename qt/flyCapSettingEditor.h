#ifndef __FLY_CAP_SETTING_EDITOR_H__
#define __FLY_CAP_SETTING_EDITOR_H__

#include <map>
#include <mutex>
#include <QLabel>
#include <QCheckBox>
#include "mio/qt/advSliderWidget/advSliderWidget.h"
#include "mio/altro/ptGreyFlyCap2.h"
#include "mio/altro/freqBuffer.h"


namespace mio{

class UiPropertySetter : public QWidget{
  Q_OBJECT

  bool is_init_;
  int id_;
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
    UiPropertySetter(int id) : is_init_(false), dbg_count_(0), from_update_ui_(false), id_(id){
      layout_ = nullptr;
      label_ = nullptr;
      val_a_slider_ = nullptr;
      abs_val_slider_ = nullptr;
      chb_auto_ = chb_on_off_ = chb_one_push_ = nullptr;
    }

    ~UiPropertySetter(){
      if(layout_) delete layout_;
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
    }

    void SetCamera(FlyCapture2::Camera *cam, std::mutex *cam_mtx){
      cam_ = cam;
      cam_mtx_ = cam_mtx;
    }

    int GetId(){
      return id_;
    }

    //TODO - each element should get linked to slot which updated prop_ and then calls cam_->setProperty(prop_)
    //TODO - all of these prop_ structs can get saved to an XML file and loaded (prop_info_ is only needed to set 
    //       up the UI, which we can get from the camera on startup)
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
          abs_val_freq_buf_.Init(AbsValFreqBufCallBack, kFreq, kBufSize, static_cast<void*>(this));
          connect(abs_val_slider_, SIGNAL(valueChanged(double)), this, SLOT(AbsValSliderValueChanged(double)));
        }
        else{
          val_a_slider_ = new AdvSlider(prop_info.min, prop_info.max, prop.valueA,
                                      prop_info.min, prop_info.max);
          layout_->addWidget(val_a_slider_);
          val_a_freq_buf_.Init(ValAFreqBufCallBack, kFreq, kBufSize, static_cast<void*>(this));
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
      printf("%s - %d\n", CURRENT_FUNC, dbg_count_++);
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
      emit ControlChanged(id_);
    }

  signals:
    void ControlChanged(int id);
};


class FlyCapControl : public QWidget{
  Q_OBJECT

  public:
    FlyCapture2::Camera *cam_;
    std::mutex *cam_mtx_;
    std::map<FlyCapture2::PropertyType, UiPropertySetter*> ui_prop_setter_map_;
    bool absolute_mode_;
    QVBoxLayout *layout_;

    FlyCapControl(){
      layout_ = nullptr;
      for(auto &elem : ui_prop_setter_map_)
        delete elem.second;
    }

    ~FlyCapControl(){
      if(layout_) delete layout_;
    }

    void SetCamera(FlyCapture2::Camera *cam, std::mutex *cam_mtx){
      cam_ = cam;
      cam_mtx_ = cam_mtx;
    }

    void Setup(){
      PGR_ERR_VAR
      layout_ = new QVBoxLayout();

      std::vector<FlyCapture2::PropertyType> prop_type_vec =
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
      std::vector<std::string> prop_type_name_vec =
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

      STD_INVALID_ARG_E(prop_type_vec.size() == prop_type_name_vec.size())

      FlyCapture2::Property prop;
      FlyCapture2::PropertyInfo prop_info;
      int id = 0;
      for(size_t i = 0; i < prop_type_vec.size(); ++i){
        prop_info.type = prop_type_vec[i];
        PGR_ERR_OK(cam_->GetPropertyInfo(&prop_info), continue)

        if(prop_info.present){
          prop.type = prop_type_vec[i];
          PGR_ERR_OK(cam_->GetProperty(&prop), continue)
          ui_prop_setter_map_[ prop_type_vec[i] ] = new UiPropertySetter(id);
          ++id;
          UiPropertySetter *ui_prop_setter = ui_prop_setter_map_[ prop_type_vec[i] ];
          cam_mtx_->lock();
          ui_prop_setter->SetCamera(cam_, cam_mtx_);
          cam_mtx_->unlock();
          ui_prop_setter->Setup(QString::fromStdString(prop_type_name_vec[i]), prop_info, prop);
          layout_->addWidget(ui_prop_setter);
        }
      }

      for(auto &pair : ui_prop_setter_map_)
        connect(pair.second, SIGNAL(ControlChanged(int)), this, SLOT(UpdateUi(int)));

      setLayout(layout_);
      setFixedHeight( sizeHint().height() );
    }

  private slots:
    void UpdateUi(int id){
      for(auto &pair : ui_prop_setter_map_)
        if(pair.second->GetId() != id)
          pair.second->UpdateUi();
    }
};

} //namespace mio

#endif //__FLY_CAP_SETTING_EDITOR_H__

