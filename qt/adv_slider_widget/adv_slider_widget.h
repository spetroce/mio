#ifndef __ADV_SLIDER_WIDGET__
#define __ADV_SLIDER_WIDGET__

#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QLayout>
#include <QFrame>
#include <QLabel>
#include <QString>
#include <limits.h> //int limits
#include <float.h> //double limits
#include "mio/altro/freq_buffer.h"


class CAdvSliderWidget : public QWidget{
  Q_OBJECT

  public:
    /*explicit */CAdvSliderWidget(QWidget *parent = 0);
    // (min, max, value, abs_min, abs_max, step_size, label_text, parent)
    CAdvSliderWidget(const int min, const int max, const int value, const int abs_min = INT_MIN, 
                     const int abs_max = INT_MAX, const int step_size = 1, const QString label_text = QString(),
                     QWidget *parent = 0);
    ~CAdvSliderWidget();

    void Init(const int min, const int max, const int value, const int abs_min = INT_MIN,
              const int abs_max = INT_MAX, const int step_size = 1, const QString label_text = QString());
    int Value();
    int Min();
    int Max();
    void SetEnabled(bool enabled);
    int SingleStep();
    bool ReadOnly();
    void EnableFreqBuffer(const bool enable, const size_t freq = 10, const size_t max_buf_size = 5,
                          const bool with_fifo_checking = false);
    void EnableDebugPrint(const bool enable);
    void EmitOnSliderRelease(const bool enable);

  private:
    QSpinBox *max_spin_box_, *min_spin_box_, *value_spin_box_;
    QSlider *slider_;
    QHBoxLayout *layout_;
    QFrame *line_[3];
    QLabel *label_;
    int min_, max_, value_, abs_min_, abs_max_, step_size_;
    bool is_init_, flag_, freq_buffer_is_init_, emit_on_slider_release_;
    QString label_text_;
    void Init();
    void CreateInterface();
    mio::CFreqBuffer<int> freq_buffer_;
    static void FreqBufCallBack(int value, void *user_data);

  private slots:
    void SetValueSpinBox(const int value);
    void SetSliderValue(const int value);
    void SetMinimumAdv(const int min);
    void SetMaximumAdv(const int max);
    void ValueSpinBoxValueChanged(int value);
    void SliderReleased();

  public slots:
    void SetValue(const int value);
    void SetMinimum(const int min);
    void SetMaximum(const int max);
    void SetSingleStep(const int step_size);
    void SetReadOnly(const bool val_read_only, const bool min_read_only = false, const bool max_read_only = false);
    void PrintValue(const int value){ printf("val %d\n", value); }
    void PrintAll(const int value);

  signals:
    void ValueChanged(const int value);
};


class CDoubleAdvSliderWidget : public QWidget{
    Q_OBJECT

  public:
    /*explicit */CDoubleAdvSliderWidget(QWidget *parent = 0);
    // (min, max, value, num_decimal, abs_min, abs_max, step_size, label_text, parent)
    CDoubleAdvSliderWidget(const double min, const double max, const double value, const int num_decimal,
                           const double abs_min = DBL_MIN, const double abs_max = DBL_MAX,
                           const double step_size = 1, const QString label_text = QString(), QWidget *parent = 0);
    ~CDoubleAdvSliderWidget();

    void Init(const double min, const double max, const double value, const int num_decimal, 
              const double abs_min = DBL_MIN, const double abs_max = DBL_MAX, const double step_size = 1,
              const QString label_text = QString());
    double Value();
    double Min();
    double Max();
    void SetEnabled(const bool enabled);
    bool ReadOnly();
    void EnableFreqBuffer(const bool enable, const size_t freq = 10, const size_t max_buf_size = 5,
                          const bool with_fifo_checking = false);
    void EnableDebugPrint(const bool enable);
    void EmitOnSliderRelease(const bool enable);

  private:
    QDoubleSpinBox *max_spin_box_, *min_spin_box_, *value_spin_box_;
    QSlider *slider_;
    QHBoxLayout *layout_;
    QFrame *line_[3];
    QLabel *label_;
    double min_, max_, value_, abs_min_, abs_max_;
    bool is_init_, flag_, freq_buffer_is_init_, emit_on_slider_release_;
    QString label_text_;
    int num_decimal_;
    double single_step_;
    static double pow_of_ten[13];
    static double pow_of_ten_inv[13];
    void Init();
    mio::CFreqBuffer<double> freq_buffer_;
    static void FreqBufCallBack(double value, void *user_data);

  private slots:
    void SetValueSpinBox(const int value);
    void SetSliderValue(const double value);
    void SetMinimumAdv(const double min);
    void SetMaximumAdv(const double max);
    void SetDecimals(int num_decimal);
    void SetSingleStep(const double single_step);
    void ValueSpinBoxValueChanged(double value);
    void SliderReleased();

  public slots:
    void SetValue(const double value);
    void SetMinimum(const double min);
    void SetMaximum(const double max);
    void SetReadOnly(const bool val_read_only, const bool min_read_only = false, const bool max_read_only = false);
    void PrintValue(const double value){ printf("val %.6f\n", value); }
    void PrintAll(const double value);

  signals:
    void ValueChanged(double value);
};

typedef CAdvSliderWidget AdvSlider;
typedef CDoubleAdvSliderWidget DblAdvSlider;

#endif //__ADV_SLIDER_WIDGET__

