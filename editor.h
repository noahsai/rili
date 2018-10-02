#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <notedata.h>
#include<QDebug>
#include<QFileDialog>
#include<QStandardPaths>
//#include<QMimeDatabase>


namespace Ui {
class editor;
}
//这个对象必须传入一个已指向实例的指针。
class editor : public QWidget
{
    Q_OBJECT

public:
    explicit editor(QWidget *parent = 0);
    ~editor();
    void setdata(notedata*);//会直接操作指针指向的实例

signals:
    void edited(notedata*);

private slots:
    void on_ok_clicked();

    void on_cancel_clicked();

    void on_findfile_clicked();

    void on_type_currentIndexChanged(int index);

    void on_img_clicked();

private:
    Ui::editor *ui;
    notedata* data;

};

#endif // EDITOR_H
