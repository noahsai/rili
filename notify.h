#ifndef NOTIFY_H
#define NOTIFY_H

#include <QWidget>
#include<QTimer>
#include<QMediaPlayer>
#include<QDesktopWidget>
#include<QMouseEvent>
#include <QSettings>
#include<QTime>
namespace Ui {
class Notify;
}

class Notify : public QWidget
{
    Q_OBJECT

public:
    explicit Notify(QWidget *parent = 0);
    ~Notify();
    void init(int t, QString& m , QString& i);
    void message(QString& );

private slots:
    void timeout();
    void repeat(QMediaPlayer::MediaStatus);
private:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void saveset();
    void readset();
    void stop();

    Ui::Notify *ui;
    QTimer* timer;

    QPoint oldpos;
    bool pressing;

    int  time;
    QString icon , music;
    QMediaPlayer* player;

};

#endif // NOTIFY_H
