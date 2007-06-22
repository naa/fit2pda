#include <QApplication>
#include <image_processor.h>
#include <iostream>

#include <QApplication>

    #include "mainwindow.h"

    int main(int argc, char *argv[])
    {
        Q_INIT_RESOURCE(application);

        QApplication app(argc, argv);
        MainWindow mainWin;
        mainWin.show();
        return app.exec();
    }
//  int main(int argc, char * argv[]) {
//	if (argc<3) {
//		std::cout << "usage: qtsplitter <width> <filenames>";
//		return 1;
//	}
//	QApplication app(argc,argv);
//	char *wdstring=argv[1];
//	int wdth;
//	sscanf(wdstring,"%d",&wdth);
//	for (int i=2;i<argc;i++) {
//		char *fname=argv[i];
//		processImage(wdth,fname);
//	}
//	return 0;
//}
//
