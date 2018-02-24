#include "imagelabel.h"
#include <QMessageBox>

imagelabel::imagelabel(QWidget *parent):QLabel(parent){
    this->setMouseTracking(true);

    scaleFactor = 1;
    scribbling = false;
    myPenColor = Qt::red;
    myPenWidth = 2;
}

imagelabel::~imagelabel(){

}

bool imagelabel::openImage(const QString &filename)
{
    //opencv open file example
    //cv::Mat input = cv::imread(filename.toLatin1().data());
    //cv::imshow("Display Image", input);
    QImage Loaded_Image(filename);
    if(Loaded_Image.isNull()){
        QMessageBox::warning(this,"..","Failed to load image.");
        return false;
    }

    Paint_Image = Loaded_Image.copy();
    qpixmap = QPixmap::fromImage(Loaded_Image);
    this->setPixmap(qpixmap);
    update();
    return true;

}

//SCALING:ZOOM_IN/ZOOM_OUT
void imagelabel::scaleImage(double scale_fac)
{
    scaleFactor = scaleFactor * scale_fac;

    this->QLabel::resize(scaleFactor * (this->pixmap()->size()));
    this->setVisible(true);
    //QSize newSize = QSize(Paint_Image.width() * scale_fac, Paint_Image.height() * scale_fac);
    /*
    QPixmap qpixmap = QPixmap::fromImage(Paint_Image);
    ui->label->setPixmap(qpixmap);
    ui->label->resize(scaleFactor * (ui->label->pixmap()->size()));
    my_label->setVisible(true);
    */
    //Paint_Image = Paint_Image.scaled(newSize, Qt::IgnoreAspectRatio);
    update();
}

void imagelabel::updateScaleFactor(double f)
{
    scaleFactor = f;
}


//DRAW LINES
void imagelabel::setPenColor(const QColor &newColor)
{
    myPenColor = newColor;
}

void imagelabel::setPenWidth(int newWidth){
    myPenWidth = newWidth;
}

void imagelabel::drawLineTo_example(const QPoint &endPoint)
{
    QPainter painter(&Paint_Image);
    //lastPoint = seeds.last();
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);
    //modified = true;

    int rad = (myPenWidth / 2) + 2;
    update(QRect(lastPoint, endPoint).normalized()
                                     .adjusted(-rad, -rad, +rad, +rad));
    lastPoint = endPoint;

}

void imagelabel::drawLineTo(const QPoint &endPoint, std::vector<Node*> &nodes, int width)
{

    QPainter painter(&Paint_Image);
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    QPoint tmp = endPoint;
    int idx = endPoint.x() * width + endPoint.y();
    while(nodes[idx]->prevNode!=NULL){
        Node *next = nodes[idx]->prevNode;
        int new_x = next->column;
        int new_y = next->row;
        tmp.setX(new_x);
        tmp.setY(new_y);
        //draw_list.append(tmp);
        idx = new_y * width + new_x;
        painter.drawPoint(tmp);
    }

}

//DRAW DEBUG GRAPH
void imagelabel::drawCostGraph(){

}

void imagelabel::drawPixelNode(){
    int org_width = Paint_Image.width();
    int org_height = Paint_Image.height();
    int width = org_width * 3 + 1;
    int height = org_height * 3 + 1;

    //QSize newSize(width, height);
    QImage *pixelGraph = new QImage(width, height, QImage::Format_Indexed8);
    pixelGraph->fill(Qt::white);
    for(int x = 0; x < org_width; x++){
        for(int y = 0; y < org_height; y++){
            //QPoint b(x*3, y*3);
            pixelGraph->setPixel(x*3+1, y*3+1, Paint_Image.pixel(x, y));
        }
    }

    qpixmap = QPixmap::fromImage(*pixelGraph);
    this->setPixmap(qpixmap);

    update();
    //cv::imshow()
    //return true;
}

void imagelabel::drawPathTree(){

}

void imagelabel::drawMinPath(){

}

//MOUSE EVENTS
void imagelabel::mousePressEvent(QMouseEvent *event)
{
    /* EXAMPLE:
    if(event->button() == Qt::LeftButton){
        lastPoint = event->pos();
        scribbling = true;
        if(lastPoint.x() >= 0 && lastPoint.x() <= this->size().width()){
            if(lastPoint.y()>=0 && lastPoint.y() <= this->size().height()){
                //current_x;
            }
        }
    }
    */
    if(event->button() == Qt::LeftButton){
        QPoint seed_pos = event->pos();

        seed_x = seed_pos.x();
        seed_y = seed_pos.y();
        if(seeds.isEmpty()){
            seeds.append(seed_pos);
        }
        else{
            drawLineTo_example(seed_pos);
            seeds.append(seed_pos);
        }
        //seeds.append(seed_pos);
        //drawLineTo();
    }
}

void imagelabel::mouseMoveEvent(QMouseEvent *event)
{
    /* EXAMPLE:
     * if((event->buttons() & Qt::LeftButton) && scribbling){
        drawLineTo(event->pos());
    }
    */
    QPoint curr_pos = event->pos();
    my_x = curr_pos.x();
    my_y = curr_pos.y();
    //extend_wire(my_x, my_y);
}

void imagelabel::mouseReleaseEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::LeftButton) && scribbling){
        drawLineTo_example(event->pos());
        scribbling = false;
    }
}

void imagelabel::paintEvent(QPaintEvent *event)
{
   // QLabel::paintEvent(ev);
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, Paint_Image, dirtyRect);
}


//void imagelabel::getWirePoints(int x, int y)
//{

//}

//OPENCV FUNC
QImage imagelabel::mat_to_qimage_ref(cv::Mat &mat, QImage::Format format)
{
  return QImage(mat.data, mat.cols, mat.rows, mat.step, format);
}

cv::Mat imagelabel::qimage_to_mat_ref(QImage &img, int format)
{
    return cv::Mat(img.height(), img.width(),
            format, img.bits(), img.bytesPerLine());
}
