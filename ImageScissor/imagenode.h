#ifndef IMAGENODE_H
#define IMAGENODE_H

#include <QVector>
#include <QPoint>
#include <vector>
#include <queue>

const double MAX = std::numeric_limits<double>::infinity();

struct Node{
    double linkCost[8];
    double rgbCost[8];
    int state; //0-initial, 1-active, 2-expanded
    double totalCost;
    double maxDeriv;
    Node *prevNode; //predecessor
    int column;
    int row; //position of the node in the image

    Node (){
        prevNode = NULL;
        totalCost = MAX;
        state = 0;
    }

    void graph(int &off_x, int &off_y, int idx){
        switch (idx)
        {
            case 0:
                off_x = 1;
                off_y = 0;
                break;
            case 1:
                off_x = 1;
                off_y = -1;
                break;
            case 2:
                off_x = 0;
                off_y = -1;
                break;
            case 3:
                off_x = -1;
                off_y = -1;
                break;
            case 4:
                off_x = -1;
                off_y = 0;
                break;
            case 5:
                off_x = -1;
                off_y = 1;
                break;
            case 6:
                off_x = 0;
                off_y = 1;
                break;
            case 7:
                off_x = 1;
                off_y = 1;
                break;
            default:
                break;
        }
    }
};

class ImageNode
{
public:
    ImageNode();
    void GetPath(int st, std::vector<Node*> &nodes, int w, int h);
    QVector<QPoint> DrawPath(int x, int y, std::vector<Node*> &nodes, int w);
};

#endif // IMAGENODE_H
