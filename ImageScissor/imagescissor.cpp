#include "imagescissor.h"
#include "ui_imagescissor.h"

ImageScissor::ImageScissor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ImageScissor)
{
    ui->setupUi(this);
    //connect(this,SIGNAL(scale_changed(double)),ui->label,SLOT(receive_scale(double)));
    ui->label->setText("");
    ui->label->setBackgroundRole(QPalette::Base);
    ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label->setScaledContents(true);


    ui->scrollArea->setBackgroundRole((QPalette::Dark));
    //ui->scrollArea->setWidget(ui->label);
    //ui->scrollArea->setVisible(false);
    setCentralWidget(ui->scrollArea);
    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    //Scale factor
    scale_xy = 1.0;
}

ImageScissor::~ImageScissor()
{
    delete ui;
}

void ImageScissor::on_actionOpen_triggered()
{
    //Open the file
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    if(!fileName.isEmpty()){
        //Read in image
        QImage Load_Image(fileName);
        if(Load_Image.isNull()){
            QMessageBox::warning(this,"..","Failed to load image.");
            return;
        }
        //Transfer image into pixmap and show in label
        ui->label->setPixmap(QPixmap::fromImage(Load_Image));
        ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
        ui->scrollArea->setVisible(true);
        //Enable image resize
        ui->actionZoom_In->setShortcut(QKeySequence::ZoomIn);
        ui->actionZoom_In->setEnabled(true);
        ui->actionZoom_Out->setShortcut(QKeySequence::ZoomOut);
        ui->actionZoom_Out->setEnabled(true);
    }

}

void ImageScissor::scaleImage(double scaleFactor){
    scale_xy = scale_xy * scaleFactor;
    ui->label->resize(scale_xy * (ui->label->pixmap()->size()));
    //imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), scaleFactor);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), scaleFactor);

    //zoomInAct->setEnabled(scale_xy < 3.0);
    //zoomOutAct->setEnabled(scale_xy > 0.333);

}

void ImageScissor::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
     scrollBar->setValue(int(factor * scrollBar->value()+ ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageScissor::on_actionExit_triggered()
{

}

void ImageScissor::on_actionScissor_Start_triggered()
{

}

void ImageScissor::on_actionScissor_Stop_triggered()
{

}

void ImageScissor::on_actionScissor_Undo_triggered()
{

}

void ImageScissor::on_actionSave_Contour_triggered()
{

}

void ImageScissor::on_actionSave_Mask_triggered()
{

}

void ImageScissor::on_actionZoom_In_triggered()
{
    scaleImage(1.1);
    //emit scale_changed(scale_xy);
}

void ImageScissor::on_actionZoom_Out_triggered()
{
    scaleImage(0.9);
    //emit scale_changed(scale_xy);
}
