#ifndef IMAGESCISSOR_H
#define IMAGESCISSOR_H

#include <QMainWindow>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <QScrollBar>
#include <QPixmap>
#include <QPalette>
#include <QSizePolicy>
#include <QFileDialog>
#include <QImage>
#include <QKeySequence>

namespace Ui {
class ImageScissor;
}

class ImageScissor : public QMainWindow
{
    Q_OBJECT

public:
    QImage Load_Image;
    QImage Cut_Image;
    int my_x, my_y;
    double scale_xy;
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

private:
    Ui::ImageScissor *ui;

    void scaleImage(double scaleFactor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
};

#endif // IMAGESCISSOR_H
