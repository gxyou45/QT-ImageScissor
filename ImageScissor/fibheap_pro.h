#ifndef FIBHEAP_PRO_H
#define FIBHEAP_PRO_H

#define OK      0
#define NOTOK   -1


class FibHeapNode
{
friend class FibHeap;

     FibHeapNode *Left, *Right, *Parent, *Child;
     short Degree, Mark, NegInfinityFlag;

protected:
     int  FHN_Cmp(FibHeapNode& RHS);
     void FHN_Assign(FibHeapNode& RHS);

public:
     FibHeapNode();
     virtual ~FibHeapNode();
     virtual void operator =(FibHeapNode& RHS);
     virtual int  operator ==(FibHeapNode& RHS);
     virtual int  operator <(FibHeapNode& RHS);

     virtual void Print();
};


//========================================================================
// Fibonacci Heap Class
//========================================================================

class FibHeap
{
     FibHeapNode *MinRoot;
     long NumNodes, NumTrees, NumMarkedNodes;

     int HeapOwnershipFlag;

public:

     FibHeap();
     virtual ~FibHeap();

// The Standard Heap Operations

     void Insert(FibHeapNode *NewNode);
     void Union(FibHeap *OtherHeap);

     inline FibHeapNode *Minimum();
     FibHeapNode *ExtractMin();

     int DecreaseKey(FibHeapNode *theNode, FibHeapNode& NewKey);
     int Delete(FibHeapNode *theNode);

    // Extra utility functions


     int  GetHeapOwnership() {
         return HeapOwnershipFlag;
     };

     void SetHeapOwnership() {
         HeapOwnershipFlag = 1;
     };

     void ClearHeapOwnership() {
         HeapOwnershipFlag = 0;
     };

     long GetNumNodes() {
         return NumNodes;
     };

     long GetNumTrees() {
         return NumTrees;
     };

     long GetNumMarkedNodes() {
         return NumMarkedNodes;
     };


     //void Print(FibHeapNode *Tree = NULL, FibHeapNode *theParent=NULL);

private:

// Internal functions that help to implement the Standard Operations

     inline void _Exchange(FibHeapNode*&, FibHeapNode*&);
     void _Consolidate();
     void _Link(FibHeapNode *, FibHeapNode *);
     void _AddToRootList(FibHeapNode *);
     void _Cut(FibHeapNode *, FibHeapNode *);
     void _CascadingCut(FibHeapNode *);
};


#endif // FIBHEAP_PRO_H
