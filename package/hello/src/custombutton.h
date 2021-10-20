//custombutton.h
#ifndef CUSTOMBUTTON_H 
#define CUSTOMBUTTON_H



#include <QObject>
#include <QPushButton>
#include <QPixmap>
#include <QBitmap>
 
class custombutton: public QPushButton

{
    Q_OBJECT
public:

    explicit custombutton(QString idle_img, QString pressed_img, QObject *parent = 0);

private:

};
 

#endif // CUSTOMBUTTON_H
