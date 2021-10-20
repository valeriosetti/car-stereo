#include <QtWidgets>
#include "radio.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Radio radio;
    radio.show();

    return app.exec();
};
