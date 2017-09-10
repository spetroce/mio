#include "imageListViewer.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QApplication::setStyle(QStyleFactory::create("Fusion"));
  mio::ImageListViewer w(true);
  w.adv_img_disp_->SetNormalizeRoi(true);

  const std::string keys = "{help h usage ? | | img_list_viewer_test -d=/path/to/images -e=tiff}"
                           "{d directory | null | directory containing images}"
                           "{e extension | tiff | image file extension (eg. jpeg, tiff, bmp)}";
  cv::CommandLineParser parser(argc, argv, keys);
  if(parser.has("help")){
    parser.printMessage();
    return 0;
  }
  std::string img_dir = parser.get<std::string>("directory");
  std::cout << "directory: " << img_dir << std::endl;
  std::string img_ext = parser.get<std::string>("extension");
  std::cout << "extension: " << img_ext << std::endl;

  if(img_dir != "null"){
      std::vector<std::string> img_file_name_vec;
      mio::GetDirList(img_dir, std::vector<std::string>(), img_ext, img_file_name_vec);
      if(img_file_name_vec.size() > 0)
        w.SetImageList(img_dir, img_file_name_vec);
  }
  w.show();

  return a.exec();
}

