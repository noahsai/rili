#include "editor.h"
#include "ui_editor.h"

editor::editor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::editor)
{
    ui->setupUi(this);
    setWindowTitle("日程");
    ui->type->setCurrentIndex(-3);//确保初始setdata时能触发槽函数
    data = NULL;
    setWindowFlags(Qt::CustomizeWindowHint|Qt::Window|Qt::WindowTitleHint);
}

editor::~editor()
{
    //此处千万别delete data，否则data指向的实例会被销毁。
    delete ui;
}

void editor::setdata(notedata* d){
    ui->per->setValue(0);
    ui->img->setToolTip("");
    ui->img->setIcon(QIcon());
    ui->img->setText("图片");
    data = d;
    //会直接操作指针指向的实例
    ui->date->setText(d->date);
    qDebug()<<d->note<<d->time<<d->id<<d->mp3_cmd<<d->img<<d->type<<d->per<<d->playtime;
    QString hour="12";
    QString min="00";
    if(d->time.indexOf(":")!=-1)
    {
        hour = d->time.split(":").at(0);
        min = d->time.split(":").at(1);
    }
    ui->hour->setValue(hour.toInt());
    ui->min->setValue(min.toInt());
    ui->note->setText(d->note);
    ui->mp3_cmd->setText(d->mp3_cmd);
    ui->type->setCurrentIndex(d->type);//输入数字与本来不同时会触发槽函数，相同不会触发
    ui->per->setValue(d->per);
    ui->playtime->setValue(d->playtime);
    if(!d->img.isEmpty()) {
        ui->img->setToolTip(d->img);
        ui->img->setIcon(QIcon(d->img));
        ui->img->setText("");
    }
}

void editor::on_ok_clicked()
{
    data->time = QString("%1:%2").arg(ui->hour->value(),2,10,QLatin1Char('0')).arg(ui->min->value(),2,10,QLatin1Char('0'));
    //补0,必须QLatin1Char
    data->note = ui->note->text();
    data->mp3_cmd = ui->mp3_cmd->text();
    data->type = ui->type->currentIndex();
    data->per = ui->per->value();
    data->playtime = ui->playtime->value();
    data->img = ui->img->toolTip();
    emit edited(data);
    //data = NULL;
    //qDebug()<<data->img;
    hide();
}

void editor::on_cancel_clicked()
{
    if(data->id==-2) delete data;//-2为新建的
    //data = NULL;
    hide();
}

void editor::on_findfile_clicked()
{
    QString home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QString file =  QFileDialog::getOpenFileName(this,"选择文件",home,tr("所有 (*)"));
    if(file.isEmpty()) return;
    else ui->mp3_cmd->setText(file);
//    QMimeDatabase db;
//    QMimeType mime = db.mimeTypeForFile(file);  //根据前面定义的文件名（含后缀）
//    QString type = mime.name();
//    qDebug()<<type;
}

void editor::on_type_currentIndexChanged(int index)
{
    qDebug()<<index;
    if(index == 3) ui->per->show();
    else ui->per->hide();
}

void editor::on_img_clicked()
{
    QString home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QString file =  QFileDialog::getOpenFileName(this,"选择文件",home,tr("图片 (*.png *.jpeg *.jpg)"));
    if(file.isEmpty()) return;
    else {
        ui->img->setText("");
        ui->img->setIcon(QIcon(file));
        ui->img->setToolTip(file);
    }
}
