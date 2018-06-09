#include "rili.h"
#include "ui_rili.h"

extern int this_month = 0;

QString change(int in)
{
    QString out ;
    out = out.setNum(in);
    out = out.replace("1","一");
    out = out.replace("2","二");
    out = out.replace("3","三");
    out = out.replace("4","四");
    out = out.replace("5","五");
    out = out.replace("6","六");
    out = out.replace("7","日");
    return out;
}


rili::rili(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rili)
{
    ui->setupUi(this);
    QDir().mkpath("/tmp/rili");
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint);

    currday ="01";
    LayerItemDelegate *delegate = new LayerItemDelegate;

    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setRowCount(6);
    QStringList head;
    head<<"一"<<"二"<<"三"<<"四"<<"五"<<"六"<<"日";
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setHorizontalHeaderLabels(head);
    ui->tableWidget->setItemDelegate(delegate);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(55);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(45);
    ui->tableWidget->verticalHeader()->setHidden(true);
    ui->tableWidget->horizontalHeader()->setHidden(false);
    ui->tableWidget->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget->horizontalHeader()->setAutoScroll(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->setCursor(QCursor(Qt::PointingHandCursor));
    ui->tableWidget->setFrameShape(QFrame::NoFrame);
   // ui->tableWidget->setStyleSheet(" \  ");
    ui->widget->setStyleSheet(" #widget{background-color:#5af;}\
                                #day{background-color:#fb0;\
                                color:white;\
                                font-size:52px;border-radius:3px;}\
                                #line,QLabel{color:white;font-size:13px;}\
                                #yi,#ji{font-size:24px;}  ");
    ui->yishi->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    ui->jishi->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    ui->day->setFixedSize(75,75);
    ui->tableWidget->setFixedSize(385,295);
    ui->widget_2->setFixedWidth(385+4+4);
    ui->widget->setFixedWidth(130);
    this->setFixedHeight(335);
    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("/tmp/rili");
    diskCache->setMaximumCacheSize(10*1024*1024);//10mb
    manager.setCache(diskCache);

    ui->yishi2->setWordWrap(true);
    ui->jishi2->setWordWrap(true);
    ui->widget_3->setFixedWidth(200);
    ui->widget_3->setParent(this);
    ui->widget_3->hide();

    ui->widget_4->installEventFilter(this);

    QIcon icon = QIcon(":/img/rili.png");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayiconactive(QSystemTrayIcon::ActivationReason)));
    creattrayicon();
    trayIcon->show();

    initrili();
}

rili::~rili()
{
    delete ui;
}

void rili::makedatelist(QString date){
    ui->tableWidget->clearSelection();

    //根据日期生成整个月的日期列表，包含前后两个月的头尾，共35天，7×5
    QJSEngine myEngine;
    QString fileName = ":/js/lunar5.js";
     QFile scriptFile(fileName);
     if (!scriptFile.open(QIODevice::ReadOnly)){
         qDebug()<<"open js error"<<scriptFile.errorString();
         return;
     }
     QTextStream stream(&scriptFile);
     QString contents = stream.readAll();
     scriptFile.close();
     myEngine.evaluate(contents, fileName);

     QDate d;
    d = d.fromString(date,"yyyy-MM-dd");
    this_month = d.month();//获取当前月份
    currday ="%1";
    currday = currday.arg( QString().setNum( d.day()) , 2 ,'0');
    //qDebug()<<this_month;
    d.setDate(d.year(),d.month(),1);//设为该月1号
    int w = d.dayOfWeek();
    d = d.addDays(0-(w - 1));
    startday =d;
    QString str;
    //"xxxx-xx-xx,干支年,干支月,干支日，农月，农日,节日,放假(0正常，1节假,2补班)，生肖,节气"
    QTableWidgetItem *item;
    for(int i =0 ; i < 7*6 ; i++)
    {
        QString date2 = d.toString("yyyy-MM-dd");
        str = myEngine.evaluate("lunar('" + date2 + "')").toString();
        //qDebug()<<str;
        item = ui->tableWidget->item(i/7%6,i%7);
        item->setSizeHint(QSize(48,33));
        item->setData(Qt::UserRole,QVariant(str));
        if(date2 == date ) {
            item->setSelected(true);
            setbartext(str);
        }
       // qDebug()<< "result:"<<str;
        d = d.addDays(1);
    }
    getholiday(date);//获取假期等,最好放这里，房前面貌似使用缓存文件时有问题。

//qDebug()<<this->size();

}


void rili::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    QString data, day, nlday,festival ,holiday;
    data = item->data(Qt::UserRole).toString();
    setbartext(data);

}

void rili::setbartext(QString &data){
    ui->yishi->clear();
    ui->jishi->clear();
    ui->yishi2->clear();
    ui->jishi->clear();
    QString  day, nlday,festival ,holiday;
    QStringList list;
    list = data.split(",");
    if(list.length() < 7) return;//确保数据无误再读取,总共是9个，此处最多用到第八个
    QString date = list.at(0);//日期，用于计算星期和判断今天
    QDate d;
    d = QDate().fromString(date,"yyyy-MM-dd");
    int w = d.dayOfWeek();//星期几
    bool istomonth = true;
    istomonth = (QString(date.split("-").takeAt(1)).toInt() == this_month) ;
    if(!istomonth) {
        makedatelist(date);
        return;
    }
    //日期
    day = date.split("-").takeLast();
    currday = day;//这个日期已经包含补0
    if(day.at(0)=='0') day.remove('0');
    //农历
    nlday = list.at(5);
    //节日
    festival = list.at(6);
    //放假吗？
    holiday = list.at(7);//空未否，非空为是
    //"xxxx-xx-xx,干支年,干支月,干支日，农月，农日,节日,放假(0正常，1节假,2补班),生肖"
    //      0       1     2     3     4    5    6   7                      8
    ui->date->setText(date+" 星期"+change(w));
    ui->day->setText(day);
    ui->nongli->setText(list.at(4)+"月"+nlday);
    ui->tiangan->setText(list.at(1)+"年"+"【"+list.at(8)+"年】");
    ui->dizhi->setText(list.at(2)+"月 "+list.at(3)+"日");


    date = d.toString("yyyy-M-d");
    if(map.contains(date))
    {
        QString yiji = map[date];
        yiji.replace(".","、");
        list = yiji.split("|");
        if(list.length() !=2) return;
        QString yi = list.at(0);
        QString ji = list.at(1);
        yi.remove(QRegularExpression("、$"));
        ji.remove(QRegularExpression("、$"));
        ui->yishi2->setText(yi);
        ui->jishi2->setText(ji);

        QRegularExpression reg("(.+?、){5}");
        QString result = reg.match(yi).captured(0);
        if(!result.isEmpty()) yi = result;

        yi.replace("、","\n");
        ui->yishi->setText(yi);
        result = reg.match(ji).captured(0);
        //qDebug()<<result;
        if(!result.isEmpty()) ji = result;

        ji.replace("、","\n");
        ui->jishi->setText(ji);
    }
    ui->widget_3->adjustSize();

}

void rili::initrili()
{
    freshicon();
    setWindowTitle("度良日历");
    int i=1900;
    for(i=1900;i<=2050;i++)
    {
        ui->year->addItem(QString().setNum(i) );
    }
    for(i=1;i<=12;i++)
    {
        ui->mon->addItem(QString().setNum(i)+"月");
    }
    QTableWidgetItem *item;
    for(int i =0 ; i < 7*6 ; i++)
    {
        item = new QTableWidgetItem;
        ui->tableWidget->setItem(i/7%6,i%7,item);
    }
    on_today_clicked();
}

void rili::on_year_activated(const QString &arg1)
{
    QString date,mon;
    mon = QString("%1").arg(QString().setNum(ui->mon->currentIndex()+1),2,'0') ;
    date = arg1 +"-"+mon +"-"+currday;
    makedatelist(date);

}

void rili::on_premon_clicked()
{
    int curr = ui->mon->currentIndex();
    curr--;
    if(curr<0) {
        int n = ui->year->currentIndex();
        n--;
        if(n<0) n=0;
        ui->year->setCurrentIndex(n);
        curr=11;
    }
    ui->mon->setCurrentIndex(curr);
    on_mon_activated(curr);
    //qDebug()<<curr;
}

void rili::on_nextmon_clicked()
{
    int curr = ui->mon->currentIndex();
    curr++;
    if(curr>11) {
        int n = ui->year->currentIndex();
        n++;
        if(n >= ui->year->count()) n=ui->year->count();
        ui->year->setCurrentIndex(n);
        curr=0;
    }
    ui->mon->setCurrentIndex(curr);
    //qDebug()<<curr;
    on_mon_activated(curr);

}


void rili::on_mon_activated(int index)
{
    QString date,year,mon;
    year = QString().setNum(ui->year->currentIndex()+1900);
    mon = QString("%1").arg(QString().setNum(index+1),2,'0');
    date =  year + "-" + mon + "-" + currday;;
    //qDebug()<<date;
    makedatelist(date);
}

void rili::on_today_clicked()
{
    QDate date;
    date = date.currentDate();
    QString cmd =date.toString("yyyy-MM-dd");
    ui->year->setCurrentIndex(date.year()-1900);
    ui->mon->setCurrentIndex(date.month()-1);
    makedatelist(cmd);

}

void rili::mousePressEvent(QMouseEvent* event)
{
    if(event->button()==Qt::LeftButton )
    {
        oldpos=event->globalPos()-this->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void rili::mouseMoveEvent(QMouseEvent * event){
    move(event->globalPos()-oldpos);//貌似linux要这样
    event->accept();
}

void rili::mouseReleaseEvent(QMouseEvent * event){
    setCursor(Qt::ArrowCursor);
   event->accept();
}

void rili::on_exit_clicked()
{
    hide();
}

void rili::replyfinished(){
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QByteArray data = reply->readAll();
    reply->deleteLater();
    QString text = codec->toUnicode( data);
    //qDebug()<<text;
    gotdata(text);
}

void rili::gotdata(QString data){
    QRegularExpression reg;
    reg.setPattern("(?<=almanac\":).+?]");
    QString yiji = reg.match(data).captured(0);
    //qDebug()<<yiji;
    QJsonObject ob;
    QString time,yi,ji;
    QJsonArray arr;

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson( yiji.toUtf8() , &json_error );
    if(json_error.error != QJsonParseError::NoError) {
        qDebug()<<"json error"<<json_error.errorString();
        return;
    }
    arr = document.array();

    for(int i=0;i<arr.count();i++)
    {
        ob = arr.at(i).toObject();
        time = ob.value("date").toString();
        yi = ob.value("suit").toString();
        ji = ob.value("avoid").toString();
        map[time]=yi+"|"+ji;
    }
    //qDebug()<<map;

    QString holiday;
    reg.setPattern("(?<=festival\":).+?(?=])");
    QRegularExpressionMatchIterator matchs = reg.globalMatch(data);
    QRegularExpressionMatch item ;
    if(!matchs.hasNext()) return;//没匹配就结束
    while(matchs.hasNext())
    {
        item = matchs.next();
        holiday = holiday + item.captured();
    }

    reg.setPattern("\\d{4}-\\d{1,2}-\\d{1,2}[^\\d]+?\\d(?=\")");
    matchs = reg.globalMatch(holiday);
    while(matchs.hasNext())
    {
        item = matchs.next();
        QString date,statu;
        QStringList text;
        date = item.captured(0);
        text = date.split(QRegularExpression("\".+\""));
        if(text.length()<2)  return;
        date = text.at(0);
        statu = text.at(1);
        holidaymap[date] = statu;

        QDate d;
        QTableWidgetItem *item;
        d = d.fromString(date,"yyyy-M-d");
        int n = startday .daysTo( d);
        if(n>=0) {
            int c = n%7;
            int r = n/7%6;
            item = ui->tableWidget->item(r,c);
            QString data = item->data(Qt::UserRole).toString();
            data = data.replace(",0,",","+statu+",");
            //qDebug()<<n<<c<<r<<data;;
            item->setData(Qt::UserRole,data);
        }
    }
    //    qDebug()<<"holiday";

}

void rili::getholiday(QString& date){
   QString url="https://sp0.baidu.com/8aQDcjqpAAV3otqbppnN2DJv/api.php?query="+date+"&resource_id=6018";
   QIODevice *io =  NULL;//初始化
   if(diskCache ) io = diskCache->data(QUrl(url));
   //qDebug()<<diskCache<<io;
   if( io )
   {
       QTextCodec *codec = QTextCodec::codecForName("GBK");
       QByteArray data = io->readAll();
       QString text = codec->toUnicode( data);
      // qDebug()<<text;
       gotdata(text);
   }
   else {
       QNetworkRequest request;
       request.setRawHeader(QByteArray("User-Agent"), "Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.99 Safari/537.36");
       request.setRawHeader(QByteArray("Referer"), "https://www.baidu.com/");
       request.setUrl(QUrl(url) );
       reply = manager.get(request);
       connect(reply , SIGNAL(finished()),this, SLOT(replyfinished()));
   }
}

bool rili::eventFilter(QObject *object, QEvent *event){
    if(ui->yishi2->text().isEmpty()&&ui->jishi2->text().isEmpty()) return false;
    if( (object == ui->widget_4)&& event->type() == QEvent::Enter){
       // qDebug()<<"enter";
        ui->widget_3->adjustSize();
        ui->widget_3->move(this->width()-ui->widget_3->width()-130-3 , this->height()-ui->widget_3->height()-1);
        ui->widget_3->show();
    }
    else if( (object == ui->widget_4)&& event->type() == QEvent::Leave){
       // qDebug()<<"leave";
        ui->widget_3->hide();
    }
    return false;//还需要原目标处理
    //event处理后返回true表示已经处理该事件，返回false则qt会将event发给原目标
}

void rili::creattrayicon()
{
//    QAction *change=new QAction(tr("切换城市"),this);
//    connect(change,SIGNAL(triggered()),ui->city,SLOT(click()));
//    QAction *set=new QAction(tr("设置"),this);
//    connect(set,SIGNAL(triggered()),this,SLOT(callsetting()));

    QAction* quitAction = new QAction(tr("退出"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));//若触发了退出就退出程序
    trayIconMenu = new QMenu(this);//菜单
//    trayIconMenu->addAction(change);
//    trayIconMenu->addAction(set);

    trayIconMenu->addAction(quitAction);//把退出加到入菜单项
    trayIcon->setContextMenu(trayIconMenu);//设置托盘上下文菜单为trayIconMenu

}


void rili::trayiconactive(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Context ||reason == QSystemTrayIcon::Unknown ) return;
    else {
        if(isActiveWindow()) this->hide();
        else {
        showNormal();
        activateWindow();
        }
    }
}


void rili::closeEvent(QCloseEvent* event){
    hide();
    event->ignore();
}

void rili::freshicon(){
    QString today ;
    today = today.setNum(QDate::currentDate().day());
    QPixmap p(":/img/rili0.png");
    QPainter painter(&p);
    QRect rect = p.rect();
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setColor(QColor("red"));
    pen.setBrush(QColor("red"));
    painter.setPen(pen);
    QFont font;
    font.setPixelSize(p.rect().height()/5*3);
    //font.setBold(true);
    painter.setFont(font);
    QTextOption textOption;
    textOption.setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    QRectF rectf (0,rect.y()+rect.height()/5,rect.width(),rect.height()-rect.height()/5);
    painter.drawText(rectf,today,textOption);
    setWindowIcon(QIcon(p));
    trayIcon->setIcon(QIcon(p));
}
