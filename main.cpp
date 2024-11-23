#include "widget.h"

#include <QApplication>


#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    w.show();
    return a.exec();
}
