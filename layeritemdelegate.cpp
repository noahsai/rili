#include "layeritemdelegate.h"

extern int this_month;



LayerItemDelegate::LayerItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

LayerItemDelegate::~LayerItemDelegate()
{

}



void LayerItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{

        QString data, day, nlday,festival ,holiday,jieqi,hasnote;
        QStringList list;
        QRect rect = option.rect;
        QRect textRect;;
        QTextOption textOption;
        textOption.setAlignment( Qt::AlignHCenter | Qt::AlignBottom);
        textOption.setWrapMode(QTextOption::NoWrap);
        QFont font;
        QPen pen;
        //pen.setColor(QColor("black"));
        //painter->setPen(pen);

        data = index.model()->data(index, Qt::UserRole).toString();
        //qDebug()<<index.row()<<index.column()<<data;
        list = data.split(",");
        if(list.length() < 9) return;//确保数据无误再读取,总共是10个
        QString date = list.at(0);//日期，用于计算星期和判断今天
        int w = QDate().fromString(date,"yyyy-MM-dd").dayOfWeek();//星期几
        bool istoday =(QDate().fromString(date,"yyyy-MM-dd").daysTo(QDate().currentDate())==0) ? true : false;
        bool istomonth = true;
        istomonth = (QString(date.split("-").takeAt(1)).toInt() == this_month) ;

        //qDebug()<<"istoday:"<<istoday;
        //日期
        day = date.split("-").takeLast();
        if(day.at(0)=='0') day.remove('0');
        //农历
        nlday = list.at(5);
        //节日
        festival = list.at(6);
        //放假吗？
        holiday = list.at(7);
        //节气
        jieqi = list.at(9);

        hasnote = list.at(10);
//        ======================================
//        画背景
        pen.setStyle(Qt::NoPen);
        //painter->setBrush(QBrush(QColor("#fff")));
        QString brushcolor = "white";
        if(istoday)     brushcolor="#FFC125";

        if(holiday == "1" && (!istoday))
        {
            brushcolor = "#fff0f0";
        }
        else if( holiday == "2" &&(!istoday))
        {
            brushcolor = "#f5f5f5";
        }
        painter->setPen(pen);
        painter->setBrush(QColor(brushcolor));
        painter->drawRect(rect);

        if (option.state & QStyle::State_Selected )   {
            //话选中框
            painter->setBrush(QColor("#FFD700"));
            painter->drawRect(rect);
            painter->setBrush(QColor(brushcolor));
            painter->drawRect(rect.x()+3,rect.y()+3,rect.width()-6,rect.height()-6);
        }

        //画底&写"班","休"
        if(holiday == "1" )
        {
            pen.setStyle(Qt::NoPen);
            painter->setBrush(QBrush(QColor("#f43")));
            textRect.setRect(rect.x(), rect.y(), 15, 15);
            painter->setPen(pen);
            painter->drawRect(textRect);
            //---------------
            font.setPixelSize(13);
            painter->setFont(font);
            pen.setStyle(Qt::SolidLine);
            pen.setColor(QColor("white"));
            painter->setPen(pen);
            painter->drawText(textRect,"休",textOption);
        }
        else if( holiday == "2" )
        {
            pen.setStyle(Qt::NoPen);
            painter->setBrush(QBrush(QColor("#969799")));
            textRect.setRect(rect.x(), rect.y(), 15, 15);
            painter->setPen(pen);
            painter->drawRect(textRect);
            //-------------
            font.setPixelSize(13);
            painter->setFont(font);
            pen.setStyle(Qt::SolidLine);
            pen.setColor(QColor("white"));
            painter->setPen(pen);
            painter->drawText(textRect,"班",textOption);
        }

        //写日期
        textOption.setAlignment( Qt::AlignHCenter | Qt::AlignBottom);
        font.setPixelSize(16);
        painter->setFont(font);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor("black"));
        if(w==6||w==7) {
            pen.setColor(QColor("#e02d2d"));
        }
        if(istoday)     pen.setColor(QColor("white"));
       // else if(!istomonth )     pen.setColor(QColor("#bfbfbf"));
        painter->setPen(pen);
        textRect.setRect(rect.x(), rect.y(), rect.width(), rect.height()/2);
        painter->drawText(textRect,day,textOption);
        //qDebug()<<textRect;

        //写节日 或 写农历 , 优先节日,其次节气 ，最后农历
        textOption.setAlignment( Qt::AlignHCenter | Qt::AlignTop);
        font.setPixelSize(12);
        painter->setFont(font);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor("grey"));
        textRect.setRect(rect.x(), rect.y()+rect.height()/2, rect.width(), rect.height()/2);
        if(istoday)     pen.setColor(QColor("white"));//必须
        //else if(!istomonth )     pen.setColor(QColor("#bfbfbf"));

        painter->setPen(pen);//放在setColor后
        if(!festival.isEmpty()||!jieqi.isEmpty()){
            pen.setColor(QColor("#e02d2d"));
            if(istoday)    pen.setColor(QColor("white"));//也必须
           // else if(!istomonth )     pen.setColor(QColor("#bfbfbf"));
            painter->setPen(pen);
            if(festival.length()>4) festival.resize(4);
            if(!festival.isEmpty()) painter->drawText(textRect,festival,textOption);
            else painter->drawText(textRect,jieqi,textOption);
        }
        else    painter->drawText(textRect,nlday,textOption);

        if(hasnote=="1") {
//            QPainterPath path;
//            path.moveTo(rect.x()+rect.width()*1.0/2-8,rect.y()+rect.height());
//            path.lineTo(rect.x()+rect.width()*1.0/2+8,rect.y()+rect.height());
//            path.lineTo(rect.x()+rect.width()*1.0/2,rect.y()+rect.height()-8);
//            painter->setPen(Qt::NoPen);
//            painter->setBrush(QBrush(QColor("#50a0f0")));
//            painter->drawPath(path);
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(2);
            pen.setColor(QColor("#cc50a0ff"));
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(rect.center()+QPoint(0,1),20,20);
        }

        if(!istomonth ){
            //整体添加蒙版
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor("#aaffffff"));
            painter->drawRect(rect);
        }
        //画分隔线
        pen.setWidth(1);//设为1
        painter->setRenderHint(QPainter::Antialiasing,false);//必须关闭反锯齿,保证线条清晰纤细
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor("#c8cacc"));
        painter->setPen(pen);
        painter->drawLine(rect.topLeft(),rect.topRight());
}


bool LayerItemDelegate::editorEvent(QEvent * event,
    QAbstractItemModel * model,
    const QStyleOptionViewItem & option,
    const QModelIndex & index)
{
    Q_UNUSED(event);
    Q_UNUSED(model);
    Q_UNUSED(index);
    Q_UNUSED(option);

    return false;
}

//QWidget *LayerItemDelegate::createEditor(QWidget *parent,
//    const QStyleOptionViewItem &option,
//    const QModelIndex &index) const
//{
////   // //qDebug() << "createEditor";
////    if (index.column() == 1) // value column
////    {
////        editnote* editor = new editnote(parent->parentWidget());
////      //  editor->setFixedHeight(60);
////        //editnote->setContentsMargins(48, 15, 50, 0);
////        return editor;
////    }
////    else return 0;  // no editor attached
//}

//void LayerItemDelegate::setEditorData(QWidget *e, const QModelIndex &index) const
//{
////    LayerItem value = index.model()->data(index, Qt::EditRole).value<LayerItem>();

////    editnote* editor = static_cast<editnote*>(e);
//   // editor->item = value ;
////    editor->setdata(value);
//  //  //qDebug() << "setEditorData";
//}

//void LayerItemDelegate::updateEditorGeometry(QWidget *editor,
//    const QStyleOptionViewItem &option, const QModelIndex & index ) const
//{
//    //editor->setGeometry(option.rect);
////    editor->setGeometry(editor->parentWidget()->rect());
//}

//void LayerItemDelegate::setModelData(QWidget *e, QAbstractItemModel *model,
//    const QModelIndex &index) const
//{
//  //  //qDebug() << "setModelData";
////    editnote* editor = static_cast<editnote*>(e);
////    QVariant data;
////    data.setValue(editor->item) ;
////    model->setData(index, data, Qt::EditRole);

//}
