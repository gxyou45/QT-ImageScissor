#include "imagenode.h"
#include <iostream>
//#include <fibheap_pro.h>
//using namespace std;

ImageNode::ImageNode()
{

}

struct greaterNode
{
    bool operator() (Node *a, Node *b)
    {
        return a->totalCost > b->totalCost;
    }
};

void ImageNode::GetPath(int st, std::vector<Node*> &nodes, int w, int h)
{
    //starting point
    nodes[st]->totalCost = 0;
    std::priority_queue<Node*, std::vector<Node*>, greaterNode> pq;
    pq.push(nodes[st]);

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
            }
            else if(new_state == 1){
                int current_cost = nodes[idx]->totalCost+nodes[idx]->linkCost[i];
                if(current_cost < nodes[new_idx]->totalCost){
                    nodes[new_idx]->prevNode = nodes[idx];
                    nodes[new_idx]->totalCost = current_cost;
                }
            }
        }
    }
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
