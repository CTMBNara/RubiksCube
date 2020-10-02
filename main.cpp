#include "rubikscube.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RubiksCube w;
    w.show();

    return a.exec();
}
