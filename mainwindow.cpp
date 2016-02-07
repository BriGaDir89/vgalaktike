#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect( manager,   SIGNAL( finished(QNetworkReply*)),
             this,      SLOT(   replyFinished(QNetworkReply*)));

    QNetworkRequest request;
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    request.setUrl(QUrl("https://api.v-galaktike.ru/api/"));
    //request.setRawHeader("(Request-Line)", "POST /api/ HTTP/1.1");
    request.setRawHeader("Host", "api.v-galaktike.ru");
    request.setRawHeader("X-Requested-With", "ShockwaveFlash/20.0.0.286");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36");
    request.setRawHeader("Content-Type", "application/x-amf");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4");

    qDebug() << "rawHeader: " << request.rawHeaderList();

    QString fileName = QFileDialog::getOpenFileName( this,
                                                     tr("Open API"),
                                                     QApplication::applicationDirPath(),
                                                     tr("API Files (*.html *.txt *.amf3)"));
    QFile File( fileName );
    qDebug() << fileName;
    File.open( QIODevice::ReadOnly );

    qDebug() << "manager->get: " << manager->post( request, File.readAll() );

    //qDebug() <<  "ByteArray: "<<  File.readAll();

    //AMF amf;
    //qDebug() << "AMF, version: " << amf.Decode( File.readAll() );
/*
    QTextStream Stream( &File );
    ui->textEdit->setText( Stream.readAll() );
*/
    File.close();
}
void MainWindow::replyFinished( QNetworkReply* tmp )
{
     //qDebug() << "QNetworkReply: " << tmp->readAll();
     AMF amf;
     qDebug() << "AMF, version: " << amf.Decode( tmp->readAll() );
}
