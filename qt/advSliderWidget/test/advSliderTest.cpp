#include "advSliderWidget.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  // (min, max, value, abs_min, abs_max, step_size, label_text, parent)
  AdvSlider w(-100, 20, 15, -1000, 1000000000, 1, "my val");

  w.show();
  return a.exec();
}

