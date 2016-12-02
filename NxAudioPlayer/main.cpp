#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("eucKR"));
	w.setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
	w.show();
    return a.exec();
}
