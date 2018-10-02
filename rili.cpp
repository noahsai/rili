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
    ui->widget_5->hide();
    ui->listWidget->setSpacing(1);
    nowid = 0;
    noteeditor = NULL;
    notify = NULL;
    QDir().mkpath("/tmp/rili");
    cfgpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/rili";
    QDir().mkpath(cfgpath);

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
    ui->tableWidget->installEventFilter(this);

    QIcon icon = QIcon(":/img/rili.png");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayiconactive(QSystemTrayIcon::ActivationReason)));
    creattrayicon();
    trayIcon->show();

    wheel = new QTimer(this);
    wheel->setInterval(300);
    wheel->setSingleShot(true);
    connect(wheel , SIGNAL(timeout()) , this ,SLOT(wheelstop()));

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(900);
    connect(timer , SIGNAL(timeout()) ,this , SLOT(timeout()));
    readlist();
    initrili();
}

rili::~rili()
{
    savelist();
    for(int i=0;i<notelist.count();i++)
    {
        delete notelist.at(i);
    }
    delete ui;
}

void rili::makedatelist(QString date){
    ui->tableWidget->clearSelection();

    QDate d;
    d = d.fromString(date,"yyyy-MM-dd");
    //如果日期出错，设为该月最后一日
    if(d.isNull()) {
        date = date.replace(date.length()-2,2,"01");
        d = d.fromString(date,"yyyy-MM-dd");
        int days = d.daysTo(d.addMonths(1));
        d.setDate(d.year(),d.month(),days);
        date = d.toString("yyyy-MM-dd");
        //qDebug()<<"date error" <<d.daysTo(d.addMonths(1))<<d<<date;
    }
    ui->year->setCurrentIndex(d.year()-1900);//同步设置年
    ui->mon->setCurrentIndex(d.month()-1);//同步设置月

    this_month = d.month();//获取当前月份
    currday ="%1";
    currday = currday.arg( QString().setNum( d.day()) , 2 ,'0');
    //qDebug()<<currday;
    d.setDate(d.year(),d.month(),1);//设为该月1号
    int w = d.dayOfWeek();
    d = d.addDays(0-(w - 1));
    startday =d;
    QString str;
    //"xxxx-xx-xx,干支年,干支月,干支日，农月，农日,节日,放假(0正常，1节假,2补班)，生肖,节气,日程"
    //     0       1     2     3     4    5   6   7                     8   9  10
    //根据日期生成整个月的日期列表，包含前后两个月的头尾，共35天，7×5
    //初始化js引擎
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
    //==========================================
     QTableWidgetItem *item;
    for(int i =0 ; i < 7*6 ; i++)
    {
        QString date2 = d.toString("yyyy-MM-dd");
        str = myEngine.evaluate("lunar('" + date2 + "')").toString();
        //qDebug()<<str;
        item = ui->tableWidget->item(i/7,i%7);
        item->setSizeHint(QSize(48,33));
        item->setData(Qt::UserRole,QVariant(str));
        if(date2 == date ) {
            item->setSelected(true);
        }
        //qDebug()<< "result:"<<str<<d;
        d = d.addDays(1);
    }
    updatenotesignal();

    wheel->start();
}

void rili::wheelstop()
{
    QString date,mon,year;
    year = QString().setNum(ui->year->currentIndex()+1900);
    mon = QString("%1").arg(QString().setNum(ui->mon->currentIndex()+1),2,'0') ;
    date = year +"-"+mon +"-"+currday;
    //makedatelist(date);
    getholiday(date);//数据获取后会执行setbartext

}

void rili::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    QString data;
    data = item->data(Qt::UserRole).toString();

    QStringList list;
    list = data.split(",");
    if(list.length() < 7) return;//确保数据无误再读取,总共是9个，此处最多用到第八个
    QString date = list.at(0);//日期，用于计算星期和判断今天
    QDate d;
    d = QDate().fromString(date,"yyyy-MM-dd");
    bool istomonth = true;
    istomonth = (QString(date.split("-").takeAt(1)).toInt() == this_month) ;
    if(!istomonth) {
        makedatelist(date);
        return;
    }
    setbartext(data);
}

void rili::setbartext(QString &data){
    ui->yishi->clear();
    ui->jishi->clear();
    ui->yishi2->clear();
    ui->jishi->clear();
    QString  day, nlday;
    QStringList list;
    list = data.split(",");
    if(list.length() < 7) return;//确保数据无误再读取
    //===============
    QString date = list.at(0);//日期，用于计算星期和判断今天

    notesofdate(date);//刷新当前日程

    QDate d;
    d = QDate().fromString(date,"yyyy-MM-dd");
    int w = d.dayOfWeek();//星期几

    //日期
    day = date.split("-").takeLast();
    currday = day;//这个日期已经包含补0
    if(day.at(0)=='0') day.remove('0');
    //农历
    nlday = list.at(5);

    //"xxxx-xx-xx,干支年,干支月,干支日，农月，农日,节日,放假(0正常，1节假,2补班),生肖,term,日程"
    //      0       1     2    3     4   5   6   7                     8   9   10
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
    if(QString().setNum(date.year()) == ui->year->currentText() && (ui->mon->currentIndex()+1) == date.month())
    {
        int i = date.day();
        date.setDate(date.year(),date.month(),1);//设为该月1号
        int w = date.dayOfWeek();
        i = i + w;
        int r = i/7;
        int c = i%7;
        ui->tableWidget->item(r,c)->setCheckState(Qt::Checked);
        //qDebug()<<"zaici";
    }
    else {
        QString cmd =date.toString("yyyy-MM-dd");
        ui->year->setCurrentIndex(date.year()-1900);
        ui->mon->setCurrentIndex(date.month()-1);
        makedatelist(cmd);
    }
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

void rili::gotdata(QString& data){
    QString clean;
    setbartext(clean);//清空bar
    QRegularExpression reg;
    reg.setPattern("(?<=almanac\":).+?]");
    QString yiji = reg.match(data).captured(0);
    //qDebug()<<data;
    QJsonObject ob;
    QString time,yi,ji;
    QJsonArray arr;

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson( yiji.toUtf8() , &json_error );
    if(json_error.error != QJsonParseError::NoError) {
        qDebug()<<"json error"<<json_error.errorString();
        return;
    }
    else {
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
        //qDebug()<<"data:"<<data;
    }
    QString holiday;
    reg.setPattern("(?<=festival\":).+?(?=])");
    QRegularExpressionMatchIterator matchs = reg.globalMatch(data);
    QRegularExpressionMatch item ;
    if(matchs.hasNext()) {
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
            d = d.fromString(date,"yyyy-M-d");//这个格式没错
            int n = startday .daysTo( d);
            if(n>=0) {
                int c = n%7;
                int r = n/7%6;
                item = ui->tableWidget->item(r,c);
                QString data = item->data(Qt::UserRole).toString();
                QStringList list = data.split(",");
                list.replace( 7 ,statu);//7是调休
                data = list.join(",");//
                //qDebug()<<n<<c<<r<<data;;
                item->setData(Qt::UserRole,data);
            }
        }
     }
    else  qDebug()<<"data not found holiday";
    //获取后更新侧栏数据
    QString str =  ui->tableWidget->selectedItems().at(0)->data(Qt::UserRole).toString();
    //qDebug()<<"str:"<<str;
    setbartext(str);//获取假期后再setbartext，这样宜忌就有数据了

}

void rili::getholiday(QString& date){
   QString url="https://sp0.baidu.com/8aQDcjqpAAV3otqbppnN2DJv/api.php?query="+date+"&resource_id=6018";
   QIODevice *io =  NULL;//初始化
   if(diskCache ) io = diskCache->data(QUrl(url));
   //qDebug()<<diskCache<<url;
   if( io )
   {
       QTextCodec *codec = QTextCodec::codecForName("GBK");
       QByteArray data = io->readAll();
       QString text = codec->toUnicode( data);
       qDebug()<<"has cache";
       gotdata(text);

   }
   else {
       QNetworkRequest request;
       request.setRawHeader(QByteArray("User-Agent"), "Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.99 Safari/537.36");
       request.setRawHeader(QByteArray("Referer"), "https://www.baidu.com/");
       request.setUrl(QUrl(url) );
       reply = manager.get(request);
       connect(reply , SIGNAL(finished()),this, SLOT(replyfinished()));
       qDebug()<<"to get net data";

   }
}

bool rili::eventFilter(QObject *object, QEvent *event){
    if(ui->yishi2->text().isEmpty()&&ui->jishi2->text().isEmpty()) return false;
    if( (object == ui->widget_4)){
        if( event->type() == QEvent::Enter)
        {
           // qDebug()<<"enter";
            ui->widget_3->adjustSize();
            ui->widget_3->move(this->width()-ui->widget_3->width()-130-3 , this->height()-ui->widget_3->height()-1);
            ui->widget_3->show();
            event->accept();//必须
            return true;
        }
        else if(event->type() == QEvent::Leave){
            ui->widget_3->hide();
            event->accept();//必须
            return true;
        }
    }
    else if( (object == ui->tableWidget) ){
        if(event->type() == QEvent::Wheel){
            QWheelEvent *e=static_cast<QWheelEvent*>(event);
            int degrees=e->delta();//上为正，下为负
            if(degrees>0)
            {
               // qDebug()<<"wheel up";
                on_premon_clicked();
                event->accept();//必须
            }
            else{
                //qDebug()<<"wheel down";
                on_nextmon_clicked();
                event->accept();//必须
            }
            return true;
        }
    }
    else   return QWidget::eventFilter(object , event );//还需要原目标处理
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

void rili::on_richeng_clicked()
{
    ui->widget->hide();
    ui->widget_5->show();
}

void rili::on_huangli_clicked()
{
    ui->widget_5->hide();
    ui->widget->show();

}

bool rili::readlist()
{
    tasklist.clear();
    QFile file;
    QString path = cfgpath+"/notelist";
    qDebug()<<"readlist"<<path;
    file.setFileName(path);
    QDataStream in(&file);
    if(file.open(QIODevice::ReadOnly))
    {
        for(;!in.atEnd();)
        {
            notedata* item = new notedata;//notelist存的是指针！！所以需要不同的指针！！item自身是变化的！！
            in>>*item;
            qDebug()<<item->date<<item->time<<item->note<<item->type<<item->per<<item->mp3_cmd<<item->img;
            item->id = nowid;
            notelist.append(item);
            ++nowid;
        }
        qDebug()<<notelist;
        file.close();
        updatetasklist();//更新tasklist
        return true;
    }
    else qDebug()<<"open notelist error.";
    return false;

//    notedata *item = new notedata;
//    item->date="2018-10-29";
//    item->type=1;
//    item->note="呵呵哈哈哈哈";
//    item->time="09:26";
//    notelist.append(item);
}

bool rili::savelist()
{
    QFile file;
    QString path = cfgpath+"/notelist";
    //qDebug()<<"savelist"<<path;
    file.setFileName(path);
    if(file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        //qDebug()<<layerList.count();
        for(int i=0;i<notelist.count();i++)
        {
            out<<(*(notelist[i]));
        }
        return true;
    }
    else qDebug()<<"open notelist error.";
     return false;
}

void rili::updatenotesignal()
{
    QTableWidgetItem *item;
   item = ui->tableWidget->item(0,0);
   QDate start , end;
   QString day = item->data(Qt::UserRole).toString().split(',').at(0);
   start = start.fromString(day , "yyyy-MM-dd");
   end = start.addDays(41);
   QList<QDate> tmplist;

   qDebug()<<"start"<<start<<"end"<<end;

   for(int j = 0 ; j<notelist.count() ; j++)
   {
        notedata *data = notelist[j];
        QString date = data->date;
        QDate d,tmp;
        d = d.fromString(date , "yyyy-MM-dd");
        int m;
        switch(data->type)
        {
        case 0:
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }
            break;
        case 1:
            d.setDate(start.year() , d.month() , d.day());
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }
            //=======================
            d.setDate(start.addDays(15).year() , d.month() , d.day());
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }            d.setDate(end.year() , d.month() , d.day());
            //=======================
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }            qDebug()<<tmplist;
            break;
        case 2:
            d.setDate(start.year() , start.month() , d.day());
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }
            //======================
            d.setDate(start.addDays(15).year() , start.addDays(15).month() , d.day());
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }
            //======================

            d.setDate(end.year() , end.month() , d.day());
            m = start.daysTo((d));
            if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
            {
                if(!tmplist.contains(d))    tmplist.append(d);
            }
            break;
        case 3:
            tmp = start;
            //tmp从头到尾走一遍，与固定的d对比
            for(int q =0 ;q<42 ;)
            {
                if(d.daysTo(tmp)%data->per == 0)   {
                    if(!tmplist.contains(tmp))    tmplist.append(tmp);
                    tmp = tmp.addDays(data->per);//找到就不在逐一对比，直接步进改为频率
                    q+=data->per;
                }
                else {
                    tmp = tmp.addDays(1);
                    q++;
                }
            }
            break;
        }

    }
   QDate dd;
   for(int i=0;i<tmplist.count();i++)
   {
       dd = tmplist.at(i);
       int n = start.daysTo((dd));
       //这里不用判断dd在不在改页范围了，因为生产列表时已经判断过了
       //qDebug()<<dd;
       item = ui->tableWidget->item(n/7 , n%7);
       day = item->data(Qt::UserRole).toString();
       QStringList list = day.split(",");
       list.replace( 10 ,"1");
       day = list.join(",");
       item->setData(Qt::UserRole , day);
   }

  //qDebug()<< "result:"<<str<<d;
}

void rili:: cleannotesignal(){
   QTableWidgetItem *item;
   for(int n=0;n<42;n++)
   {
       item = ui->tableWidget->item(n/7 , n%7);
       QString day = item->data(Qt::UserRole).toString();
       QStringList list = day.split(",");
       if(list.count()<10) return;
       list.replace( 10 ,"0");
       day = list.join(",");
       item->setData(Qt::UserRole , day);
   }
}


void rili::on_allnote_clicked()
{
    ui->listWidget->clear();
    ui->listWidget->setCursor(QCursor(Qt::PointingHandCursor));
    QListWidgetItem* item;
    notedata* tmp;
    QString text;
    QStringList tmplist;//用于排除重复日期
    for(int i=0;i<notelist.count();i++)
    {
        tmp = notelist[i];
        switch(notelist[i]->type)
        {
        case 0:
            text = notelist[i]->date;
            break;
        case 1:
            text = notelist[i]->date;
            text.remove(0,5);
            text = "每年"+text+" "+notelist[i]->time+"\n"+notelist[i]->note;
            tmp = notelist[i];
            break;
        case 2:
            text = notelist[i]->date;
            text.remove(0,8);
            text = "每月"+text+"日 "+notelist[i]->time+"\n"+notelist[i]->note;
            tmp = notelist[i];
            break;
        case 3:
            text = notelist[i]->date+"起每"+QString().setNum(notelist[i]->per)+"日 "+notelist[i]->time+" \n"+notelist[i]->note;
            tmp = notelist[i];
            break;
        default:break;
        }

        if(!(tmp->type==0 && tmplist.contains(tmp->date))){
            item = new QListWidgetItem(ui->listWidget);
            if(tmp->type == 3) item->setSizeHint(QSize(60,60));
            else item->setSizeHint(QSize(60,40));
            item->setText(text);
            item->setToolTip(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackgroundColor(QColor("#eeeeee"));
            if(tmp->type ==0 ) item->setData(Qt::UserRole,-1);//全部列表时，-1代表整天
            else item->setData(Qt::UserRole,tmp->id);
            ui->listWidget->addItem(item);
            tmplist.append(tmp->date);
        }
    }
    ui->listWidget->sortItems(Qt::DescendingOrder);
    ui->allnote->hide();
}

void rili::notesofdate(QString &date){
    ui->listWidget->setCursor(QCursor(Qt::ArrowCursor));
    ui->listWidget->clear();
    QListWidgetItem* item;
    QDate d;
    notedata* tmp;
    QString text;
    QDate day;
    day = day.fromString(date , "yyyy-MM-dd");
    for(int i=0;i<notelist.count();i++)
    {

        d = d.fromString(notelist[i]->date , "yyyy-MM-dd");
        tmp = NULL ;
        switch(notelist[i]->type)
        {
        case 0:
            if(day == d) {
                tmp = notelist[i];
                text = notelist[i]->time+"\n"+notelist[i]->note;
            }
            break;
        case 1:
            if(day.month()==d.month()&&day.day()==d.day()){
                text = notelist[i]->date;
                text.remove(0,5);
                text = "每年"+text+" "+notelist[i]->time+"\n"+notelist[i]->note;
                tmp = notelist[i];
            }
            break;
        case 2:
            if(day.day()==d.day()){
                text = notelist[i]->date;
                text.remove(0,8);
                text = "每月"+text+"日 "+notelist[i]->time+"\n"+notelist[i]->note;
                tmp = notelist[i];
            }
            break;
        case 3:
            if(day.daysTo(d)%notelist[i]->per==0)  {
                text = notelist[i]->date+"起每"+QString().setNum(notelist[i]->per)+"日 "+notelist[i]->time+"\n"+notelist[i]->note;
                tmp = notelist[i];
            }
            break;
        default:break;
        }
        if(tmp != NULL){
            item = new QListWidgetItem(ui->listWidget);
            if(tmp->type == 3) item->setSizeHint(QSize(60,60));
            else item->setSizeHint(QSize(60,40));
            item->setText(text);
            item->setToolTip(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackgroundColor(QColor("#FFEC8B"));
            item->setData(Qt::UserRole,tmp->id);
            ui->listWidget->addItem(item);

        }
    }
    ui->listWidget->sortItems(Qt::DescendingOrder);
    ui->allnote->show();
}


void rili::updatetasklist(){
    tasklist.clear();
    for(int i=0;i<notelist.count();i++)
    {
        //out<<(*(notelist[i]));
        QDate d;
        QDate today;
        d = d.fromString(notelist[i]->date , "yyyy-MM-dd");
        today = today.currentDate();
        switch(notelist[i]->type)
        {
        case 0:
            if(today == d) tasklist.append(notelist[i]);
            break;
        case 1:
            if(today.month()==d.month()&&today.day()==d.day())  tasklist.append(notelist[i]);
            break;
        case 2:
            if(today.day()==d.day())  tasklist.append(notelist[i]);
            break;
        case 3:
            if(today.daysTo(d)%notelist[i]->per==0)  tasklist.append(notelist[i]);
            break;
        }
    }
    qDebug()<<"tasklist:"<<tasklist;
    if(tasklist.count()>0) timer->start();
    else timer->stop();
}

void rili::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if(ui->allnote->isHidden()){
        if(item->text().indexOf("每")!=-1) return;
        QTableWidgetItem *item2;
        item2 = ui->tableWidget->item(0,0);
        QDate start , end , d;
        QString day = item2->data(Qt::UserRole).toString().split(',').at(0);
        start = start.fromString(day , "yyyy-MM-dd");
        end = start.addDays(41);
        d = d.fromString(item->text(),"yyyy-MM-dd");
        int m;
        m = start.daysTo(d);
       // qDebug()<<"item.text"<<item->text()<<m;
        if( m >= 0 && end.daysTo(d)<=0)  //这里判断d在不在改页范围
        {
           item2 = ui->tableWidget->item(m/7 , m%7);
           on_tableWidget_itemClicked(item2);
        }
        else {
            QString date = item->text();
            makedatelist(date);
        }
    }
    else{
        //日程编辑模式
        if(noteeditor==NULL) {
            noteeditor = new editor(this);
            connect(noteeditor , SIGNAL(edited(notedata*)),this , SLOT(editfined(notedata*)));
        }
        int id = item->data(Qt::UserRole).toInt();
        qDebug()<<"setdata"<<id<<notelist;
        for(int i = 0;i<notelist.length();i++)
        {
            if(id == notelist[i]->id) {
                noteeditor->setdata(notelist[i]);
                noteeditor->show();
                break;
            }
        }
    }
}

void rili::on_delnote_clicked()
{
    int result;
    if(ui->allnote->isHidden())    result = QMessageBox::question(this,"删除日期的日程？","是否删除所选日期的日程？","否","是");
    else   result = QMessageBox::question(this,"删除日程？","是否删除所选日程？","否","是");
    if (result!=0) {
        int id = -2;
        QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
        for(int i = 0;i<list.count();i++)
        {
            id = list[i]->data(Qt::UserRole).toInt();
            qDebug()<<"delete"<<id<<notelist;
            if(id == -1){
                //全部列表的某天，需遍历删除某天的所有日程
                for(int j = 0;j<notelist.count();j++)
                {
                    if(notelist[j]->date == list[i]->text() && (notelist[j]->type==0)) {
                        notelist.removeAt(j);
                        --j;//因为实时改动list会导致list长度也改变。
                    }
                }
            }
            else{
                //证明是每xxx，可直接删除它
                for(int j = 0;j<notelist.count();j++)
                {
                    if(notelist[j]->id == id) {
                        notelist.removeAt(j);
                        --j;//因为实时改动list会导致list长度也改变。
                    }
                }
            }
            ui->listWidget->removeItemWidget(list[i]);
            delete list[i];
        }
        updatetasklist();
        cleannotesignal();
        updatenotesignal();
    }
}

void rili::on_addnote_clicked()
{
    if(noteeditor==NULL) {
        noteeditor = new editor(this);
        connect(noteeditor , SIGNAL(edited(notedata*)),this , SLOT(editfined(notedata*)));
    }
    notedata* data = new notedata;
    data->date = ui->tableWidget->selectedItems().at(0)->data(Qt::UserRole).toString().left(10);
    data->time = QTime().currentTime().toString("hh:mm");
    data->playtime = 10;
    data->per = 0;
    data->type = 0;
    data->id = -2;//标记为新建的。
    noteeditor->setdata(data);
    noteeditor->show();
}

void rili::editfined(notedata *d ){
    if(!d) return;
    if(d->id == -2) {
        notelist.append(d);
        d->id = nowid;
        ++nowid;
    }
    updatetasklist();
    cleannotesignal();
    updatenotesignal();
    on_tableWidget_itemClicked(ui->tableWidget->selectedItems().at(0));
    on_richeng_clicked();
}

void rili::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    on_addnote_clicked();
}

void rili::timeout(){
    QTime now;
    now = now.currentTime();
    int playtime=0;
    QString mp3 , img ,text;
    for(int i= 0;i<tasklist.length();i++)
    {
        if(now.toString("hh:mm:ss")==(tasklist[i]->time+":00")){
            if(notify == NULL) {
                notify = new Notify(this);
            }
            qDebug()<<"work:"<<tasklist[i];
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForFile(tasklist[i]->mp3_cmd);  //
            QString type = mime.name();
            //qDebug()<<type;
            //不匹配时返回application/octet-stream
            if(tasklist[i]->mp3_cmd.isEmpty() ){
                //什么都不用干,依然使用就的mp3
            }
            else if(type.indexOf("audio")!=-1){
                mp3 = tasklist[i]->mp3_cmd;
            }
            else if(type == "application/octet-stream"){
                QProcess pro;
                pro.startDetached(tasklist[i]->mp3_cmd);
                qDebug()<<"run cmd:"<<tasklist[i]->mp3_cmd;
            }
            else if(type == "application/x-sharedlib"||type == "application/x-shellscript"){
                QProcess pro;
                pro.startDetached(("bash \""+tasklist[i]->mp3_cmd +"\""));
                qDebug()<<"run shellscript sharedlib:"<<tasklist[i]->mp3_cmd;
            }
            else {
                QProcess pro;
                pro.startDetached(("xdg-open  \""+tasklist[i]->mp3_cmd+"\""));
                qDebug()<<"run xdg-open"<<tasklist[i]->mp3_cmd ;
            }
            if(!tasklist[i]->img.isEmpty() ){
                img = tasklist[i]->img;
            }
            if(playtime < tasklist[i]->playtime) playtime = tasklist[i]->playtime;

            if(!text.isEmpty()) text += "\n";
            text = text  + tasklist[i]->note ;
           //不要去除notelist里面的！！这只是闹铃
            tasklist.removeAt(i);
            --i;//因为实时改动list会导致list长度也改变。
            if(tasklist.count()<1) timer->stop();
            //不要updatetasklist了。直接删除tasklist里面的就行了
        }
    }
    if(playtime!=0) {
        notify->init(playtime , mp3 ,img);
        notify->message(text);//这时会启动notify计时
        notify->show();
    }
}
