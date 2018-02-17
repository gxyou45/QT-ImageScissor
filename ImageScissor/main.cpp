#include "imagescissor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageScissor w;
    w.show();

    return a.exec();
}
