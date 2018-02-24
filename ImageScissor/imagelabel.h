#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QMouseEvent>
//#include <QPaintEvent>
#include <QPainter>
#include <QtGui>
#include <QtCore>
#include <imagenode.h>
#include <vector>
#include <opencv2/opencv.hpp>


class imagelabel:public QLabel{
    Q_OBJECT
public:
    int my_x;
    int my_y;
    int seed_x;
    int seed_y;
    double scaleFactor;
    QImage Paint_Image;
    QPixmap qpixmap;
    QPixmap current_pixmap;
    std::vector<QColor> rgbImage;
    std::vector<Node*> graphNode;
    QVector<QPoint> seeds;
    QVector<QPoint> wirepoints;

    explicit imagelabel(QWidget *parent = 0);
    ~imagelabel();
    bool openImage(const QString &filename);
    void scaleImage(double scaleFactor);
    void updateScaleFactor(double f);
    void setPenColor(const QColor &newcolor);
    void setPenWidth(int newWidth);
    void drawPixelNode();
    void drawCostGraph();
    void drawPathTree();
    void drawMinPath();


private:
    //test var and func
    bool scribbling;
    int myPenWidth;
    QColor myPenColor;
    QPoint lastPoint;
    void drawLineTo_example(const QPoint &endPoint);
    //***************************
    void drawLineTo(const QPoint &endPoint, std::vector<Node*> &nodes, int width);
    //void computeCost(QImage &image);
    QImage mat_to_qimage_ref(cv::Mat &mat, QImage::Format format);
    cv::Mat qimage_to_mat_ref(QImage &img, int format);
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // IMAGELABEL_H
