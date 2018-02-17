#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QMouseEvent>

class imagelabel:public QLabel{
    Q_OBJECT
public:
    int my_x;
    int my_y;
    explicit imagelabel(QWidget *parent = 0);
    ~imagelabel();
    QImage Load_Image;
private:
    void mouseMoveEvent(QMouseEvent *ev);

    void mousePressEvent(QMouseEvent *ev);

};

#endif // IMAGELABEL_H
