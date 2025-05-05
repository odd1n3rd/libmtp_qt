#include "mainwindow.h"
#include <QApplication>
#include "../libmtpviewmodel/stubmtpdevice.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    StubMtpDevice stubDevice;
    MainWindow window(&stubDevice);
    window.show();
    return app.exec();
}
