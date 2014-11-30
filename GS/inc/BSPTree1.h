#ifndef _BSP_TREE1_H_
#define _BSP_TREE1_H_
#include <map>
#include "Plane.h"
#include "Polygon.h"
#include "Box3.h"
#include "BSPTree.h"

namespace GS{

class BSPTree1 {
    struct BSPTreeNode {
        Plane<double> partition;
		std::vector<Plane<double>> bplanes;

        PosRelation   relation;
        BSPTreeNode* parent;
        BSPTreeNode* left;
        BSPTreeNode* right; 
        std::vector<PlanePolygon> polygons;

        BSPTreeNode(): parent(NULL), left(NULL),right(NULL){}
        BSPTreeNode(const BSPTreeNode& other, int flag = 1);
		bool IsCell () const {return left == NULL && right == NULL;} 
	    BSPTreeNode *  copy() const;
    };
public: 
    BSPTree1();
    virtual ~BSPTree1();
    bool IsEmpty() const {return mpRoot == NULL;} 
    void BuildBSPTree(std::vector<PlanePolygon>& polygons);
    void GetPolygons(std::vector<PlanePolygon>& polys);
    void FormSubHyperPlane(const Box3& bbox);
    void Negate();
    BSPTree1* Merge(const BSPTree1* bTree, BSPTree::SET_OP op) const;
    PosRelation ClassifyPoint(const vec3<double>& p) const {return  ClassifyPoint(p, mpRoot); }
private:
    void NegateTree(BSPTreeNode* pNode);
    void DeleteChilds(BSPTreeNode* pNode);
    BSPTreeNode * Merge(const BSPTreeNode *nodeA, const BSPTreeNode *nodeB, BSPTree::SET_OP op) const;
    BSPTreeNode * MergeTreeWithCell(const BSPTreeNode* tree, const BSPTreeNode* cell, BSPTree::SET_OP op) const;
   void SplitBSPTree(const BSPTreeNode* tg, 
						   const BSPTreeNode* bp,
						   BSPTreeNode *&onNode,
						   BSPTreeNode *&leftTree, 
						   BSPTreeNode *&rightTree) const;
    void  GenSubHyperPlane(const Box3& bbox, BSPTreeNode* pNode);
    void GenNodeSubHyperPlane(const Box3& bbox, BSPTreeNode* pNode);
    void CollectPolygons(const BSPTreeNode* pNode,  std::vector<PlanePolygon>& polys);
     PosRelation ClassifyPoint(const vec3<double>& p, BSPTreeNode* pNode) const;
     void ClassifyPolygon(const BSPTreeNode* pNode,   const PlanePolygon& poly,
                                std::vector<PlanePolygon>& inPolys, std::vector<PlanePolygon>& outPolys);
    BSPTreeNode* BuildTree(std::vector<PlanePolygon>& polygons) ;
    Plane<double>  PickSplittingPlane(std::vector<PlanePolygon>& polygons) const;
    void SplitPloygonsWithPlane(std::vector<PlanePolygon>& polygons, const Plane<double>& bp, 
                                std::vector<PlanePolygon>& front, std::vector<PlanePolygon>& back, std::vector<PlanePolygon>& coincidents);
private: 
     BSPTreeNode* mpRoot;
};

}

#endif 