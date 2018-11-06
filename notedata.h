#ifndef NOTEDATA_H
#define NOTEDATA_H
#include<QString>
#include <QDataStream>

struct notedata
{
    QString date;//yyyy-M-d
    QString time;//h-m-s
    QString note;
    QString mp3_cmd;
    QString img;
    int playtime;
    int per;
    int type;//date=0 , Year=1 , Mon=2 ,Week=3 , Day=4
    int id;//全部列表时，-1代表整天，新建日程时，-2代表新建的。每次都不同，可以不保存
};

Q_DECLARE_METATYPE(notedata)



 inline QDataStream& operator<<(QDataStream& out, const notedata& item)
 {
     out<<item.date<<item.time<<item.note<<item.mp3_cmd<<item.img<<item.per<<item.type<<item.playtime<<item.id;
     return out;
 }

 inline QDataStream& operator>>(QDataStream& in,notedata& item)
 {
    in>>item.date>>item.time>>item.note>>item.mp3_cmd>>item.img>>item.per>>item.type>>item.playtime>>item.id;
    return in;
 }

#endif // NOTEDATA_H
