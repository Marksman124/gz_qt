#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    w.setWindowTitle("沙缸头驱动板工装V1.0");                         // 设置窗口标题
    return a.exec();
}
