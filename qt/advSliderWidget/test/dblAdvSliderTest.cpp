#include "advSliderWidget.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  // (min, max, value, num_decimal, abs_min, abs_max, step_size, label_text, parent)
  DblAdvSlider w(-0.001, 0.001, 0.0005, 4, -10, 10, 0.001, "my dbl val");
  w.EnableFreqBuffer(true);
  w.EnableDebugPrint(true);

  w.show();
  return a.exec();
}

