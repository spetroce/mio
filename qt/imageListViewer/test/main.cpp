#include "imageListViewer.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QApplication::setStyle(QStyleFactory::create("Fusion"));
  ImageListViewer w;
  if(argc == 3){
      std::vector<std::string> img_file_name_vec;
      mio::GetDirList(argv[1], std::vector<std::string>(), argv[2], img_file_name_vec);
      if(img_file_name_vec.size() > 0)
        w.SetImageList("/home/spetroce/code/src/testImages", img_file_name_vec);
  }
  w.show();

  return a.exec();
}

