#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("ClientOrders");
    QApplication::setOrganizationName("ClientOrders");
    MainWindow window;
    window.show();
    return app.exec();
}
