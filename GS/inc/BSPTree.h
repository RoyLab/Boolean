#ifndef _BSP_TREE_H_
#define _BSP_TREE_H_
#include <map>
#include "Plane.h"
#include "Polygon.h"
#include "Box3.h"
#include <deque>
#include <fstream>

namespace GS{

enum PosRelation{
    In, 
    Out,
    OnBoundary
};
struct OrigPlanePolygon;
struct LeafPlanePolygon;

class BSPTree {
    typedef std::map<Plane<double>, int, PlaneCompare<double> > PlaneMap;
    struct BSPTreeNode {
        Plane<double> partition;
		std::vector<Plane<double>> bplanes;

        PosRelation   relation;
        BSPTreeNode* left;
        BSPTreeNode* right; 
        std::vector<PlanePolygon> polygons;

        BSPTreeNode():left(NULL),right(NULL){}
		BSPTreeNode(const BSPTreeNode& other, int flag = 1);
       
		BSPTreeNode *copy() const;
		bool IsCell () const {return left == NULL && right == NULL;} 
		void FormBoundary(std::vector<LeafPlanePolygon*> *inPool,
						   std::vector<LeafPlanePolygon*> *outPool,
						   std::vector<PlanePolygon> *boundary) const;
		//void ClipPlanes(std::vector<PlanePolygon> *pool, const Plane<double> &bp) const;
    };

	struct BSPTreeIterator
	{
		struct Info
		{
			Info(BSPTreeNode *nd, int mk):
				node(nd), mark(mk){}
			
			int mark; // the times this node has been visited.
			BSPTreeNode *node;
		};

		std::deque<Info> deque;

		BSPTreeIterator(BSPTree *tr)
		{
			deque.push_back(Info(tr->mpRoot, 0));
		}

		BSPTreeIterator(BSPTreeNode *tr)
		{
			deque.push_back(Info(tr, 0));
		}
		BSPTreeNode *getNext();
		int genNodeId() const;
	};

	struct ConstBSPTreeIterator
	{
		struct Info
		{
			Info(const BSPTreeNode *nd, int mk):
				node(nd), mark(mk){}
			
			int mark; // the times this node has been visited.
			const BSPTreeNode *node;
		};

		std::deque<Info> deque;

		ConstBSPTreeIterator(const BSPTree *tr)
		{
			deque.push_back(Info(tr->mpRoot, 0));
		}

		ConstBSPTreeIterator(const BSPTreeNode *tr)
		{
			deque.push_back(Info(tr, 0));
		}
		const BSPTreeNode *getNext();
		int genNodeId() const;
	};
public:
	enum SET_OP{OP_NONE = 0, OP_UNION, OP_INTERSECT, OP_DIFFERENCE, OP_DIFFERENCE_TREE_CELL, OP_DIFFERENCE_CELL_TREE};

public: 
    BSPTree();
    virtual ~BSPTree();
    bool IsEmpty() const {return mpRoot == NULL;} 
    BSPTree* Merge(const BSPTree* bTree, SET_OP op) const;
    PosRelation ClassifyPoint(const vec3<double>& p) const {return  ClassifyPoint(p, mpRoot); }
    void BuildBSPTree(std::vector<PlanePolygon>& polygons);
    void GetPolygons(std::vector<PlanePolygon>& polys);
    void FormSubHyperPlane(const Box3& bbox);
    void Negate();

	void OutputDebugInfo(char*);

	friend std::ostream& operator<<(std::ostream&, const BSPTreeNode&);
	friend std::ostream& operator<<(std::ostream&, BSPTree&);
private : 
	void NegateTree(BSPTreeNode* pNode);
    void DeleteChilds(BSPTreeNode* pNode);
    void CollectPolygons(const BSPTree::BSPTreeNode* pNode,  std::vector<PlanePolygon>& polys);
	void FormBoundary(const BSPTreeNode *pNode,  std::vector<PlanePolygon>& polys) const;

    PosRelation ClassifyPoint(const vec3<double>& p, BSPTreeNode* pNode) const;
    BSPTreeNode* BuildTree(std::vector<PlanePolygon>& polygons) ;
    BSPTreeNode* BuildTree(PlanePolygon& polygon, PlaneMap& planes);
    Plane<double>  PickSplittingPlane(std::vector<PlanePolygon>& polygons) const;
    Plane<double>  PickSplittingPlane(PlanePolygon& polygons, PlaneMap& planes);
    void SplitPloygonsWithPlane(PlanePolygon& polygon, const Plane<double>& bp, PlaneMap& planes,
                                PlanePolygon& fronts, PlanePolygon& backs, std::vector<Plane<double> >& coincidents);
    void SplitPloygonsWithPlane(std::vector<PlanePolygon>& polygons, const Plane<double>& bp, 
                                std::vector<PlanePolygon>& front, std::vector<PlanePolygon>& back, std::vector<PlanePolygon>& coincidents);
    bool HomogeneousRegion(const PlanePolygon& poly, PlaneMap& planes);
	 
	BSPTreeNode *Merge(const BSPTreeNode *nodeA, const BSPTreeNode *nodeB, SET_OP) const;
	BSPTreeNode *MergeTreeWithCell(const BSPTreeNode* tree, const BSPTreeNode* cell, SET_OP) const;
	void SplitBSPTree(const BSPTreeNode* target, const BSPTreeNode* bp, BSPTreeNode *&onNode,
		BSPTreeNode *&leftTree, BSPTreeNode *&rightTree) const;

	PlanePolygon CreatePolygon(const Plane<double>&, const Box3&) const;
    void  ClassifyPolygon(const BSPTree::BSPTreeNode* pNode, const PlanePolygon& poly, std::vector<PlanePolygon>& inPolys, std::vector<PlanePolygon>& outPolys); 
private: 
     BSPTreeNode* mpRoot;
};

}


#endif 