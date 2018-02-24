#ifndef IMAGESCISSOR_H
#define IMAGESCISSOR_H

#include <QMainWindow>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <QScrollBar>
#include <QPixmap>
//#include <QPainter>
#include <QPalette>
#include <QSizePolicy>
#include <QFileDialog>
#include <QImage>
#include <QKeySequence>
#include <QSize>
#include <imagenode.h>
#include <opencv2/opencv.hpp>
//#include <QGridLayout>
#include <math.h>
#include <QtGui>

namespace Ui {
class ImageScissor;
}

class ImageScissor : public QMainWindow
{
    Q_OBJECT

public:
    //imagelabel *my_label;

    //double scale;
    explicit ImageScissor(QWidget *parent = 0);
    ~ImageScissor();

private slots:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionScissor_Start_triggered();
    void on_actionScissor_Stop_triggered();
    void on_actionScissor_Undo_triggered();
    void on_actionSave_Contour_triggered();
    void on_actionSave_Mask_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();

    void on_actionPixel_Node_triggered();

    void on_actionImage_Only_triggered();

    void on_actionCost_Graph_triggered();

    void on_actionPath_Tree_triggered();

private:
    Ui::ImageScissor *ui;
    QImage Load_Image;
    int org_width;
    int org_height;
    QImage Contour_Image;
    QImage Pixel_Image;
    QImage Cost_Image;
    QPixmap qpixmap;
    double scale_xy;
    bool work_mode;

    //testing
    bool scribbling;
    int myPenWidth;
    QColor myPenColor;
    QPoint lastPoint;

    std::vector<QColor> rgbImage;
    std::vector<Node*> graphNode;

    ImageNode *my_shortPath;
    int my_x;
    int my_y;
    int seed_x;
    int seed_y;
    QVector<QPoint> seeds;
    QVector<QPoint> wirepoints;

    bool pathTreeMode;

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void getPixelNode();
    void getCostGraph();
    void getPathTree(int x, int y);
    void getMinPath();
    void computeCost(QImage image);

    void setPenColor(const QColor &newcolor);
    void setPenWidth(int newWidth);
    void drawLineTo_example(const QPoint &endPoint);
    void drawWithPrevNode(Node* node);
    QPoint convert_position(QPoint point);
    void GetPath(int st, std::vector<Node*> &nodes, int w, int h);

protected:
    //void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // IMAGESCISSOR_H
