#include "imagescissor.h"
#include "ui_imagescissor.h"

ImageScissor::ImageScissor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ImageScissor)
{
    ui->setupUi(this);
    this->setMouseTracking(true);

    ui->label->setText("");
    ui->label->setBackgroundRole(QPalette::Base);
    ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label->setScaledContents(true);
    ui->label->setMouseTracking(true);

    ui->scrollArea->setBackgroundRole((QPalette::Dark)); 
    //ui->scrollArea->setWidget(ui->label);
    //ui->scrollArea->setVisible(false);
    ui->scrollArea->setMouseTracking(true);

    scale_xy = 1;
    work_mode = true;

    scribbling = false;
    myPenColor = Qt::red;
    myPenWidth = 2;

    pathTreeMode = false;
}

ImageScissor::~ImageScissor()
{
    delete ui;
}

struct greaterNode
{
    bool operator() (Node *a, Node *b)
    {
        return a->totalCost > b->totalCost;
    }
};

void ImageScissor::on_actionOpen_triggered()
{
    //Open the file
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    if(!fileName.isEmpty()){
        //Read in image
        QImage Loaded_Image(fileName);
        if(Loaded_Image.isNull()){
            QMessageBox::warning(this,"..","Failed to load image.");
            return;
        }
        Load_Image = Loaded_Image.copy();
        Contour_Image = Loaded_Image.copy();
        org_width = Loaded_Image.width();
        org_height = Loaded_Image.height();

        qpixmap = QPixmap::fromImage(Loaded_Image);
        ui->label->setPixmap(qpixmap);
        ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
        ui->label->setVisible(true);

        ui->actionZoom_In->setShortcut(QKeySequence::ZoomIn);
        ui->actionZoom_In->setEnabled(true);
        ui->actionZoom_Out->setShortcut(QKeySequence::ZoomOut);
        ui->actionZoom_Out->setEnabled(true);
        ui->actionScissor_Start->setEnabled(true);
        //ui->actionPixel_Node->setEnabled(true);

        //statusBar()->showMessage(QString("%1, %2").arg(org_width).arg(org_height));
        getPixelNode();
        computeCost(Load_Image);
        getCostGraph();
    }

}

void ImageScissor::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
     scrollBar->setValue(int(factor * scrollBar->value()+ ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageScissor::on_actionExit_triggered()
{
    qApp->exit();
}

void ImageScissor::on_actionScissor_Start_triggered()
{
    ui->actionScissor_Start->setDisabled(true);
    ui->actionScissor_Stop->setEnabled(true);
    ui->actionScissor_Undo->setEnabled(true);
}

void ImageScissor::on_actionScissor_Stop_triggered()
{
    ui->actionScissor_Start->setEnabled(true);
    ui->actionScissor_Stop->setDisabled(true);
    ui->actionScissor_Undo->setEnabled(true);
}

void ImageScissor::on_actionScissor_Undo_triggered()
{
    ui->actionScissor_Start->setEnabled(true);
    ui->actionScissor_Stop->setEnabled(true);
    ui->actionScissor_Undo->setDisabled(true);
}

void ImageScissor::on_actionSave_Contour_triggered()
{

}

void ImageScissor::on_actionSave_Mask_triggered()
{

}

void ImageScissor::on_actionZoom_In_triggered()
{
    scale_xy *= 1.2;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
}

void ImageScissor::on_actionZoom_Out_triggered()
{
    scale_xy *= 0.8;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
}

//DEBUG MODE
void ImageScissor::getPixelNode(){
    int curr_width = Load_Image.width();
    int curr_height = Load_Image.height();
    int width = curr_width * 3;
    int height = curr_height * 3;

    //QSize newSize(width, height);
    QImage tmp(width, height, QImage::Format_RGB32);

    tmp.fill(Qt::black);
    for(int x = 0; x < curr_width; x++){
        for(int y = 0; y < curr_height; y++){
            //QPoint b(x*3, y*3);
            tmp.setPixel(x*3+1, y*3+1, Load_Image.pixel(x, y));
        }
    }
    Pixel_Image = tmp.copy();
}

void ImageScissor::on_actionPixel_Node_triggered()
{
    work_mode = false;
    //METHOD1
    ui->label->setPixmap(QPixmap::fromImage(Pixel_Image));
    scale_xy = 1.2;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
    update();

    //cv::Mat input = qimage_to_mat_ref(Pixel_Image);
    //cv::imshow("pixel node mode", input);

}

void ImageScissor::getCostGraph(){
    int curr_width = Load_Image.width();
    int curr_height = Load_Image.height();
    int width = curr_width * 3;
    int height = curr_height * 3;
    int value, id;

    //QSize newSize(width, height);
    QImage tmp(width, height, QImage::Format_RGB32);
    tmp.fill(Qt::white);
    for(int y = 0; y < curr_height; y++){
        for(int x = 0; x < curr_width; x++){
            //QPoint b(x*3, y*3);
            id = y * curr_width + x;
            //value = qGray(Load_Image.pixel(x, y));
            tmp.setPixel(x*3+1, y*3+1, Load_Image.pixel(x, y));

            value = std::min<unsigned long>(graphNode[id]->linkCost[3], 255);
            tmp.setPixel(x*3,   y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[2], 255);
            tmp.setPixel(x*3+1, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[1], 255);
            tmp.setPixel(x*3+2, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[4], 255);
            tmp.setPixel(x*3,   y*3+1, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[0], 255);
            tmp.setPixel(x*3+2, y*3+1, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[5], 255);
            tmp.setPixel(x*3,   y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[6], 255);
            tmp.setPixel(x*3+1, y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[id]->linkCost[7], 255);
            tmp.setPixel(x*3+2, y*3+2, QColor(value, value, value).rgb());
        }
    }

    Cost_Image = tmp.copy();
}

void ImageScissor::on_actionCost_Graph_triggered()
{
    work_mode = false;
    ui->label->setPixmap(QPixmap::fromImage(Cost_Image));
    scale_xy = 1;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
    update();
}

void ImageScissor::getPathTree(int x, int y){
     int curr_id = y*org_width + x;
     my_shortPath->GetPath(curr_id, graphNode, org_width, org_height);
     drawWithPrevNode(graphNode[curr_id]);
}

void ImageScissor::on_actionPath_Tree_triggered()
{
    work_mode = false;
    pathTreeMode = true;
    getPathTree(seed_x, seed_y);
}

void ImageScissor::getMinPath(){

}

void ImageScissor::computeCost(QImage image){
    //declare var
    int w = image.width();
    int h = image.height();
    //int id;
    int rDR, gDR, bDR;
    double max = 0;
    int id;
    graphNode.reserve(1024*1024);
    //get rgb data
    //cv::Mat rgb = qimage_to_mat_ref(image, QImage::Format_RGB32);
    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            //initialize
            id = y * w + x;
            graphNode[id] = new Node();
            graphNode[id]->column = y;
            graphNode[id]->row = x;
            graphNode[id]->state = 0;
            graphNode[id]->maxDeriv = 0;
            graphNode[id]->totalCost = MAX;
            graphNode[id]->prevNode = NULL;
            if(y>=1 && x<(w-1) && y<(h-1)){
                //D(link0)
                rDR = abs((image.pixelColor(x,y-1).red()+image.pixelColor(x+1,y-1).red())-(image.pixelColor(x,y+1).red()+image.pixelColor(x+1,y+1).red()))/4;
                gDR = abs((image.pixelColor(x,y-1).green()+image.pixelColor(x+1,y-1).green())-(image.pixelColor(x,y+1).green()+image.pixelColor(x+1,y+1).green()))/4;
                bDR = abs((image.pixelColor(x,y-1).blue()+image.pixelColor(x+1,y-1).blue())-(image.pixelColor(x,y+1).blue()+image.pixelColor(x+1,y+1).blue()))/4;
                graphNode[id]->linkCost[0] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);

            }
            else{
                graphNode[id]->linkCost[0] = 0;
            }
            if(y>=1 && x<(w-1)){
                //D(link1)
                rDR = abs(image.pixelColor(x+1,y).red()-image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x+1,y).green()-image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x+1,y).blue()-image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[id]->linkCost[1] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[1] = 0;
            }
            if(y>=1 && x>=1 && x<(w-1)){
                //D(link2)
                rDR = abs((image.pixelColor(x-1,y).red()+image.pixelColor(x-1,y-1).red())-(image.pixelColor(x+1,y).red()+image.pixelColor(x+1,y-1).red()))/4;
                gDR = abs((image.pixelColor(x-1,y).green()+image.pixelColor(x-1,y-1).green())-(image.pixelColor(x+1,y).green()+image.pixelColor(x+1,y-1).green()))/4;
                bDR = abs((image.pixelColor(x-1,y).blue()+image.pixelColor(x-1,y-1).blue())-(image.pixelColor(x+1,y).blue()+image.pixelColor(x+1,y-1).blue()))/4;
                graphNode[id]->linkCost[2] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[2] = 0;
            }
            if(y>=1 && x>=1){
                //D(link3)
                rDR = abs(image.pixelColor(x-1,y).red()-image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x-1,y).green()-image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x-1,y).blue()-image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[id]->linkCost[3] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[3] = 0;
            }
            if(y>=1 && x>=1 && y<(h-1)){
                //D(link4)
                rDR = abs((image.pixelColor(x-1,y-1).red()+image.pixelColor(x,y-1).red())-(image.pixelColor(x,y+1).red()+image.pixelColor(x-1,y+1).red()))/4;
                gDR = abs((image.pixelColor(x-1,y-1).green()+image.pixelColor(x,y-1).green())-(image.pixelColor(x,y+1).green()+image.pixelColor(x-1,y+1).green()))/4;
                bDR = abs((image.pixelColor(x-1,y-1).blue()+image.pixelColor(x,y-1).blue())-(image.pixelColor(x,y+1).blue()+image.pixelColor(x-1,y+1).blue()))/4;
                graphNode[id]->linkCost[4] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[4] = 0;
            }
            if(x>=1 && y<(h-1)){
                //D(link5)
                rDR = abs(image.pixelColor(x-1,y).red()-image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x-1,y).green()-image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x-1,y).blue()-image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[id]->linkCost[5] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[5] = 0;
            }
            if(x>=1 && y<(h-1) && x<(w-1)){
                //D(link6)
                rDR = abs((image.pixelColor(x-1,y+1).red()+image.pixelColor(x-1,y).red())-(image.pixelColor(x+1,y+1).red()+image.pixelColor(x+1,y).red()))/4;
                gDR = abs((image.pixelColor(x-1,y+1).green()+image.pixelColor(x-1,y).green())-(image.pixelColor(x+1,y+1).green()+image.pixelColor(x+1,y).green()))/4;
                bDR = abs((image.pixelColor(x-1,y+1).blue()+image.pixelColor(x-1,y).blue())-(image.pixelColor(x+1,y+1).blue()+image.pixelColor(x+1,y).blue()))/4;
                graphNode[id]->linkCost[6] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[6] = 0;
            }
            if(y<(h-1) && x<(w-1)){
                //D(link7)
                rDR = abs(image.pixelColor(x+1,y).red()-image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(image.pixelColor(x+1,y).green()-image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(image.pixelColor(x+1,y).blue()-image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[id]->linkCost[7] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[id]->linkCost[7] = 0;
            }

            max = 0;
            for(int i = 0;i < 8;i++)
            {
                if(graphNode[id]->linkCost[i] > max)
                    max = graphNode[id]->linkCost[i];
                if(graphNode[id]->linkCost[i] > graphNode[id]->maxDeriv)
                    graphNode[id]->maxDeriv = graphNode[id]->linkCost[i];
            }
        }
    }
    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            for(int i = 0;i < 8;i++){
                graphNode[id]->linkCost[i] = (max - graphNode[id]->linkCost[i])* 1;
                if(i%2 != 0)
                    graphNode[id]->linkCost[i] *= sqrt(2);
            }
        }
    }

}

//WORD MODE UI
void ImageScissor::on_actionImage_Only_triggered()
{
    ui->label->setPixmap(QPixmap::fromImage(Load_Image));
    scale_xy = 1;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
    update();
}

//DRAW LINES
void ImageScissor::setPenColor(const QColor &newColor)
{
    myPenColor = newColor;
}

void ImageScissor::setPenWidth(int newWidth){
    myPenWidth = newWidth;
}

void ImageScissor::drawLineTo_example(const QPoint &endPoint)
{
    QPainter painter(&Load_Image);
    lastPoint = seeds.last();
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);
    //modified = true;

    //int rad = (myPenWidth / 2) + 2;
    //update(QRect(lastPoint, endPoint).normalized()
    //                                 .adjusted(-rad, -rad, +rad, +rad));
    lastPoint = endPoint;
    update();

}

//DRAW NODE
void ImageScissor::drawWithPrevNode(Node* node)
{
    if(seeds.isEmpty())
        return;
    Node* newNode = node;
    QPainter painter(&Load_Image);

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    if(node != NULL)
    {
        painter.drawPoint(QPoint(newNode->row,newNode->column));
        newNode = node->prevNode;
    }
    ui->label->setPixmap(QPixmap::fromImage(Load_Image));
    //update();
}

QPoint ImageScissor::convert_position(QPoint point)
{
    int range = 4;
    QPoint converted_p = point;// - QPoint(15,15);
    converted_p /= scale_xy;//QPoint(imageLabel->width(), imageLabel->height());
    //point = imageLabel->mapFromGlobal(point) + QPoint(138,43);
    //converted_p -= QPoint(15,15);
    int newX = converted_p.x() - range;
    int newY = converted_p.y() - range;
    int relocX, relocY;
    double localMax = 0.0;
    for(int i = 0;i < range * 2 + 1;i++)
    {
        for(int j = 0;j < range * 2 + 1;j++)
        {
            int tmpX = newX + i;
            int tmpY = newY + j;
            int tmpid = tmpY * org_width + tmpX;
            if(tmpX > 0
              && tmpX < ui->label->pixmap()->width()
              && tmpY > 0
              && tmpY < ui->label->pixmap()->height()){
                if(localMax < graphNode[tmpid]->maxDeriv){
                    localMax = graphNode[tmpid]->maxDeriv;
                    relocX = tmpX;
                    relocY = tmpY;
                }

            }
        }
    }

    converted_p = QPoint(relocX,relocY);
    return converted_p;
}

//MOUSE EVENTS
void ImageScissor::mousePressEvent(QMouseEvent *event)
{

    if(event->button() == Qt::LeftButton && work_mode){
        QPoint seed_pos = event->pos();
        seed_x = seed_pos.x()/scale_xy;
        seed_y = seed_pos.y()/scale_xy;
        if(seed_x >= 0 && seed_x <= org_width && seed_y >= 0 && seed_y <= org_height){
            scribbling = true;
            if(seeds.isEmpty()){
                seeds.append(seed_pos);
            }
            else{
                int id = lastPoint.y()*org_width+lastPoint.x();
                //GetPath(id, graphNode, org_width , org_height);
                drawWithPrevNode(graphNode[seed_y*org_width+seed_x]);
                seeds.append(seed_pos);
            }
            lastPoint = seed_pos;
            statusBar()->showMessage(QString("%1").arg(seeds.size()));
        }
    }

}

void ImageScissor::mouseMoveEvent(QMouseEvent *event)
{
    /* EXAMPLE:
     * if((event->buttons() & Qt::LeftButton) && scribbling){
        drawLineTo(event->pos());
    }
    */
    //QPoint curr_pos = convert_position(event->pos());
    //my_x = curr_pos.x();
    //my_y = curr_pos.y();
    //int id = my_y*org_height + my_x;
    statusBar()->showMessage(QString("%1, %2").arg(event->pos().x()).arg(event->pos().y()));
}

void ImageScissor::mouseReleaseEvent(QMouseEvent *event)
{
    //statusBar()->showMessage(QString("%1, %2, %3").arg("prev_id").arg(event->button()==Qt::LeftButton).arg(work_mode));
    if(event->button() == Qt::LeftButton && work_mode){
        //statusBar()->showMessage(QString("%1").arg(100));

        QPoint curr_pos = event->pos();
        int id = curr_pos.y() * org_width + curr_pos.x();
        int prev_id = lastPoint.y() * org_width + lastPoint.x();
        statusBar()->showMessage(QString("%1, %2").arg(prev_id).arg(id));
        //GetPath(prev_id, graphNode, org_width , org_height);
        //drawWithPrevNode(graphNode[id]);

    }
}

/*
void ImageScissor::paintEvent(QPaintEvent *event)
{
    //QLabel::paintEvent(event);
    if(work_mode){
        Contour_Image = Load_Image;
        QPainter painter(&Contour_Image);
        QRect dirtyRect = event->rect();
        painter.drawImage(dirtyRect, Contour_Image, dirtyRect);
        ui->label->setPixmap(QPixmap::fromImage(Contour_Image));
        update();
    }

}*/

void ImageScissor::GetPath(int st, std::vector<Node*> &nodes, int w, int h)
{
    //starting point
    statusBar()->showMessage(QString("starting id %1, queue size %2").arg(st));
    nodes[st]->totalCost = 0;
    nodes[st]->prevNode = NULL;
    std::priority_queue<Node*, std::vector<Node*>, greaterNode> pq;
    pq.push(nodes[st]);
    statusBar()->showMessage(QString("starting id %1, queue size %2").arg(st).arg(pq.size()));

    while(!pq.empty()){
        Node *a = pq.top();
        pq.pop();
        a->state = 2;
        int a_x = a->column;
        int a_y = a->row;
        int idx = a_y*w + a_x;
        for(int i = 0; i < 8; i++){
            //for each direction, get the offset in x and y
            int off_x, off_y, new_idx;
            a->graph(off_x, off_y, i);
            int new_x = a_x + off_x;
            int new_y = a_y + off_y;
            if (new_x >= 0 && new_y >=0 && new_x <= w && new_y <= h){
                    new_idx = new_y*w + new_x;
            }
            else{
                continue;
            }
            int new_state = nodes[new_idx]->state;
            if (new_state == 0){
                nodes[new_idx]->prevNode=nodes[idx];
                nodes[new_idx]->totalCost=nodes[idx]->totalCost+nodes[idx]->linkCost[i];
                nodes[new_idx]->state=1;
                pq.push(nodes[new_idx]);
                statusBar()->showMessage(QString("starting id %1, queue size %2").arg(st).arg(pq.size()));
            }
            else if(new_state == 1){
                int current_cost = nodes[idx]->totalCost+nodes[idx]->linkCost[i];
                if(current_cost < nodes[new_idx]->totalCost){
                    nodes[new_idx]->prevNode = nodes[idx];
                    nodes[idx]->totalCost = current_cost;
                }
            }
        }
    }
}




