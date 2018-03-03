#include "imagenode.h"
#include <iostream>
//#include <fibheap_pro.h>
//using namespace std;

ImageNode::ImageNode()
{

}

QVector<QPoint> ImageNode::DrawPath(int x, int y, std::vector<Node*> &nodes, int w)
{
    QVector<QPoint> draw_list;
    int idx = y * w + x;
    QPoint tmp(x, y);
    draw_list.append(tmp);
    while(nodes[idx]->prevNode != NULL){
        Node *next = nodes[idx]->prevNode;
        int new_x = next->column;
        int new_y = next->row;
        tmp.setX(new_x);
        tmp.setY(new_y);
        draw_list.append(tmp);
        idx = new_y * w + new_x;
    }
    return draw_list;
}
