#include "notify.h"
#include "ui_notify.h"
Notify::Notify(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Notify)
{
    ui->setupUi(this);
    ui->note->setText("");
    pressing = false;
    readset();
    setAttribute(Qt::WA_DeleteOnClose ,true );//关闭时销毁
    setAttribute(Qt::WA_TranslucentBackground ,false );//背景透明
    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    oldpos.setX(0);
    oldpos.setY(0);
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
    player = new QMediaPlayer(this);
    connect(player , SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(repeat(QMediaPlayer::MediaStatus)));
    timer->setSingleShot(true);
}

Notify::~Notify()
{
    delete ui;
}

void Notify::init(int t, QString &m, QString &i)
{
    time = t*1000;
    icon =i;
    music = m;
    if(i.isEmpty()) {
        setAttribute(Qt::WA_TranslucentBackground ,false );//背景透明
        ui->icon->hide();
    }
    else{
        setAttribute(Qt::WA_TranslucentBackground ,true );//背景透明
        ui->icon->show();
    }
    player->setVolume(100);
    qDebug()<<"music_img:"<<music<<icon;
}

void  Notify::message(QString& mes)
{
    qDebug()<<"get a message"<<mes;
    QString m = ui->note->text();
    if(m.isEmpty()) m = mes;
    else m += "\n"+ mes;
    ui->note->setText(m);
    this->adjustSize();
    if(music.isEmpty()) {
        player->setMedia(QUrl("qrc:/wei4.mp3"));
    }
    else    player->setMedia(QUrl::fromLocalFile(music));
    player->play();
    timer->start( time );
   // if(ui->icon->isVisible()){
        QPixmap pix(icon);
        pix = pix.scaled(ui->icon->width(),ui->icon->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        ui->icon->setPixmap(pix);
        qDebug()<<pix.width()<<pix.height()<<icon;
   // }
}

void Notify::timeout()
{
    close();
    emit closeed();

    //可能需要一个信号
}


void Notify::repeat(QMediaPlayer::MediaStatus s)
{
    if (s == QMediaPlayer::EndOfMedia) {

        player->play();
    }
}

//=====================================================================
void Notify::mousePressEvent(QMouseEvent* event)
{
    if(event->button()==Qt::LeftButton)
    {
        oldpos=event->globalPos()-this->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void Notify::mouseMoveEvent(QMouseEvent * event){
    if(event->buttons()==Qt::LeftButton)
    {
        pressing = true;
        move(event->globalPos()-oldpos);//貌似linux要这样
        event->accept();
    }
}

void Notify::mouseReleaseEvent(QMouseEvent * event){
    if(event->button()==Qt::LeftButton )
    {
        if(!pressing){
            close();
            setCursor(Qt::ArrowCursor);
            event->accept();
        }
        else {
            int x=this->x();
            int y=this->y();
    //        qDebug()<<x<<endl<<y;

            if(this->pos().x()<0) x=0;
            else if(QApplication::desktop()->width()-x<this->width()){
                x=QApplication::desktop()->width()-this->width();
            }

            if(this->pos().y()<0) y=0;
            else if(QApplication::desktop()->height()-y<this->height())
            {
                y=QApplication::desktop()->height()-this->height();
            }
    //        qDebug()<<x<<endl<<y;
            move(x,y);
            saveset();
            pressing = false;
        }
    }
}


void Notify::saveset(){
    QSettings settings("ShengSoft", "Notify");
   // settings.setValue("size", QSize(370, 150));//因为没得调大小，所以不要记录大小了
    settings.setValue("pos", pos());
}


void Notify::readset(){
    QSettings settings("ShengSoft", "Notify");
//    resize(settings.value("size", QSize(370, 150)).toSize());//因为没得调大小，所以不要记录大小了
    int x=QApplication::desktop()->width()-420;
    QPoint point;
    point=settings.value("pos", QPoint(x, 50)).toPoint();
    if(point.x()<0||point.x()>QApplication::desktop()->width()-20) point.setX(x);
    if(point.y()<0||point.y()>QApplication::desktop()->height()+this->height()-30) point.setY(50);
    move(point);
}

void Notify::stop(){
    timer->stop();
    emit closeed();
}
