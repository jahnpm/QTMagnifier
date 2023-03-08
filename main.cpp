#include "qtmagnifier.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTMagnifier qtMagnifier(nullptr, &app);
    qtMagnifier.setWindowFlags(Qt::FramelessWindowHint);

    qtMagnifier.show();

    return app.exec();
}
