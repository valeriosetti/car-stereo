//custombutton.cpp 
#include "custombutton.h"
#include <iostream>
 
custombutton::custombutton(QString idle_img, QString pressed_img, QObject *parent):
    QPushButton((QPushButton*)parent)
{
    QPixmap pixmap(idle_img);
    QSize imagesize = pixmap.size();
    this->setFixedSize(imagesize);
    this->setMask(pixmap.mask());
    this->setStyleSheet("QPushButton{border-image: url(\"" + idle_img.toLocal8Bit() + "\");}\n"
                        "QPushButton:pressed{border-image: url(\"" + pressed_img.toLocal8Bit() + "\");}");

} 
