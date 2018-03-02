#include "adv_slider_widget.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  // (min, max, value, abs_min, abs_max, step_size, label_text, parent)
  AdvSlider w(-100, 20, 15, -1000, 1000000000, 1, "my val");
  if(argc == 2){
    if(std::string(argv[1]) == "freq-buffer")
      w.EnableFreqBuffer(true);
    else if(std::string(argv[1]) == "release-emit")
      w.EmitOnSliderRelease(true);
  }
  w.EnableDebugPrint(true);

  w.show();
  return a.exec();
}

