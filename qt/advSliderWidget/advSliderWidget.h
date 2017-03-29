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
    int value();
    int min();
    int max();
    void setEnabled(bool enabled);
    int singleStep();
    bool readOnly();
    void setSliderTracking(bool enabled);

  private:
    QSpinBox *max_spin_box_, *min_spin_box_, *value_spin_box_;
    QSlider *slider_;
    QHBoxLayout *layout_;
    QFrame *line_[3];
    QLabel *label_;
    int min_, max_, value_, abs_min_, abs_max_, step_size_;
    bool is_init_, flag_;
    QString label_text_;
    void Init();
    void CreateInterface();

  private slots:
    void SetValueSpinBox(const int value);
    void SetSliderValue(const int value);
    void SetMinimum(const int min);
    void SetMaximum(const int max);
    void PrintValue(const int value){ printf("val %d\n", value); }

  public slots:
    void setValue(const int value);
    void setMinimum(const int min);
    void setMaximum(const int max);
    void setSingleStep(const int step_size);
    void setReadOnly(const bool val_read_only, const bool min_read_only = false, const bool max_read_only = false);

  signals:
    void valueChanged(const int value);
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
    double value();
    double min();
    double max();
    void setEnabled(const bool enabled);
    void setSliderTracking(bool enabled);

  private:
    QDoubleSpinBox *max_spin_box_, *min_spin_box_, *value_spin_box_;
    QSlider *slider_;
    QHBoxLayout *layout_;
    QFrame *line_[3];
    QLabel *label_;
    double min_, max_, value_, abs_min_, abs_max_;
    bool is_init_, flag_;
    QString label_text_;
    int num_decimal_;
    double single_step_;
    static double pow_of_ten[13];
    static double pow_of_ten_inv[13];
    void Init();

  private slots:
    void SetValueSpinBox(const int value);
    void SetSliderValue(const double value);
    void SetMinimum(const double min);
    void SetMaximum(const double max);
    void setDecimals(int num_decimal);
    void setSingleStep(const double single_step);
    void PrintValue(const double value){ printf("val %.6f\n", value); }
    void PrintAll(const double value);

  public slots:
    void setValue(const double value);
    void setMinimum(const double min);
    void setMaximum(const double max);

  signals:
    void valueChanged(double value);
};

typedef CAdvSliderWidget AdvSlider;
typedef CDoubleAdvSliderWidget DblAdvSlider;

#endif //__ADV_SLIDER_WIDGET__

