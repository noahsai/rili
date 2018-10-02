#include "rili.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    rili w;
    //Notify w;
    w.show();

    return a.exec();
}
