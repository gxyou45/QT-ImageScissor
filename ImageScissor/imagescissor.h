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
#include <QShortcut>

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

    void on_actionMin_Path_triggered();

    void on_actionOrigin_triggered();

    void on_actionBlur_1_2_triggered();

    void on_actionBlur_1_4_triggered();

    void on_actionBlur_1_8_triggered();

private:
    Ui::ImageScissor *ui;
    QImage Load_Image;
    int org_width;
    int org_height;

    QImage Contour_Image;
    QImage Pixel_Image;
    QImage Cost_Image;

    QImage blur0;
    QImage blur2;
    QImage blur4;
    QImage blur8;

    QPixmap qpixmap;
    QPixmap minPath_qpixmap;
    double scale_xy;
    bool work_mode;


    bool scribbling;
    bool moveEnable;
    bool minPathEnable;
    bool debugEnable;

    QPoint lastPoint;

    //std::vector<Node*> graphNode;

    ImageNode *my_shortPath;
    int my_x;
    int my_y;
    int seed_x;
    int seed_y;
    int selectedContour;

    QVector<QPoint> seedPoints;//on scissoring curve
    QVector<QPoint> wirePoints;//on scissoring curve
    QVector<QVector<QPoint>> wirePointsVector; //saved curve

    bool pathTreeMode;

    void setImage(const QImage &newImage);
    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void getPixelNode();
    void getCostGraph();
    void getPathTree();
    void getMinPath();
    void computeCost();

    void drawLineTo_example(const QPoint &endPoint);
    void drawWithPrevNode(QPoint mousePoint);
    void drawMinPath(QPoint mousePoint);
    void minPathEnable_drawMinPath(QPoint mousePoint);
    QPoint convert_position(QPoint point);
    QPoint cursorSnap(QPoint point);
    bool atImage(QPoint point);
    void GetPath(QPoint st);
    void drawLineWithNode();
    void addFollowingSeedPoint();
    void selectContour();
    void finishCurrentContour();
    void undo();
    void delay();
    QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly);

    QShortcut *finishCurrentContourSC;
    QShortcut *finishCurrentContourCloseSC;
    QShortcut *undoSC;

protected:
    //void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

};

#endif // IMAGESCISSOR_H
