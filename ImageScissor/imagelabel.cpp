#include "imagelabel.h"
#include <QMessageBox>

imagelabel::imagelabel(QWidget *parent):QLabel(parent){
    this->setMouseTracking(true);

}

imagelabel::~imagelabel(){

}

void imagelabel::mouseMoveEvent(QMouseEvent *ev){
    //move event position
    QPoint mouse_pos = ev->pos();
    if(mouse_pos.x()<=this->size().width() && mouse_pos.y()<=this->size().height()){
        //track moving location
        my_x = mouse_pos.x();
        my_y = mouse_pos.y();
    }

}

void imagelabel::mousePressEvent(QMouseEvent *ev){
    //if(ev->button()==Qt){

    //}
}
