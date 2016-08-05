#ifndef __FLY_CAP_SETTING_EDITOR_H__
#define __FLY_CAP_SETTING_EDITOR_H__

#include <map>
#include <QLabel>
#include <QCheckBox>
#include "mio/qt/advSliderWidget/advSliderWidget.h"

class UiPropertySetter : public QWidget{
  public:
    QHBoxLayout *layout_;
    QLabel *label_;
    AdvSlider *adv_slider_;
    DblAdvSlider *dbl_adv_slider_;
    QCheckBox *chb_auto_, *chb_on_off_, *chb_one_push_;
    QFrame *line_[3];
    //FlyCapture2::PropertyInfo prop_info_;
    FlyCapture2::Property prop_;

    UiPropertySetter(){
      layout_ = nullptr;
      label_ = nullptr;
      adv_slider_ = nullptr;
      dbl_adv_slider_ = nullptr;
      chb_auto_ = chb_on_off_ = chb_one_push_ = nullptr;
      line_[0] = line_[1] = line_[2] = nullptr;
    }

    ~UiPropertySetter(){
      if(layout_) delete layout_;
      if(label_) delete label_;
      if(adv_slider_) delete adv_slider_;
      if(dbl_adv_slider_) delete dbl_adv_slider_;
      if(chb_auto_) delete chb_auto_;
      if(chb_on_off_) delete chb_on_off_;
      if(chb_one_push_) delete chb_one_push_;
    }

    //TODO - each element should get linked to slot which updated prop_ and then calls cam_->setProperty(prop_)
    //TODO - all of these prop_ structs can get saved to an XML file and loaded (prop_info_ is only needed to set 
    //       up the UI, which we can get from the camera on startup)
    void Setup(const QString label, const FlyCapture2::PropertyInfo &prop_info, const FlyCapture2::Property &prop){
      //prop_info_ = prop_;
      prop_ = prop;

      layout_ = new QHBoxLayout();
//      for(size_t i = 0; i < 3; ++i){
//        line_[i] = new QFrame();
//        line_[i]->setFrameShape(QFrame::VLine);
//        line_[i]->setFrameShadow(QFrame::Sunken);
//      }

      label_ = new QLabel();
      label_->setText(label);
      layout_->addWidget(label_);

      if(prop_info.manualSupported){
        if(prop_info.absValSupported){
          dbl_adv_slider_ = new DblAdvSlider(prop_info.absMin, prop_info.absMax, prop.absValue,
                                             3, prop_info.absMin, prop_info.absMax);
          layout_->addWidget(dbl_adv_slider_);
        }
        else{
          adv_slider_ = new AdvSlider(prop_info.min, prop_info.max, prop.valueA,
                                      prop_info.min, prop_info.max);
          layout_->addWidget(adv_slider_);
        }
      }
      if(prop_info.autoSupported){
        chb_auto_ = new QCheckBox();
        chb_auto_->setText("auto");
        chb_auto_->setCheckState(prop.autoManualMode ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_auto_);
      }
      if(prop_info.onOffSupported){
        chb_on_off_ = new QCheckBox();
        chb_on_off_->setText("on/off");
        chb_on_off_->setCheckState(prop.onOff ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_on_off_);
      }
      if(prop_info.onePushSupported){
        chb_one_push_ = new QCheckBox();
        chb_one_push_->setText("prop_info");
        chb_one_push_->setCheckState(prop.onePush ? Qt::Checked : Qt::Unchecked);
        layout_->addWidget(chb_one_push_);
      }

      setLayout(layout_);
      setFixedHeight( sizeHint().height() );
    }
};


class FlyCapControl : public QWidget{
  public:
    FlyCapture2::Camera *cam_;
    //std::map<FlyCapture2::PropertyType, FlyCapture2::PropertyInfo> prop_info_map_;
    //std::map<FlyCapture2::PropertyType, FlyCapture2::Property> prop_map_;
    std::map<FlyCapture2::PropertyType, UiPropertySetter*> ui_prop_setter_map_;
    bool absolute_mode_;
    QVBoxLayout *layout_;

    FlyCapControl(){
      layout_ = nullptr;
    }

    ~FlyCapControl(){
      if(layout_) delete layout_;
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
      for(size_t i = 0; i < prop_type_vec.size(); ++i){
        prop_info.type = prop_type_vec[i];
        PGR_ERR_OK(cam_->GetPropertyInfo(&prop_info), continue)

        if(prop_info.present){
          prop.type = prop_type_vec[i];
          PGR_ERR_OK(cam_->GetProperty(&prop), continue)
          //prop_info_map_[ prop_type_vec[i] ] = prop_info;
          //prop_map_[ prop_type_vec[i] ] = prop;
          ui_prop_setter_map_[ prop_type_vec[i] ] = new UiPropertySetter();
          UiPropertySetter *ui_prop_setter = ui_prop_setter_map_[ prop_type_vec[i] ];
          ui_prop_setter->Setup(QString::fromStdString(prop_type_name_vec[i]), prop_info, prop);
          layout_->addWidget(ui_prop_setter);
        }
      }

      setLayout(layout_);
      setFixedHeight( sizeHint().height() );
    }
};

#endif //__FLY_CAP_SETTING_EDITOR_H__

