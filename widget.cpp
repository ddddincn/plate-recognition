#include "widget.h"
#include "./ui_widget.h"
#include <fstream>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    player_ = new QMediaPlayer(this);
    video_sink_ = new QVideoSink(this);

    QVideoWidget *video_widget = new QVideoWidget(ui->video_player);

    QWidget *video_player_widget = ui->video_player;
    QVBoxLayout *video_player_widget_layout = new QVBoxLayout;
    video_player_widget_layout->addWidget(video_widget);
    video_player_widget->setLayout(video_player_widget_layout);



    frame_label_ = new QLabel(ui->video_frame);
    frame_label_->setStyleSheet("background-color: black;");
    frame_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QWidget *video_frame_widget = ui->video_frame;
    QVBoxLayout *video_frame_widget_layout = new QVBoxLayout;
    video_frame_widget_layout->addWidget(frame_label_);
    video_frame_widget_layout->setAlignment(Qt::AlignCenter);
    video_frame_widget->setLayout(video_frame_widget_layout);

    player_->setVideoOutput(video_widget);

    connect(video_widget->videoSink(), &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (!frame.isValid())
            return;
        frame_monitor_ = frame.toImage();
    });
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_choose_video_clicked(){
    qDebug()<<"on_choose_video_clicked";
    ui->label_1->setText("");
    ui->label_2->setText("");
    QString fileName = QFileDialog::getOpenFileName(nullptr, "choose video", "", "file (*.mp4 *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        player_->setSource(QUrl::fromLocalFile(fileName));
        player_->play();
    }
}

void Widget::on_capture_clicked(){
    qDebug()<<"on_capture_clicked";
    ui->label_1->setText("");
    ui->label_2->setText("");
    frame_ = frame_monitor_;
    if (frame_.isNull()) {
        qDebug() << "No frame available for capture.";
        return;
    }

    frame_label_->setPixmap(QPixmap::fromImage(frame_).scaled(frame_label_->size(), Qt::KeepAspectRatio));
    frame_label_->show();

}


const static std::string client_id = "Fo9BRW6hYcEiREFtLtvU8Z2o";
const static std::string client_secret = "T4HlHijR6HoLGW0w2JRdlsW7PHNIcJk9";

inline size_t onWriteData(void * buffer, size_t size, size_t nmemb, void * userp)
{
    std::string * str = dynamic_cast<std::string *>((std::string *)userp);
    str->append((char *)buffer, size * nmemb);
    return nmemb;
}

std::string getAccessToken()
{
    std::string result;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "https://aip.baidubce.com/oauth/2.0/token");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string data = "grant_type=client_credentials&client_id="+client_id+"&client_secret="+client_secret;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(result.data(), result.data() + result.size(), &obj, &error);
    return obj["access_token"].asString();
}

std::string convertQImageToBase64(const QImage &image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");

    return byteArray.toBase64().toStdString();
}

std::string toURLEncoded(const std::string &input) {
    std::ostringstream encoded;
    for (unsigned char c : input) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else if (c == ' ') {
            encoded << '+';
        } else {
            encoded << '%' << std::uppercase << std::hex << int(c);
        }
    }
    return encoded.str();
}

int request(const QImage &image,std::string &result){
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, ("https://aip.baidubce.com/rest/2.0/ocr/v1/license_plate?access_token=" + getAccessToken()).c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "undefined");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string data_str = "image=" + toURLEncoded(convertQImageToBase64(image))+ "&multi_detect=false&multi_scale=false&detect_complete=false&detect_risk=false";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return (int)res;
}

void Widget::on_analysis_clicked(){
    qDebug()<<"on_analysis_clicked";
    ui->label_1->setText("");
    ui->label_2->setText("");

    if(frame_.isNull()){
        ui->label_1->setText("no frame to analysis");
        return;
    }

    std::string result;
    if(request(frame_,result)==0){
        Json::Value obj;
        std::string error;
        Json::CharReaderBuilder crbuilder;
        std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
        reader->parse(result.data(), result.data() + result.size(), &obj, &error);

        const Json::Value &error_code = obj["error_code"];
        bool successed = error_code.isNull();

        if(!successed){
            std::string error_code = obj["error_code"].asString();
            std::string error_msg = obj["error_msg"].asString();

            QString qerror_code = "error_code: " + QString::fromStdString(error_code);
            QString qerror_msg = "error_msg: " + QString::fromStdString(error_msg);
            ui->label_1->setText(qerror_code);
            ui->label_2->setText(qerror_msg);

        }else{
            std::string number = obj["words_result"]["number"].asString();
            std::string color = obj["words_result"]["color"].asString();

            QString qnumber = "number: " + QString::fromStdString(number);
            QString qcolor = "color: " + QString::fromStdString(color);
            ui->label_1->setText(qnumber);
            ui->label_2->setText(qcolor);

        }
    }
}
