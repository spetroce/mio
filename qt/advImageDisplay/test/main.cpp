#include "mio/qt/advImageDisplay/advImageDisplay.h"
#include <QApplication>


int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    AdvImageDisplay w;
    w.Init(0);
    w.SetDrawClicks(true);
    cv::Mat img = cv::imread(argv[1]);
    w.show();
    w.SetImage(img, true);

    return a.exec();
}

