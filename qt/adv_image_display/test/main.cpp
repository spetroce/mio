#include "mio/qt/adv_image_display/adv_image_display.h"
#include <QApplication>


int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    mio::AdvImageDisplay w;
    w.Init(0);
    w.SetDrawClicks(true);
    cv::Mat img = cv::imread(argv[1]);
    w.show();
    w.SetImage(img, true);

    return a.exec();
}

