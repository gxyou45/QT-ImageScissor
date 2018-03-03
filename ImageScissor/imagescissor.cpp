#include "imagescissor.h"
#include "ui_imagescissor.h"

Node graphNode[2048][2048];
double costMax = 0;

static void initialGraphNode(const QImage *loadImage){
    costMax = 0;
    int id;
    for(int i = 0;i < loadImage->width();i++)
    {
        for(int j = 0; j < loadImage->height();j++)
        {
            id = j * loadImage->width() + i;
            graphNode[i][j].row = i;
            graphNode[i][j].column = j;
            graphNode[i][j].state = 0;
            graphNode[i][j].prevNode = NULL;
            graphNode[i][j].totalCost = MAX;
            graphNode[i][j].maxDeriv = 0;
        }
    }
}

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
    ui->scrollArea->setMouseTracking(true);

    scale_xy = 1;
    work_mode = false;

    scribbling = false;
    pathTreeMode = false;
    debugEnable = false;
    minPathEnable = false;

}

ImageScissor::~ImageScissor()
{
    delete ui;
}

void ImageScissor::setImage(const QImage &newImage)
{
    Load_Image = newImage;
    Contour_Image = newImage.copy();
    org_width = newImage.width();
    org_height = newImage.height();
    qpixmap = QPixmap::fromImage(Load_Image);
    ui->label->setPixmap(qpixmap);

    org_width = Load_Image.width();
    org_height = Load_Image.height();

    scale_xy = 1.0;

    ui->label->setVisible(true);

    double factor = 400.0/Load_Image.width();
    scaleImage(factor);
    computeCost();

    lastPoint = QPoint(0,0);
    seedPoints.clear();
    wirePoints.clear();
    wirePointsVector.clear();

}

void ImageScissor::scaleImage(double factor)
{
    Q_ASSERT(ui->label->pixmap());
    scale_xy *= factor;
    ui->label->resize(scale_xy * Load_Image.size());

    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), factor);
}

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


        initialGraphNode(&Loaded_Image);
        blur0 = Loaded_Image;
        blur2 = blurred(Loaded_Image,Loaded_Image.rect(),2,false);
        blur4 = blurred(Loaded_Image,Loaded_Image.rect(),4,false);
        blur8 = blurred(Loaded_Image,Loaded_Image.rect(),8,false);
        setImage(Loaded_Image);


        ui->actionZoom_In->setShortcut(QKeySequence::ZoomIn);
        ui->actionZoom_In->setEnabled(true);
        ui->actionZoom_Out->setShortcut(QKeySequence::ZoomOut);
        ui->actionZoom_Out->setEnabled(true);
        ui->actionScissor_Start->setEnabled(true);

        this->finishCurrentContourSC = new QShortcut(QKeySequence("Return"), this);
        QObject::connect(this->finishCurrentContourSC, SIGNAL(activated()), this, SLOT(finishCurrentContour()));
        this->finishCurrentContourSC->setEnabled(false);

        this->finishCurrentContourCloseSC = new QShortcut(QKeySequence("Ctrl+Return"), ui->label);
        QObject::connect(this->finishCurrentContourCloseSC, SIGNAL(activated()), ui->label, SLOT(finishCurrentContour()));
        this->finishCurrentContourCloseSC->setEnabled(false);

        this->undoSC = new QShortcut(QKeySequence("Backspace"), ui->label);
        QObject::connect(this->undoSC, SIGNAL(activated()), ui->label, SLOT(undo()));
        this->undoSC->setEnabled(false);

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
    scribbling = true;
    moveEnable = true;
    this->finishCurrentContourSC->setEnabled(true);
    this->finishCurrentContourCloseSC->setEnabled(true);
    this->undoSC->setEnabled(true);
}

void ImageScissor::on_actionScissor_Stop_triggered()
{
    ui->actionScissor_Start->setEnabled(true);
    ui->actionScissor_Stop->setDisabled(true);
    ui->actionScissor_Undo->setEnabled(true);
    finishCurrentContour();
}

void ImageScissor::on_actionScissor_Undo_triggered()
{
    ui->actionScissor_Start->setEnabled(true);
    ui->actionScissor_Stop->setEnabled(true);
    ui->actionScissor_Undo->setDisabled(true);
    undo();
}

void ImageScissor::on_actionSave_Contour_triggered()
{
    ui->label->setPixmap(qpixmap);
    QImage tmp = qpixmap.toImage();
    QString fileName = QFileDialog::getSaveFileName(this,"Save",QDir::currentPath(),"JPG-Files (*.jpg)");
    if (!fileName.isEmpty()) {
         if (tmp.isNull()) {
             QMessageBox::information(this, tr("Error"), tr("Please Load an Image First!"));
             return;
         }
        tmp.save(fileName);
    }
}

void ImageScissor::on_actionSave_Mask_triggered()
{
    scribbling = false;
    moveEnable = false;
    debugEnable = false;
    minPathEnable = false;

    if(wirePointsVector.isEmpty())
        return;

    int tmpWidth = Load_Image.width();
    int tmpHeight = Load_Image.height();

    QImage tmp(tmpWidth, tmpHeight, QImage::Format_ARGB32);
    tmp.fill(QColor(255,255,255,0).rgba());

    bool *maskMatrix = (bool*)malloc(tmpWidth * tmpHeight * sizeof(bool));

    for(int j = 0; j < tmpHeight;j++){
        for(int i = 0;i < tmpWidth;i++){
            maskMatrix[j * tmpWidth + i] = false;
        }
    }

    for(int i = 0;i < wirePointsVector.size();i++){
        for(int j = wirePointsVector.at(i).size() - 1;j >= 0;j--){
            maskMatrix[wirePointsVector.at(i).at(j).y() * tmpWidth + wirePointsVector.at(i).at(j).x()] = true;
        }
    }

    QVector<QPoint> expandMaskVec;
    expandMaskVec.append(QPoint(0,0));

    while(!expandMaskVec.isEmpty()){
        QPoint point = expandMaskVec.last();
        expandMaskVec.pop_back();
        int tmpX = point.x();
        int tmpY = point.y();

        if(tmpX >= 0 && tmpX <= tmpWidth && tmpY >= 0 && tmpY <= tmpHeight){
            int index = tmpY * tmpWidth + tmpX;
            if(!maskMatrix[index]){
                maskMatrix[index] = true;
                expandMaskVec.append(QPoint(tmpX - 1,tmpY));
                expandMaskVec.append(QPoint(tmpX + 1,tmpY));
                expandMaskVec.append(QPoint(tmpX,tmpY - 1));
                expandMaskVec.append(QPoint(tmpX,tmpY + 1));
            }
        }
    }

    for(int i = 0;i < tmpWidth * tmpHeight;i++){
        if(!maskMatrix[i]){
            int x = i % tmpWidth;
            int y = (int)(i / tmpWidth);
            QPoint tmpP(x,y);
            tmp.setPixel(tmpP,Load_Image.pixel(tmpP));
        }
    }

    delete maskMatrix;
    ui->label->setPixmap(QPixmap::fromImage(tmp));

    QString fileName = QFileDialog::getSaveFileName(this,"Save",QDir::currentPath(),"JPG-Files (*.jpg)");
    if (!fileName.isEmpty()){
         if (tmp.isNull()){
             QMessageBox::information(this, tr("Error"),
                                      tr("Please Load an Image First!"));
             return;
         }
        tmp.save(fileName);
    }

}

void ImageScissor::on_actionZoom_In_triggered()
{
    scaleImage(1.25);
}

void ImageScissor::on_actionZoom_Out_triggered()
{
    scaleImage(0.8);
}

//DEBUG MODE
void ImageScissor::getPixelNode()
{
    int curr_width = Load_Image.width();
    int curr_height = Load_Image.height();
    int width = curr_width * 3;
    int height = curr_height * 3;

    QImage tmp(width, height, QImage::Format_RGB32);

    tmp.fill(Qt::black);
    for(int x = 0; x < curr_width; x++){
        for(int y = 0; y < curr_height; y++){
            tmp.setPixel(x*3+1, y*3+1, Load_Image.pixel(x, y));
        }
    }
    Pixel_Image = tmp.copy();
}

void ImageScissor::on_actionPixel_Node_triggered()
{
    work_mode = false;
    getPixelNode();
    ui->label->setPixmap(QPixmap::fromImage(Pixel_Image));
    update();
}

void ImageScissor::getCostGraph(){
    int curr_width = Load_Image.width();
    int curr_height = Load_Image.height();
    int width = curr_width * 3;
    int height = curr_height * 3;
    int value;

    QImage tmp(width, height, QImage::Format_RGB32);
    tmp.fill(Qt::white);
    for(int y = 0; y < curr_height; y++){
        for(int x = 0; x < curr_width; x++){

            tmp.setPixel(x*3+1, y*3+1, Load_Image.pixel(x, y));

            value = std::min<unsigned long>(graphNode[x][y].linkCost[3], 255);
            tmp.setPixel(x*3,   y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[2], 255);
            tmp.setPixel(x*3+1, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[1], 255);
            tmp.setPixel(x*3+2, y*3,   QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[4], 255);
            tmp.setPixel(x*3,   y*3+1, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[0], 255);
            tmp.setPixel(x*3+2, y*3+1, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[5], 255);
            tmp.setPixel(x*3,   y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[6], 255);
            tmp.setPixel(x*3+1, y*3+2, QColor(value, value, value).rgb());

            value = std::min<unsigned long>(graphNode[x][y].linkCost[7], 255);
            tmp.setPixel(x*3+2, y*3+2, QColor(value, value, value).rgb());
        }
    }

    Cost_Image = tmp.copy();
}

void ImageScissor::on_actionCost_Graph_triggered()
{
    work_mode = false;
    getCostGraph();
    ui->label->setPixmap(QPixmap::fromImage(Cost_Image));
    update();
}

void ImageScissor::getPathTree(){
    minPathEnable = false;
    moveEnable = false;
    scribbling = false;
    debugEnable = true;

    if(seedPoints.isEmpty())
        return;
    GetPath(seedPoints.last());

    int tmpWidth = 3 * Load_Image.width();
    int tmpHeight = 3 * Load_Image.height();

    QImage tmp(tmpWidth, tmpHeight, QImage::Format_RGB32);
    tmp.fill(Qt::black);
    ui->label->setPixmap(QPixmap::fromImage(tmp));

    int count = 0;

    std::priority_queue<Node*, std::vector<Node*>, greaterNode> treeq;
    for(int x = 0; x < Load_Image.width(); x++){
        for(int y = 0; y < Load_Image.height(); y++){
            treeq.push(&graphNode[x][y]);
        }
    }
    int a_x, a_y;
    while(!treeq.empty()){
        Node* a = treeq.top();
        a_x = a->row;
        a_y = a->column;
        treeq.pop();
        int value = int(255 * a->totalCost /(costMax * 2) + 125);
        tmp.setPixel(3 * a_x + 1, 3 * a_y + 1, QColor(value, value,0).rgb());
        if(graphNode[a_x][a_y].prevNode != NULL)
        {
            int i = graphNode[a_x][a_y].prevNode->row - graphNode[a_x][a_y].row;
            int j = graphNode[a_x][a_y].prevNode->column - graphNode[a_x][a_y].column;

            tmp.setPixel(3 * a_x + 1 + i, 3 * a_y + 1 + j, QColor(value, value,0).rgb());
            tmp.setPixel(3 * a_x + 1 + 2 * i, 3 * a_y + 1 + 2 * j, QColor(value, value,0).rgb());
         }
        count++;
        if(count % 5000 == 0){
            ui->label->setPixmap(QPixmap::fromImage(tmp));
            delay();
        }
    }

    ui->label->setPixmap(QPixmap::fromImage(tmp));
}

void ImageScissor::delay(){
    QTime dieTime= QTime::currentTime().addMSecs(50);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void ImageScissor::on_actionPath_Tree_triggered()
{
    getPathTree();
}

void ImageScissor::getMinPath(){
    scribbling = false;
    moveEnable = true;
    debugEnable = true;
    minPathEnable = true;

    if(seedPoints.isEmpty())
        return;
    GetPath(seedPoints.last());

    int tmpWidth = 3 * Load_Image.width();
    int tmpHeight = 3 * Load_Image.height();

    QImage tmp(tmpWidth, tmpHeight, QImage::Format_RGB32);
    tmp.fill(Qt::black);
    ui->label->setPixmap(QPixmap::fromImage(tmp));

    std::priority_queue<Node*, std::vector<Node*>, greaterNode> treeq;
    for(int x = 0; x < Load_Image.width(); x++){
        for(int y = 0; y < Load_Image.height(); y++){
            treeq.push(&graphNode[x][y]);
        }
    }
    int a_x, a_y;
    while(!treeq.empty()){
        Node* a = treeq.top();
        a_x = a->row;
        a_y = a->column;
        treeq.pop();
        int value = int(255 * a->totalCost /(costMax * 2) + 125);
        tmp.setPixel(3 * a_x + 1, 3 * a_y + 1, QColor(value, value,0).rgb());
        if(graphNode[a_x][a_y].prevNode != NULL)
        {
            int i = graphNode[a_x][a_y].prevNode->row - graphNode[a_x][a_y].row;
            int j = graphNode[a_x][a_y].prevNode->column - graphNode[a_x][a_y].column;

            tmp.setPixel(3 * a_x + 1 + i, 3 * a_y + 1 + j, QColor(value, value,0).rgb());
            tmp.setPixel(3 * a_x + 1 + 2 * i, 3 * a_y + 1 + 2 * j, QColor(value, value,0).rgb());
         }
    }
    minPath_qpixmap = QPixmap::fromImage(tmp);
    ui->label->setPixmap(minPath_qpixmap);
}

void ImageScissor::on_actionMin_Path_triggered()
{
    getMinPath();
}

//WORD MODE UI
void ImageScissor::on_actionImage_Only_triggered()
{
    ui->label->setPixmap(QPixmap::fromImage(Load_Image));
    scale_xy = 1;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
    update();
}

//DRAW NODE
void ImageScissor::drawWithPrevNode(QPoint mousePoint)
{
    if(seedPoints.isEmpty())
        return;
    //Node* newNode = node;
    QImage tmpImage = Load_Image.copy();
    QPainter painter(&tmpImage);

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    Node *node = &graphNode[mousePoint.x()][mousePoint.y()];

    if(node != NULL)
    {
        statusBar()->showMessage(QString("Current cost%1").arg(node->totalCost));
        painter.drawPoint(QPoint(node->row,node->column));
        node = node->prevNode;
    }
    ui->label->setPixmap(QPixmap::fromImage(tmpImage));
}

void ImageScissor::drawMinPath(QPoint mousePoint){

    if(seedPoints.isEmpty() || !moveEnable)
        return;

    ui->label->setPixmap(qpixmap);
    if(mousePoint.x() <= 0 ||mousePoint.y() <= 0 || !atImage(mousePoint))
        return;

    QPixmap tmpPixmap = qpixmap.copy();
    QPainter painter(&tmpPixmap);

    painter.setPen(QPen(Qt::yellow, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));


    Node *node = &graphNode[mousePoint.x()][mousePoint.y()];

    while(node != NULL)
    {
        painter.drawPoint(QPoint(node->row,node->column));
        node = node->prevNode;
    }
    ui->label->setPixmap(tmpPixmap);
    statusBar()->showMessage(QString("drawMinPath%1,%2,%3").arg(wirePoints.size()).arg(seedPoints.size()).arg(node == NULL));

}

void ImageScissor::minPathEnable_drawMinPath(QPoint mousePoint)
{
    if(seedPoints.isEmpty() || !moveEnable || !minPathEnable)
            return;

    ui->label->setPixmap(minPath_qpixmap);
    if(mousePoint.x() <= 0 ||mousePoint.y() <= 0 || !atImage(mousePoint))
    {
        ui->label->setPixmap(minPath_qpixmap);
        return;
    }

    QPixmap tmpPixmap = minPath_qpixmap.copy();
    QPainter painter(&tmpPixmap);

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    Node *node = &graphNode[mousePoint.x()][mousePoint.y()];
    statusBar()->showMessage(QString("Mouse(%1,%2)*%3 = Node(%4,%5)").arg(mousePoint.x()).arg(mousePoint.y()).
                             arg(scale_xy).arg(node->row).arg(node->column));

    while(node != NULL)
    {
        painter.drawPoint(QPoint(int(node->row*3),int(node->column*3)));

        if(node->prevNode != NULL)
        {
            int i = node->prevNode->row - node->row;
            int j = node->prevNode->column - node->column;

            painter.drawPoint(QPoint(node->row *3 + 1 + i, node->column *3 + 1 + j));
            painter.drawPoint(QPoint(node->row *3 + 1 + 2 * i, node->column*3 + 1 + 2 * j));
         }

        node = node->prevNode;
    }
    ui->label->setPixmap(tmpPixmap);
}

//MOUSE EVENTS
void ImageScissor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling)
        lastPoint = cursorSnap(event->pos());

}

void ImageScissor::mouseMoveEvent(QMouseEvent *event)
{

    if(Load_Image.isNull())
        lastPoint = convert_position(event->pos());
    else
        lastPoint = cursorSnap(event->pos());

    if(!scribbling && !moveEnable)
        selectContour();
    else if(scribbling && !minPathEnable)
        drawMinPath(lastPoint);
    else if(moveEnable && minPathEnable)
        minPathEnable_drawMinPath(lastPoint);

    statusBar()->showMessage(QString("%1, %2").arg(event->pos().x()).arg(event->pos().y()));
}

void ImageScissor::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_Control && !debugEnable){
       scribbling = true;
       moveEnable = true;
       finishCurrentContourSC->setEnabled(true);
       finishCurrentContourCloseSC->setEnabled(true);
       undoSC->setEnabled(true);

    }
}

void ImageScissor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        if(!atImage(lastPoint))
            return;
        addFollowingSeedPoint();

    }
}

void ImageScissor::computeCost(){
    //declare var
    int w = Load_Image.width();
    int h = Load_Image.height();
    int rDR, gDR, bDR;
    double max = 0;
    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            if(y>=1 && x<(w-1) && y<(h-1)){
                //D(link0)
                rDR = abs((Load_Image.pixelColor(x,y-1).red()+  Load_Image.pixelColor(x+1,y-1).red())-  (Load_Image.pixelColor(x,y+1).red()+  Load_Image.pixelColor(x+1,y+1).red()))/4;
                gDR = abs((Load_Image.pixelColor(x,y-1).green()+Load_Image.pixelColor(x+1,y-1).green())-(Load_Image.pixelColor(x,y+1).green()+Load_Image.pixelColor(x+1,y+1).green()))/4;
                bDR = abs((Load_Image.pixelColor(x,y-1).blue()+ Load_Image.pixelColor(x+1,y-1).blue())- (Load_Image.pixelColor(x,y+1).blue()+ Load_Image.pixelColor(x+1,y+1).blue()))/4;
                graphNode[x][y].linkCost[0] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);

            }
            else{
                graphNode[x][y].linkCost[0] = 0;
            }
            if(y>=1 && x<(w-1)){
                //D(link1)
                rDR = abs(Load_Image.pixelColor(x+1,y).red()-  Load_Image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(Load_Image.pixelColor(x+1,y).green()-Load_Image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(Load_Image.pixelColor(x+1,y).blue()- Load_Image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[1] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[1] = 0;
            }
            if(y>=1 && x>=1 && x<(w-1)){
                //D(link2)
                rDR = abs((Load_Image.pixelColor(x-1,y).red()+  Load_Image.pixelColor(x-1,y-1).red())-  (Load_Image.pixelColor(x+1,y).red()+  Load_Image.pixelColor(x+1,y-1).red()))/4;
                gDR = abs((Load_Image.pixelColor(x-1,y).green()+Load_Image.pixelColor(x-1,y-1).green())-(Load_Image.pixelColor(x+1,y).green()+Load_Image.pixelColor(x+1,y-1).green()))/4;
                bDR = abs((Load_Image.pixelColor(x-1,y).blue()+ Load_Image.pixelColor(x-1,y-1).blue())- (Load_Image.pixelColor(x+1,y).blue()+ Load_Image.pixelColor(x+1,y-1).blue()))/4;
                graphNode[x][y].linkCost[2] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[2] = 0;
            }
            if(y>=1 && x>=1){
                //D(link3)
                rDR = abs(Load_Image.pixelColor(x-1,y).red()-  Load_Image.pixelColor(x,y-1).red())/sqrt(2.0);
                gDR = abs(Load_Image.pixelColor(x-1,y).green()-Load_Image.pixelColor(x,y-1).green())/sqrt(2.0);
                bDR = abs(Load_Image.pixelColor(x-1,y).blue()- Load_Image.pixelColor(x,y-1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[3] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[3] = 0;
            }
            if(y>=1 && x>=1 && y<(h-1)){
                //D(link4)
                rDR = abs((Load_Image.pixelColor(x-1,y-1).red()+  Load_Image.pixelColor(x,y-1).red())-  (Load_Image.pixelColor(x,y+1).red()+  Load_Image.pixelColor(x-1,y+1).red()))/4;
                gDR = abs((Load_Image.pixelColor(x-1,y-1).green()+Load_Image.pixelColor(x,y-1).green())-(Load_Image.pixelColor(x,y+1).green()+Load_Image.pixelColor(x-1,y+1).green()))/4;
                bDR = abs((Load_Image.pixelColor(x-1,y-1).blue()+ Load_Image.pixelColor(x,y-1).blue())- (Load_Image.pixelColor(x,y+1).blue()+ Load_Image.pixelColor(x-1,y+1).blue()))/4;
                graphNode[x][y].linkCost[4] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[4] = 0;
            }
            if(x>=1 && y<(h-1)){
                //D(link5)
                rDR = abs(Load_Image.pixelColor(x-1,y).red()-  Load_Image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(Load_Image.pixelColor(x-1,y).green()-Load_Image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(Load_Image.pixelColor(x-1,y).blue()- Load_Image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[5] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[5] = 0;
            }
            if(x>=1 && y<(h-1) && x<(w-1)){
                //D(link6)
                rDR = abs((Load_Image.pixelColor(x-1,y+1).red()+  Load_Image.pixelColor(x-1,y).red())-  (Load_Image.pixelColor(x+1,y+1).red()+  Load_Image.pixelColor(x+1,y).red()))/4;
                gDR = abs((Load_Image.pixelColor(x-1,y+1).green()+Load_Image.pixelColor(x-1,y).green())-(Load_Image.pixelColor(x+1,y+1).green()+Load_Image.pixelColor(x+1,y).green()))/4;
                bDR = abs((Load_Image.pixelColor(x-1,y+1).blue()+ Load_Image.pixelColor(x-1,y).blue())- (Load_Image.pixelColor(x+1,y+1).blue()+ Load_Image.pixelColor(x+1,y).blue()))/4;
                graphNode[x][y].linkCost[6] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[6] = 0;
            }
            if(y<(h-1) && x<(w-1)){
                //D(link7)
                rDR = abs(Load_Image.pixelColor(x+1,y).red()-  Load_Image.pixelColor(x,y+1).red())/sqrt(2.0);
                gDR = abs(Load_Image.pixelColor(x+1,y).green()-Load_Image.pixelColor(x,y+1).green())/sqrt(2.0);
                bDR = abs(Load_Image.pixelColor(x+1,y).blue()- Load_Image.pixelColor(x,y+1).blue())/sqrt(2.0);
                graphNode[x][y].linkCost[7] = sqrt((rDR*rDR+gDR*gDR+bDR*bDR)/3);
            }
            else{
                graphNode[x][y].linkCost[7] = 0;
            }

            for(int i = 0;i < 8;i++)
            {
                if(graphNode[x][y].linkCost[i] > max)
                    max = graphNode[x][y].linkCost[i];
                if(graphNode[x][y].linkCost[i] > graphNode[x][y].maxDeriv)
                    graphNode[x][y].maxDeriv = graphNode[x][y].linkCost[i];
            }
        }

    }

    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            for(int i = 0;i < 8;i++){
                graphNode[x][y].linkCost[i] = (max - graphNode[x][y].linkCost[i]) * 1;
                if(i%2 != 0)
                    graphNode[x][y].linkCost[i] *= sqrt(2);
            }
        }
    }

}

void ImageScissor::GetPath(QPoint st)
{
    costMax = 0;

    //initialize
    for(int i = 0;i < Load_Image.width();i++){
        for(int j = 0; j < Load_Image.height();j++){
           graphNode[i][j].prevNode = NULL;
           graphNode[i][j].state  = 0;
           graphNode[i][j].totalCost = MAX;
        }
    }

    graphNode[st.x()][st.y()].totalCost = 0.0;
    graphNode[st.x()][st.y()].prevNode = NULL;
    std::priority_queue<Node*, std::vector<Node*>, greaterNode> pq;
    pq.push(&graphNode[st.x()][st.y()]);

    while(!pq.empty()){
        Node* a = pq.top();
        pq.pop();
        a->state = 2;
        int a_x = a->row;
        int a_y = a->column;

        if(a->totalCost > costMax)
            costMax = a->totalCost;

        for(int i = 0; i < 8; i++){
            //for each direction, get the offset in x and y
            int off_x, off_y;
            a->graph(off_x, off_y, i);
            int new_x = a_x + off_x;
            int new_y = a_y + off_y;
            if (new_x < 0 || new_y  < 0){
                continue;
            }
            int new_state = graphNode[new_x][new_y].state;
            if (new_state == 0){
                graphNode[new_x][new_y].prevNode= &graphNode[a_x][a_y];
                graphNode[new_x][new_y].totalCost= graphNode[a_x][a_y].totalCost + graphNode[a_x][a_y].linkCost[i];
                graphNode[new_x][new_y].state=1;
                pq.push(&graphNode[new_x][new_y]);
            }
            else if(new_state == 1){
                int current_cost = graphNode[a_x][a_y].totalCost + graphNode[a_x][a_y].linkCost[i];
                if(current_cost < graphNode[new_x][new_y].totalCost){
                    graphNode[new_x][new_y].prevNode = &graphNode[a_x][a_y];
                    graphNode[a_x][a_y].totalCost = current_cost;
                }
            }
        }
    }
}

void ImageScissor::addFollowingSeedPoint()
{
    if(Load_Image.isNull() || !scribbling)
        return;

    if(!atImage(lastPoint))
        return;

    Node *node = &graphNode[lastPoint.x()][lastPoint.y()];
    if(seedPoints.isEmpty()){
        wirePoints.append(QPoint(lastPoint.x(), lastPoint.y()));
    }
    else{
        QPoint lastSeed = seedPoints.last();
        QVector<QPoint> tmpVec;
        while(node != NULL){
            if(node->row == lastSeed.x() && node->column == lastSeed.y())
                break;

            tmpVec.prepend(QPoint(node->row,node->column));
            node = node->prevNode;
        }
        wirePoints += tmpVec;
    }

    drawLineWithNode();
    seedPoints.append(QPoint(lastPoint.x(),lastPoint.y()));
    GetPath(seedPoints.last());
    statusBar()->showMessage(QString("%1, %2").arg(wirePoints.size()).arg(seedPoints.size()));
}

QPoint ImageScissor::convert_position(QPoint point)
{
    QPoint converted_p = point;
    converted_p /= scale_xy;
    return converted_p;
}

QPoint ImageScissor::cursorSnap(QPoint point)
{
    int range = 4;
    QPoint converted_p = point;
    converted_p /= scale_xy;

    int newX = converted_p.x() - range;
    int newY = converted_p.y() - range;
    int relocX = converted_p.x(), relocY = converted_p.y();
    double localMax = 0.0;
    for(int i = 0;i < range * 2 + 1;i++){
        for(int j = 0;j < range * 2 + 1;j++){
            int tmpX = newX + i;
            int tmpY = newY + j;
            //make sure mouse location is not at the edge of the picture;
            if(atImage(QPoint(tmpX,tmpY))){
                if(localMax < graphNode[tmpX][tmpY].maxDeriv){
                    localMax = graphNode[tmpX][tmpY].maxDeriv;
                    relocX = tmpX;
                    relocY = tmpY;
                }

            }
        }
    }

    //force the node not to be at the image edges
    if(relocX == 0)
        relocX += 1;
    else if(relocX == ui->label->pixmap()->height() - 1)
        relocX -= 1;

    if(relocY == 0)
        relocY += 1;
    else if(relocY == ui->label->pixmap()->height() - 1)
        relocY -= 1;

    converted_p = QPoint(relocX,relocY);
    return converted_p;
}

void ImageScissor::drawLineWithNode()
{
    QPainter painter(&qpixmap);

    for(int i = 0;i < wirePointsVector.size();i++){

        if(i == selectedContour)
            painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                                Qt::RoundJoin));
        else
            painter.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::RoundCap,
                                Qt::RoundJoin));

        for(int j = wirePointsVector.at(i).size() - 1;j >= 0;j--)
            painter.drawPoint(wirePointsVector.at(i).at(j));
    }

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    for(int j = wirePoints.size() - 1;j >= 0;j--)
        painter.drawPoint(wirePoints.at(j));

    ui->label->setPixmap(qpixmap);
}

bool ImageScissor::atImage(QPoint point)
{
    int tmpX = point.x();
    int tmpY = point.y();

    if(tmpX >= 0
      && tmpX < ui->label->pixmap()->width()
      && tmpY >= 0
      && tmpY < ui->label->pixmap()->height())
        return true;
    else
        return false;
}

//Edit Contour
void ImageScissor::selectContour()
{
    if(scribbling && moveEnable){
        selectedContour = -1;
        return;
    }
    if(wirePointsVector.isEmpty()){
        selectedContour = -1;
        return;
    }
    for(int i = 0;i < wirePointsVector.size();i++){
        for(int j = 0;j < wirePointsVector.at(i).size();j++){
            QPoint tmpPoint = wirePointsVector.at(i).at(j) - lastPoint;
            if((tmpPoint.x() * tmpPoint.x() + tmpPoint.y() * tmpPoint.y()) <= 4){
                selectedContour = i;
                return;
            }
        }
    }
    drawLineWithNode();
}

void ImageScissor::finishCurrentContour()
{
    if(seedPoints.isEmpty() || !moveEnable || !scribbling)
        return;
    lastPoint = seedPoints.first();
    addFollowingSeedPoint();

    QVector<QPoint> tmpVec;
    for(int i = 0;i < wirePoints.size();i++){
        QPoint tmpPoint = wirePoints.at(i);
        tmpVec.append(QPoint(tmpPoint.x(),tmpPoint.y()));
    }
    if(!tmpVec.isEmpty())
        wirePointsVector.append(tmpVec);

    wirePoints.clear();
    seedPoints.clear();
    scribbling = false;
    moveEnable = false;
    ui->label->setPixmap(qpixmap);
    update();
}

void ImageScissor::undo(){

    if(!scribbling && !moveEnable && selectedContour >= 0){
        wirePointsVector.remove(selectedContour);
        selectedContour = -1;
    }
    else{
        if(seedPoints.isEmpty())
            return;
        seedPoints.pop_back();
        wirePoints.pop_back();

        if(!seedPoints.isEmpty()){
            while((wirePoints.last() != seedPoints.last()  && !(wirePoints.isEmpty()))){
                wirePoints.pop_back();
                //statusBar()->showMessage(QString("%1,%2").arg(wirePoints.size()).arg(seedPoints.size()));
            }
        }

        if(seedPoints.size() == 1){
            wirePoints.pop_back();
            seedPoints.clear();
        }

        if(!seedPoints.isEmpty())
            GetPath(seedPoints.last());
    }

    qpixmap = QPixmap::fromImage(Load_Image);
    drawLineWithNode();
    statusBar()->showMessage(QString("%1,%2").arg(wirePoints.size()).arg(seedPoints.size()));
}

QImage ImageScissor::blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    int i1 = 0;
    int i2 = 3;

    if (alphaOnly)
        i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

void ImageScissor::on_actionOrigin_triggered()
{
    setImage(blur0);
}

void ImageScissor::on_actionBlur_1_2_triggered()
{
    setImage(blur2);
}

void ImageScissor::on_actionBlur_1_4_triggered()
{
    setImage(blur4);
}

void ImageScissor::on_actionBlur_1_8_triggered()
{
    setImage(blur8);
}
