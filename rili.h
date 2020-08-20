#ifndef RILI_H
#define RILI_H

#include <QWidget>
#include<layeritemdelegate.h>
#include<QTextStream>
#include<QFile>
#include<QJSEngine>
#include<QTableWidgetItem>
#include <QMouseEvent>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonValue>
#include<QJsonDocument>
#include<QJsonParseError>
#include<QNetworkAccessManager>
#include<QNetworkRequest>
#include<QNetworkReply>
#include<QRegularExpression>
#include<QNetworkDiskCache>
#include<QDir>
#include<QIODevice>
#include<QTextCodec>
#include<QSettings>
#include<QDesktopWidget>
#include<QSystemTrayIcon>
#include<QMenu>
#include<QTimer>
#include<notedata.h>
#include<QStandardPaths>
#include<QListWidgetItem>
#include<QMessageBox>
#include<editor.h>
#include<notify.h>
#include<QMimeDatabase>
#include<QProcess>
namespace Ui {
class rili;
}

class rili : public QWidget
{
    Q_OBJECT

public:
    explicit rili(QWidget *parent = 0);
    ~rili();

private slots:

    void on_tableWidget_itemClicked(QTableWidgetItem *item);

    void on_year_activated(const QString &arg1);

    void on_premon_clicked();

    void on_nextmon_clicked();

    void on_mon_activated(int index);

    void on_today_clicked();

    void on_exit_clicked();

    void gotholiday();
    void gotmonth();

    void trayiconactive(QSystemTrayIcon::ActivationReason reason);

    void wheelstop();

    void on_richeng_clicked();

    void on_huangli_clicked();

    void on_allnote_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_delnote_clicked();

    void on_addnote_clicked();

    void editfined(notedata*);
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
    void timeout();

private:
    Ui::rili *ui;
    QStringList datelist;//"xxxx-xx-xx,干支年,干支月,干支日，农月，农日,节日,放假(0正常，1节假,2补班),生肖，节气"
                         //      0       1     2     3     4    5    6   7                      8   9
    bool eventFilter(QObject *, QEvent *);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void makedatelist(QString);
    void initrili();
    void setbartext(QString&);
    void getholiday(QString&);
    void getmonth(QString&);

    void gotholidaydata(QString&);
    void gotmonthdata(QString&);
    void creattrayicon();
    void freshicon();
    bool readlist();
    bool savelist();
    void cleannotesignal();
    void updatenotesignal();
    QString notesofdate(QString &);
    void updatetasklist();
    void saveset();
    void readset();
    QString currday;
    QNetworkAccessManager manager,manager2;
    QNetworkReply* reply;
    QNetworkDiskCache *diskCache;
    QPoint oldpos;
    QMap<QString,QString> map,holidaymap;
    QDate startday;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QTimer *wheel , *timer;
    QList<notedata*> notelist;
    QList< notedata* > tasklist;

    QString cfgpath;
    int nowid;

    editor* noteeditor;
    Notify *notify;
};

#endif // RILI_H
