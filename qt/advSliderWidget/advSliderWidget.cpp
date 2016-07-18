#include "advSliderWidget.h"
#include <cmath>


CAdvSliderWidget::CAdvSliderWidget(QWidget *parent) : QWidget(parent), is_init_(false), flag_(false){
}


CAdvSliderWidget::CAdvSliderWidget(const int min, const int max, const int value, const int abs_min,
                                   const int abs_max, const int step_size, const QString label_text, QWidget *parent) :
                                   QWidget(parent), is_init_(false), flag_(false){
  Init(min, max, value, abs_min, abs_max, step_size, label_text);
}


CAdvSliderWidget::~CAdvSliderWidget(){
  disconnect( min_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetMinimum(int) ) );
  disconnect( max_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetMaximum(int) ) );
  disconnect( slider_, SIGNAL( sliderMoved(int) ), this, SLOT( SetValueSpinBox(int) ) );
  disconnect( value_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetSliderValue(int) ) );
  disconnect( value_spin_box_, SIGNAL( valueChanged(int) ), this, SIGNAL( valueChanged(int) ) );
  //disconnect( this, SIGNAL( valueChanged(int) ), this, SLOT( PrintValue(int) ) );

  delete label_;
  delete value_spin_box_;
  delete min_spin_box_;
  delete max_spin_box_;
  delete slider_;
  for(size_t i = 0; i < 3; ++i)
    delete line_[i];
  delete layout_;
}


void CAdvSliderWidget::Init(){
  label_ = new QLabel();
  layout_ = new QHBoxLayout();
  value_spin_box_ = new QSpinBox();
  min_spin_box_ = new QSpinBox();
  max_spin_box_ = new QSpinBox();
  slider_ = new QSlider(Qt::Horizontal);

  connect( min_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetMinimum(int) ) );
  connect( max_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetMaximum(int) ) );
  //signal sliderMoved() is emitted when the user drags the slider
  //when slider_ is moved by the user, it tells value_spin_box_ to update its value, but then value_spin_box_ 
  //tells slider_ to update its value.  This happens because we don't have a signal valueChangedFromClick() for 
  //value_spin_box_. The solution is to use flag_.
  connect( slider_, SIGNAL( sliderMoved(int) ), this, SLOT( SetValueSpinBox(int) ) );
  connect( value_spin_box_, SIGNAL( valueChanged(int) ), this, SLOT( SetSliderValue(int) ) );
  connect( value_spin_box_, SIGNAL( valueChanged(int) ), this, SIGNAL( valueChanged(int) ) );
  //connect( this, SIGNAL( valueChanged(int) ), this, SLOT( PrintValue(int) ) );

  min_spin_box_->setMinimum(abs_min_);
  min_spin_box_->setMaximum(abs_max_);
  max_spin_box_->setMinimum(abs_min_);
  max_spin_box_->setMaximum(abs_max_);

  slider_->setMinimum(min_);
  slider_->setMaximum(max_);
  value_spin_box_->setMinimum(min_);
  value_spin_box_->setMaximum(max_);
  min_spin_box_->setValue(min_);
  max_spin_box_->setValue(max_);

  slider_->setSingleStep(step_size_);
  value_spin_box_->setSingleStep(step_size_);
  min_spin_box_->setSingleStep(step_size_);
  max_spin_box_->setSingleStep(step_size_);

  value_spin_box_->setValue(value_);

  for(size_t i = 0; i < 3; ++i){
    line_[i] = new QFrame();
    line_[i]->setFrameShape(QFrame::VLine);
    line_[i]->setFrameShadow(QFrame::Sunken);
  }

  if(!label_text_.isEmpty()){
    label_->setText(label_text_);
    layout_->addWidget(label_);
  }
  layout_->addWidget(value_spin_box_);
  layout_->addWidget(line_[0]);
  layout_->addWidget(min_spin_box_);
  layout_->addWidget(line_[1]);
  layout_->addWidget(slider_);
  layout_->addWidget(line_[2]);
  layout_->addWidget(max_spin_box_);

  setLayout(layout_);
  setFixedHeight( sizeHint().height() );

  is_init_ = true;
}


void CAdvSliderWidget::Init(const int min, const int max, const int value, const int abs_min,
                            const int abs_max, const int step_size, const QString label_text){
  if(abs_min == abs_max)
    printf("CAdvSliderWidget::vInit() - error: abs_min == abs_max\n");
  else{
    if(!is_init_){
      label_text_ = label_text;
      abs_min_ = abs_min;
      abs_max_ = abs_max;
      min_ = min < abs_min_ ? abs_min_ : min;
      max_ = max > abs_max_ ? abs_max_ : max;
      value_ = value < min_ || value > max_ ? min_ : value;
      step_size_ = step_size;
      Init();
    }
  }
}


void CAdvSliderWidget::SetValueSpinBox(const int value){
  flag_ = true;
  value_spin_box_->setValue(value);
}


void CAdvSliderWidget::SetSliderValue(const int value){
  //check if this came from SetValueSpinBox() being called from a signal/slot
  if(flag_)
    flag_ = false;
  else
    slider_->setValue(value);
}


void CAdvSliderWidget::SetMinimum(const int min){
  const int cur_max = max_spin_box_->value();
  if(min < cur_max){
    slider_->setMinimum(min);
    value_spin_box_->setMinimum(min);
  }
  else
    min_spin_box_->setValue(cur_max - 1);
}


void CAdvSliderWidget::SetMaximum(const int max){
  const int cur_min = min_spin_box_->value();
  if(max > cur_min){
    slider_->setMaximum(max);
    value_spin_box_->setMaximum(max);
  }
  else
    max_spin_box_->setValue(cur_min + 1);
}


//public set/get func

void CAdvSliderWidget::setValue(const int value){
  if(is_init_)
    value_spin_box_->setValue(value);
}

int CAdvSliderWidget::value(){
  return is_init_ ? value_spin_box_->value() : 0;
}


int CAdvSliderWidget::min(){
  return is_init_ ? min_spin_box_->value() : 0;
}


void CAdvSliderWidget::setMinimum(const int min){
  if(is_init_)
    min_spin_box_->setValue(min);
}


int CAdvSliderWidget::max(){
  return is_init_ ? max_spin_box_->value() : 0;
}


void CAdvSliderWidget::setMaximum(const int max){
  if(is_init_)
    max_spin_box_->setValue(max);
}


void CAdvSliderWidget::setSingleStep(const int step_size){
  if(is_init_){
    step_size_ = step_size;
    slider_->setSingleStep(step_size_);
    value_spin_box_->setSingleStep(step_size_);
    min_spin_box_->setSingleStep(step_size_);
    max_spin_box_->setSingleStep(step_size_);
  }
}

int CAdvSliderWidget::singleStep(){
  return is_init_ ? step_size_ : 0;
}


void CAdvSliderWidget::setReadOnly(const bool val_read_only, const bool min_read_only, const bool max_read_only){
  if(is_init_){
    value_spin_box_->setReadOnly(val_read_only);
    min_spin_box_->setReadOnly(min_read_only);
    max_spin_box_->setReadOnly(max_read_only);
  }
}

bool CAdvSliderWidget::readOnly(){
  return is_init_ ? value_spin_box_->isReadOnly() : false;
}


void CAdvSliderWidget::setEnabled(const bool enabled){
  value_spin_box_->setEnabled(enabled);
  min_spin_box_->setEnabled(enabled);
  slider_->setEnabled(enabled);
  max_spin_box_->setEnabled(enabled);
}


/*** CDoubleAdvSliderWidget ***/

double CDoubleAdvSliderWidget::pow_of_ten[] = {1.0f, //10^0
                                               10.0f,
                                               100.0f,
                                               1000.0f,
                                               10000.0f,
                                               100000.0f,
                                               1000000.0f, //10^6
                                               10000000.0f,
                                               100000000.0f,
                                               1000000000.0f,
                                               10000000000.0f,
                                               100000000000.0f,
                                               1000000000000.0f}; //10^12

double CDoubleAdvSliderWidget::pow_of_ten_inv[] = {1.0f, //10^0
                                                   0.1f,
                                                   0.01f,
                                                   0.001f,
                                                   0.0001f,
                                                   0.00001f,
                                                   0.000001f, //10^-6
                                                   0.0000001f,
                                                   0.00000001f,
                                                   0.000000001f,
                                                   0.0000000001f,
                                                   0.00000000001f,
                                                   0.000000000001f}; //10^-12


CDoubleAdvSliderWidget::CDoubleAdvSliderWidget(QWidget *parent) : QWidget(parent), is_init_(false), flag_(false){
}


//TODO: step_size is ignored, setSingleStep() is called within setDecimals(). To properly use setSingleStep(), the
//value given must be less than or equal to pow_of_ten_inv[num_decimal_].
CDoubleAdvSliderWidget::CDoubleAdvSliderWidget(const double min, const double max, const double value,
                                               const int num_decimal, const double abs_min,  const double abs_max,
                                               const double step_size, const QString label_text, QWidget *parent) :
                                               QWidget(parent), is_init_(false), flag_(false){
  Init(min, max, value, num_decimal, abs_min, abs_max, step_size, label_text);
}


CDoubleAdvSliderWidget::~CDoubleAdvSliderWidget(){
  disconnect( min_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetMinimum(double) ) );
  disconnect( max_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetMaximum(double) ) );
  disconnect( slider_, SIGNAL( sliderMoved(int) ), this, SLOT( SetValueSpinBox(int) ) ); 
  disconnect( value_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetSliderValue(double) ) );
  disconnect( value_spin_box_, SIGNAL( valueChanged(double) ), this, SIGNAL( valueChanged(double) ) );
  //disconnect( this, SIGNAL( valueChanged(double) ), this, SLOT( PrintValue(double) ) );

  delete label_;
  delete value_spin_box_;
  delete min_spin_box_;
  delete max_spin_box_;
  delete slider_;
  for(size_t i = 0; i < 3; ++i)
    delete line_[i];
  delete layout_;
}


void CDoubleAdvSliderWidget::Init(){
  label_ = new QLabel();
  layout_ = new QHBoxLayout();
  value_spin_box_ = new QDoubleSpinBox();
  min_spin_box_ = new QDoubleSpinBox();
  max_spin_box_ = new QDoubleSpinBox();
  slider_ = new QSlider(Qt::Horizontal);  

  setDecimals(num_decimal_);

  connect( min_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetMinimum(double) ) );
  connect( max_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetMaximum(double) ) );
  connect( slider_, SIGNAL( sliderMoved(int) ), this, SLOT( SetValueSpinBox(int) ) ); 
  connect( value_spin_box_, SIGNAL( valueChanged(double) ), this, SLOT( SetSliderValue(double) ) );
  connect( value_spin_box_, SIGNAL( valueChanged(double) ), this, SIGNAL( valueChanged(double) ) );
  //connect( this, SIGNAL( valueChanged(double) ), this, SLOT( PrintValue(double) ) );

  min_spin_box_->setMinimum(abs_min_);
  min_spin_box_->setMaximum(abs_max_);
  max_spin_box_->setMinimum(abs_min_);
  max_spin_box_->setMaximum(abs_max_);

  slider_->setMinimum(min_ * pow_of_ten[num_decimal_]);
  slider_->setMaximum(max_ * pow_of_ten[num_decimal_]);
  value_spin_box_->setMinimum(min_);
  value_spin_box_->setMaximum(max_);
  min_spin_box_->setValue(min_);
  max_spin_box_->setValue(max_);

  value_spin_box_->setValue(value_);

  for(size_t i = 0; i < 3; ++i){
    line_[i] = new QFrame();
    line_[i]->setFrameShape(QFrame::VLine);
    line_[i]->setFrameShadow(QFrame::Sunken);
  }

  //min_spin_box_->setFixedWidth( min_spin_box_->sizeHint().width() );

  if(!label_text_.isEmpty()){
    label_->setText(label_text_);
    label_->setFixedWidth( label_->sizeHint().width() );
    layout_->addWidget(label_);
  }
  layout_->addWidget(value_spin_box_);
  layout_->addWidget(line_[0]);
  layout_->addWidget(min_spin_box_);
  layout_->addWidget(line_[1]);
  layout_->addWidget(slider_);
  layout_->addWidget(line_[2]);
  layout_->addWidget(max_spin_box_);

  setLayout(layout_);
  setFixedHeight( sizeHint().height() );

  is_init_ = true;
}


void CDoubleAdvSliderWidget::Init(const double min, const double max, const double value, const int num_decimal,
                                  const double abs_min, const double abs_max, const double step_size,
                                  const QString label_text){
  if(abs_min == abs_max)
    printf("CAdvSliderWidget::vInit() - error: abs_min == abs_max\n");
  else{
    if(!is_init_){
      label_text_ = label_text;
      abs_min_ = abs_min;
      abs_max_ = abs_max;
      min_ = min < abs_min_ ? abs_min_ : min;
      max_ = max > abs_max_ ? abs_max_ : max;
      value_ = value < min_ || value > max_ ? min_ : value;
      num_decimal_ = num_decimal;
      Init();
    }
  }
}


void CDoubleAdvSliderWidget::SetValueSpinBox(const int value){
  flag_ = true;
  value_spin_box_->setValue(static_cast<double>(value) * single_step_);
}


void CDoubleAdvSliderWidget::SetSliderValue(const double value){
  //check if this came from SetValueSpinBox() being called from a signal/slot
  if(flag_)
    flag_ = false;
  else
    slider_->setValue( std::lround( value * pow_of_ten[num_decimal_] ) );
}


void CDoubleAdvSliderWidget::SetMinimum(const double min){
  const double cur_max_dbl = max_spin_box_->value();
  const int cur_max_scaled = std::lround( cur_max_dbl * pow_of_ten[num_decimal_] ),
            min_scaled = lround( min * pow_of_ten[num_decimal_] );

  if(min_scaled < cur_max_scaled){
    slider_->setMinimum(min_scaled);
    value_spin_box_->setMinimum(min);
  }
  else
    min_spin_box_->setValue(cur_max_dbl - single_step_);
}


void CDoubleAdvSliderWidget::SetMaximum(const double max){
  const double cur_min_dbl = min_spin_box_->value();
  const int cur_min_scaled = lround( cur_min_dbl * pow_of_ten[num_decimal_] ), 
            max_scaled = lround(max * pow_of_ten[num_decimal_]);

  if(max_scaled > cur_min_scaled){
    slider_->setMaximum(max_scaled);
    value_spin_box_->setMaximum(max);
  }
  else
    max_spin_box_->setValue(cur_min_dbl + single_step_);
}


//public set/get func

void CDoubleAdvSliderWidget::setValue(const double value){
  if(is_init_)
    value_spin_box_->setValue(value);
}


double CDoubleAdvSliderWidget::value(){
  return is_init_ ? value_spin_box_->value() : 0;
}


double CDoubleAdvSliderWidget::min(){
  return is_init_ ? min_spin_box_->value() : 0;
}


double CDoubleAdvSliderWidget::max(){
  return is_init_ ? max_spin_box_->value() : 0;
}


void CDoubleAdvSliderWidget::setMinimum(const double min){
  if(is_init_)
    min_spin_box_->setValue(min);
}


void CDoubleAdvSliderWidget::setMaximum(const double max){
  if(is_init_)
    max_spin_box_->setValue(max);
}


void CDoubleAdvSliderWidget::setDecimals(int num_decimal){
  if(num_decimal < 0 || num_decimal > 12){
    printf("CDoubleAdvSliderWidget::setDecimals(int) - invalid decimal value [%d], should be 0-12\n", num_decimal);
    num_decimal = 2;
  }

  num_decimal_ = num_decimal;
  min_spin_box_->setDecimals(num_decimal);
  value_spin_box_->setDecimals(num_decimal);
  max_spin_box_->setDecimals(num_decimal);
  slider_->setMinimum(min_spin_box_->value() * pow_of_ten[num_decimal_]);
  slider_->setMaximum(max_spin_box_->value() * pow_of_ten[num_decimal_]);
  slider_->setValue(slider_->value() * pow_of_ten[num_decimal_]);

  setSingleStep(pow_of_ten_inv[num_decimal_]);
}


void CDoubleAdvSliderWidget::setSingleStep(const double single_step){
  min_spin_box_->setSingleStep(single_step);
  value_spin_box_->setSingleStep(single_step);
  max_spin_box_->setSingleStep(single_step);
  single_step_ = single_step;
}


void CDoubleAdvSliderWidget::setEnabled(const bool enabled){
  value_spin_box_->setEnabled(enabled);
  min_spin_box_->setEnabled(enabled);
  slider_->setEnabled(enabled);
  max_spin_box_->setEnabled(enabled);
}


void CDoubleAdvSliderWidget::PrintAll(const double value){
  printf( "\nvalueChanged [%f]\n"
"minSpinBox min [%f]\n"
"minSpinBox max [%f]\n"
"maxSpinBox min [%f]\n"
"maxSpinBox max [%f]\n"
"slider min [%d]\n"
"slider max [%d]\n\n", value, min_spin_box_->minimum(), min_spin_box_->maximum(), 
                       max_spin_box_->minimum(), max_spin_box_->maximum(), 
                       slider_->minimum(), slider_->maximum() );
}

