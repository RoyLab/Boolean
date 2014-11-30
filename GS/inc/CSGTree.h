#ifndef _CSG_TREE_H_
#define _CSG_TREE_H_
#include <vector>
#include <map>
#include <hash_map>
#include <assert.h>
#include "CSGExprNode.h"
#include "CSGMesh.h"
#include "OctTree.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

namespace GS{

 
typedef std::map<CSGRecMesh*, bool> RecMeshDict;

const int TMR_UNKNOWN  =  0;
const int TMR_INSIDE   =  1;
const int TMR_OUTSIDE  =  2;
const int TMR_SAME     =  4;
const int TMR_OPPOSITE =  8;

const int UnionRelationTable[4][4] = {
  // {TMR_INSIDE, TMR_OUTSIDE,  TMR_SAME,   TMR_OPPOSITE}  right
	 {TMR_INSIDE, TMR_INSIDE,   TMR_INSIDE, TMR_INSIDE  },// left = TMR_INSIDE
	 {TMR_INSIDE, TMR_OUTSIDE,  TMR_SAME,   TMR_OPPOSITE},// left = TMR_OUTSIDE
	 {TMR_INSIDE, TMR_SAME,     TMR_SAME,   TMR_INSIDE  },// left = TMR_SAME
	 {TMR_INSIDE, TMR_OPPOSITE, TMR_INSIDE, TMR_OPPOSITE}// left = TMR_OPPOSITE
};


const int IntersectRelationTable[4][4] = {
  // {TMR_INSIDE,   TMR_OUTSIDE,  TMR_SAME,    TMR_OPPOSITE}  right
	 {TMR_INSIDE,   TMR_OUTSIDE,  TMR_SAME,    TMR_OPPOSITE },// left = TMR_INSIDE
	 {TMR_OUTSIDE,  TMR_OUTSIDE,  TMR_OUTSIDE, TMR_OUTSIDE  },// left = TMR_OUTSIDE
	 {TMR_SAME,     TMR_OUTSIDE,  TMR_SAME,    TMR_OUTSIDE  },// left = TMR_SAME
	 {TMR_OPPOSITE, TMR_OUTSIDE,  TMR_OUTSIDE, TMR_OPPOSITE}// left = TMR_OPPOSITE
};

const int DiffRelationTable[4][4] = {
  // {TMR_INSIDE,   TMR_OUTSIDE,  TMR_SAME,    TMR_OPPOSITE}  right
	 {TMR_OUTSIDE,   TMR_INSIDE,  TMR_OPPOSITE,  TMR_SAME },// left = TMR_INSIDE
	 {TMR_OUTSIDE,  TMR_OUTSIDE,  TMR_OUTSIDE,   TMR_OUTSIDE  },// left = TMR_OUTSIDE
	 {TMR_OUTSIDE,     TMR_SAME,  TMR_OUTSIDE,   TMR_SAME  },// left = TMR_SAME
	 {TMR_OUTSIDE, TMR_OPPOSITE,  TMR_OPPOSITE,  TMR_OUTSIDE}// left = TMR_OPPOSITE
};


struct CSGGroupInfo{
    std::vector<int> TriIds;
    bool      bUnified;
	bool      bReverse;
	CSGMesh*  pMesh;

	CSGGroupInfo(CSGMesh* mesh, bool unified, bool reverse) 
		: pMesh(mesh)
        , bUnified(unified)
		, bReverse(reverse)
	{

	}
    CSGGroupInfo()
        :pMesh(nullptr)
        ,bUnified(false)
        ,bReverse(false)
    {
    }

    int Size() const {return TriIds.size();}
};

class CSGOpNode;
class CSGUnionNode;
class CSGDiffNode;
class CSGIntersectNode;
class CSGTreeNode {

public:
    CSGTreeNode();
	virtual       ~CSGTreeNode(); 
	virtual void   Solve() = 0; 
	virtual void   GetAllMesh(RecMeshDict& CSGRecMeshDict)= 0;
	virtual bool   RelationTest(CSGMesh* pMesh, int TriId, bool bReverse , int Relations);
            int    RelationWith(CSGMesh* pMesh, int TriId, bool bReverse );
         CSGMesh*  CreateResultMesh();
protected:
	virtual int DoRelationWith(CSGMesh* pMesh, int TriId, bool bReverse) = 0;

protected:
	std::vector<CSGGroupInfo> mInfoList;
    BaseMesh* mpNoneCriticalMesh;
	friend CSGOpNode;
    friend CSGUnionNode;
    friend CSGDiffNode;
    friend CSGIntersectNode;
};

class CSGLeafNode : public CSGTreeNode {

public:
    CSGLeafNode(CSGExprNode*  node,  std::hash_map<int, CSGRecMesh*>& recMeshDict, OctTree* pTree =nullptr);
	virtual ~CSGLeafNode();
	virtual void Solve();
	virtual void GetAllMesh(RecMeshDict& CSGRecMeshDict);
protected:
	virtual int DoRelationWith(CSGMesh* pMesh, int TriId, bool bReverse);
protected:
	CSGRecMesh* mpRecMesh;
    OctTree*  mpTree;
};

class CSGOpNode: public CSGTreeNode{
private:
  
     class LeftRelationTest{
            CSGOpNode*  mpOpNode;
            int         mGroupID;
        public:
            CSGGroupInfo mNewInfo;
            LeftRelationTest( CSGOpNode* pNode, int groupId)
                : mpOpNode(pNode)
                , mGroupID(groupId)
            {}
             LeftRelationTest( LeftRelationTest& left, tbb::split)
                : mpOpNode(left.mpOpNode)
                , mGroupID(left.mGroupID) {}
            void operator() (const tbb::blocked_range<int>& r) ;
             void join( const LeftRelationTest& left );  
    };
     class RightRelationTest{
            CSGOpNode*  mpOpNode;
            int         mGroupID;
        public:
           CSGGroupInfo mNewInfo;
            RightRelationTest( CSGOpNode* pNode, int groupId)
                : mpOpNode(pNode)
                , mGroupID(groupId){}
             RightRelationTest( RightRelationTest& right, tbb::split)
                  : mpOpNode(right.mpOpNode)
                , mGroupID(right.mGroupID) {}

            void operator() (const tbb::blocked_range<int>& r);
              void join( const RightRelationTest& right );  
     };
public:
	CSGOpNode(const int (&relation)[4][4],CSGTreeNode* leftNode, CSGTreeNode* rightNode);
	virtual ~CSGOpNode(); 
	virtual void Solve();
	virtual void GetAllMesh(RecMeshDict& CSGRecMeshDict)
	{
		mpLeftNode->GetAllMesh(CSGRecMeshDict);
		mpRightNode->GetAllMesh(CSGRecMeshDict);
	}
protected:
	virtual int  LeftRelations() = 0;
	virtual int  RightRelations() = 0;
	virtual bool ReverseRight() {return false;}
    virtual int DoRelationWith(CSGMesh* pMesh, int TriId, bool bReverse);
private:
	int GetTableIndex(const int relation)
	{
		switch(relation)
		{
			case TMR_INSIDE  : return 0;
			case TMR_OUTSIDE : return 1;
			case TMR_SAME    : return 2;
			case TMR_OPPOSITE: return 3;
			default:  assert(0);
		}
	}
	



protected:
	CSGTreeNode* mpLeftNode;
	CSGTreeNode* mpRightNode;
private:
	const int (&mRelation)[4][4];
};


class CSGUnionNode: public CSGOpNode{
public :
	CSGUnionNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode);
	virtual ~CSGUnionNode() {}
    virtual void Solve(); 
protected : 
	virtual int  LeftRelations() ;
	virtual int  RightRelations() ;
};

class CSGIntersectNode: public CSGOpNode{
public :
	CSGIntersectNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode);
	virtual ~CSGIntersectNode() {}
    virtual void Solve(); 
protected : 
	virtual int  LeftRelations() ;
	virtual int  RightRelations() ;
};

class CSGDiffNode: public CSGOpNode{
public :
	CSGDiffNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode);
	virtual ~CSGDiffNode() {}
    virtual void Solve(); 
protected : 
	virtual bool ReverseRight() {return true;}
	virtual int  LeftRelations() ;
	virtual int  RightRelations() ;
};

class CSGTree{

public:
    CSGTree()
        :mpTree(nullptr)
    {
    }
	~CSGTree();
	void AddExpr(CSGExprNode* node );
    void Evaluate();
    void Evaluate1();
	void Clear();
    CSGMesh* GetResultCopy(CSGExprNode* node );
protected:
	void BuildTrees();
    CSGTreeNode* DoBuild(CSGExprNode* node);
protected:
	std::map<CSGExprNode*, CSGTreeNode*> mRoots;
    std::hash_map<int, CSGRecMesh*> mCSGRecMesh;
    OctTree*   mpTree;

};

}
#endif 