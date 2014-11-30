#pragma once
#include <map>
#include "FixedPlane.h"
#include "FixedPolygon.h"
#include "BSPTree.h"
#include "Box3.h"
#include <deque>
#include <fstream>

namespace GS{

struct FixedOrigPlanePolygon;
struct FixedLeafPlanePolygon;

class FixedBSPTree {
    typedef std::map<FixedPlane, int, FixedPlaneCompare> PlaneMap;
    struct FixedBSPTreeNode {
        FixedPlane partition;
		std::vector<FixedPlane> bplanes;

        PosRelation   relation;
        FixedBSPTreeNode* left;
        FixedBSPTreeNode* right; 
        std::vector<FixedPlanePolygon> polygons;

        FixedBSPTreeNode():left(NULL),right(NULL){}
		FixedBSPTreeNode(const FixedBSPTreeNode& other, int flag = 1);
        ~FixedBSPTreeNode(){if(left) delete left; if (right) delete right;}
       
		FixedBSPTreeNode *copy() const;
		bool IsCell () const {return left == NULL && right == NULL;} 
		void FormBoundary(std::vector<FixedLeafPlanePolygon*> *inPool,
						   std::vector<FixedLeafPlanePolygon*> *outPool,
						   std::vector<FixedPlanePolygon> *boundary) const;
		//void ClipPlanes(std::vector<FixedPlanePolygon> *pool, const FixedPlane &bp) const;
    };

	struct FixedBSPTreeIterator
	{
		struct Info
		{
			Info(FixedBSPTreeNode *nd, int mk):
				node(nd), mark(mk){}
			
			int mark; // the times this node has been visited.
			FixedBSPTreeNode *node;
		};

		std::deque<Info> deque;

		FixedBSPTreeIterator(FixedBSPTree *tr)
		{
			deque.push_back(Info(tr->mpRoot, 0));
		}

		FixedBSPTreeIterator(FixedBSPTreeNode *tr)
		{
			deque.push_back(Info(tr, 0));
		}
		FixedBSPTreeNode *getNext();
		int genNodeId() const;
	};

	struct FixedConstBSPTreeIterator
	{
		struct Info
		{
			Info(const FixedBSPTreeNode *nd, int mk):
				node(nd), mark(mk){}
			
			int mark; // the times this node has been visited.
			const FixedBSPTreeNode *node;
		};

		std::deque<Info> deque;

		FixedConstBSPTreeIterator(const FixedBSPTree *tr)
		{
			deque.push_back(Info(tr->mpRoot, 0));
		}

		FixedConstBSPTreeIterator(const FixedBSPTreeNode *tr)
		{
			deque.push_back(Info(tr, 0));
		}
		const FixedBSPTreeNode *getNext();
		int genNodeId() const;
	};
public:
	enum SET_OP{OP_NONE = 0, OP_UNION, OP_INTERSECT, OP_DIFFERENCE, OP_DIFFERENCE_TREE_CELL, OP_DIFFERENCE_CELL_TREE};

public: 
    FixedBSPTree();
    virtual ~FixedBSPTree();
    bool IsEmpty() const {return mpRoot == NULL;} 
    FixedBSPTree* Merge(const FixedBSPTree* bTree, SET_OP op) const;
    PosRelation ClassifyPoint(const double3& p) const {return  ClassifyPoint(p, mpRoot); }
    void BuildBSPTree(std::vector<FixedPlanePolygon>& polygons);
    void GetPolygons(std::vector<FixedPlanePolygon>& polys);
    void FormSubHyperPlane(const Box3& bbox);
    void Negate();

	void OutputDebugInfo(char*);

	friend std::ostream& operator<<(std::ostream&, const FixedBSPTreeNode&);
	friend std::ostream& operator<<(std::ostream&, FixedBSPTree&);
private : 
	void NegateTree(FixedBSPTreeNode* pNode);
    void DeleteChilds(FixedBSPTreeNode* pNode);
    void CollectPolygons(const FixedBSPTree::FixedBSPTreeNode* pNode,  std::vector<FixedPlanePolygon>& polys);
	void FormBoundary(const FixedBSPTreeNode *pNode,  std::vector<FixedPlanePolygon>& polys) const;

    PosRelation ClassifyPoint(const double3& p, FixedBSPTreeNode* pNode) const;
    FixedBSPTreeNode* BuildTree(std::vector<FixedPlanePolygon>& polygons) ;
    FixedBSPTreeNode* BuildTree(FixedPlanePolygon& polygon, PlaneMap& planes);
    FixedPlane  PickSplittingPlane(std::vector<FixedPlanePolygon>& polygons) const;
    FixedPlane  PickSplittingPlane(FixedPlanePolygon& polygons, PlaneMap& planes);
    void SplitPloygonsWithPlane(FixedPlanePolygon& polygon, const FixedPlane& bp, PlaneMap& planes,
                                FixedPlanePolygon& fronts, FixedPlanePolygon& backs, std::vector<FixedPlane >& coincidents);
    void SplitPloygonsWithPlane(std::vector<FixedPlanePolygon>& polygons, const FixedPlane& bp, 
                                std::vector<FixedPlanePolygon>& front, std::vector<FixedPlanePolygon>& back, std::vector<FixedPlanePolygon>& coincidents);
    bool HomogeneousRegion(const FixedPlanePolygon& poly, PlaneMap& planes);
	 
	FixedBSPTreeNode *Merge(const FixedBSPTreeNode *nodeA, const FixedBSPTreeNode *nodeB, SET_OP) const;
	FixedBSPTreeNode *MergeTreeWithCell(const FixedBSPTreeNode* tree, const FixedBSPTreeNode* cell, SET_OP) const;
	void SplitBSPTree(const FixedBSPTreeNode* target, const FixedBSPTreeNode* bp, const FixedPlanePolygon& bppoly, FixedBSPTreeNode *&onNode,
		FixedBSPTreeNode *&leftTree, FixedBSPTreeNode *&rightTree) const;

	//void CreatePolygon(const FixedPlane&, const Box3&, FixedPlanePolygon&) const;
private: 
     FixedBSPTreeNode* mpRoot;
};

}
