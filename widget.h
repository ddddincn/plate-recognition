#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoWidget>
#include <QVideoFrame>
#include <QImage>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <QBuffer>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_choose_video_clicked();
    void on_capture_clicked();
    void on_analysis_clicked();

private:
    Ui::Widget *ui;

    QMediaPlayer *player_;
    QVideoSink *video_sink_;
    QLabel *frame_label_;
    QImage frame_monitor_;
    QImage frame_;
};
#endif // WIDGET_H
