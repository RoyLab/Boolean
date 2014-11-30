#include <assert.h>
#include "BSPTree1.h"
#include "PlaneMesh.h"
#include "Polygon.h"
#include <cmath>
#include <bitset>

namespace GS{

BSPTree1::BSPTreeNode *BSPTree1::BSPTreeNode::copy() const
{
	BSPTreeNode *newNode = new BSPTreeNode(*this, 0);

	if (left)
		newNode->left = left->copy();
	
	if (right)
		newNode->right = right->copy();

	return newNode;
}

BSPTree1::BSPTreeNode::BSPTreeNode(const BSPTreeNode &other, int flag)
{
	memset(this, 0, sizeof(*this));
	if (other.IsCell())
	{
		relation = other.relation;
	}
	else
	{
		partition =		other.partition;
		bplanes	=		other.bplanes;
        parent  =       other.parent;
		if (flag == 1)
		{
			left =			other.left;
			right =			other.right;
		}
		else
		{
			left = right = nullptr;
		}

		polygons =		other.polygons;
	}
}
///////////////////////////////////////////////////////////////////////////
// BSPTree Implementation 
//////////////////////////////////////////////////////////////////////////////

BSPTree1::BSPTree1()
    :mpRoot(NULL)
{
}

BSPTree1::~BSPTree1()
{
    DeleteChilds(mpRoot);
}


void BSPTree1::DeleteChilds(BSPTreeNode* pNode)
{
    if (pNode != NULL)
    {
        DeleteChilds(pNode->left);
        DeleteChilds(pNode->right);
        delete pNode;
    }
    pNode = NULL;
      
}


void BSPTree1::BuildBSPTree( std::vector<PlanePolygon>& polygons)
{
    mpRoot = BuildTree(polygons);
   
}

PosRelation BSPTree1::ClassifyPoint(const vec3<double>& p, BSPTreeNode* pNode) const
{
    assert (pNode);
    if (pNode->IsCell())
        return pNode->relation;
    RelationToPlane ptRelation = pNode->partition.ClassifyPointToPlane(p);
    if (ptRelation == Behind)
        return  ClassifyPoint(p, pNode->right);
    if (ptRelation == Front)
        return ClassifyPoint(p, pNode->left);
    else {
        PosRelation l = ClassifyPoint(p, pNode->left);
        PosRelation r=  ClassifyPoint(p, pNode->right);
        if (l == r)
            return r; 
        return OnBoundary;
    }
}

void BSPTree1::ClassifyPolygon(const BSPTreeNode* pNode,   const PlanePolygon& poly,
                                std::vector<PlanePolygon>& inPolys, std::vector<PlanePolygon>& outPolys)
{
    assert (pNode != nullptr);
    assert(poly.bplanes.size() >= 3);
    if (pNode->IsCell())
    {
        if (pNode->relation == In)
            inPolys.push_back(poly);
        else 
            outPolys.push_back(poly);
        return ; 
    }
    PlanePolygon front;
    PlanePolygon back;
    std::vector<PlanePolygon> coincidents;
    RelationToPlane relation = poly.ClipByPlane(pNode->partition, front, back);
    switch (relation)
    {
        case On:
        { 
          //  assert(0);
            return ;
        }
        case Front:
            ClassifyPolygon(pNode->left, front, inPolys, outPolys);
            break;
        case Behind:
            ClassifyPolygon(pNode->right, back, inPolys, outPolys);
            break;
        default:
            ClassifyPolygon(pNode->left, front, inPolys, outPolys);
            ClassifyPolygon(pNode->right, back, inPolys, outPolys);
    }
}

BSPTree1::BSPTreeNode* BSPTree1::BuildTree(std::vector<PlanePolygon>& polygons)
{

    const Plane<double>& bp = PickSplittingPlane(polygons);
    BSPTreeNode* pNode = new BSPTreeNode;
    pNode->partition = bp;
    std::vector<PlanePolygon> fronts;
    std::vector<PlanePolygon> backs;
    std::vector<PlanePolygon> coincidents; 
    SplitPloygonsWithPlane(polygons, bp, fronts, backs, coincidents);
    pNode->polygons = coincidents;
    bool bSameOrient = bp.IsSimilarlyOrientation( pNode->polygons[0].splane);
    if (fronts.empty())
    {
        pNode->left = new BSPTreeNode;
        if (bSameOrient)
            pNode->left->relation = In; 
        else 
            pNode->left->relation = Out;
    }else 
        pNode->left = BuildTree(fronts);
    pNode->left->parent = pNode;
    if (backs.empty())
    {
        pNode->right = new BSPTreeNode;
        if (bSameOrient)
            pNode->right->relation = Out; 
        else 
            pNode->right->relation = In;
    }else 
        pNode->right = BuildTree(backs);
    pNode->right->parent = pNode; 
    fronts.clear();
    backs.clear();
    coincidents.clear();
    return pNode;
}
 

void BSPTree1::SplitPloygonsWithPlane( std::vector<PlanePolygon>& polygons, const Plane<double>& bp, 
                                     std::vector<PlanePolygon>& fronts, std::vector<PlanePolygon>& backs, std::vector<PlanePolygon>& coincidents)
{
    PlanePolygon front, back;
    fronts.clear();
    backs.clear();
    coincidents.clear();
    for (int i = 0 ; i < polygons.size(); i++)
    {
        front.Clear();
        back.Clear();
        RelationToPlane rp = polygons[i].ClipByPlane(bp, front, back);
        if (rp == On)
            coincidents.push_back(polygons[i]);
        else if (rp == Straddling)
        {
            fronts.push_back(front);
            backs.push_back(back);
        }else if (rp == Front)
            fronts.push_back(front);
        else 
            backs.push_back(back);
    }
}
 

Plane<double> BSPTree1::PickSplittingPlane( std::vector<PlanePolygon>& polygons) const 
{
    return polygons[0].splane;
   /* const float K = 0.8f;
    Plane<double> bestPlane;
    float bestScore = FLT_MAX;
    for (int i = 0 ; i< polygons.size(); i++)
    {
        int numFront = 0 ; 
        int numBack = 0; 
        int numStraddling = 0;
        const Plane<double >& bp = polygons[i].splane;
        for (int j = 0 ; j < polygons.size(); j++)
        {
            if (i == j )
                continue;
            switch(polygons[j].ClassifyPloygonToPlane(bp))
            {
               case On: 
               case Front:
                    numFront++;
                    break;
                case Behind:
                    numBack++;
                    break;
                case Straddling:
                    numStraddling++;
                    break;
            }
        }
        float score = K*numStraddling + (1.0 - K)*abs(numFront - numBack);
        if (score < bestScore)
        {
            bestScore = score;
            bestPlane = bp;
        }
    }
    return bestPlane;*/
}

void BSPTree1::GetPolygons(std::vector<PlanePolygon>& polys)
{
    

     CollectPolygons( mpRoot,  polys);
}

void BSPTree1::CollectPolygons(const BSPTreeNode* pNode,  std::vector<PlanePolygon>& polys)
{
    if (pNode->IsCell())
        return ; 
    std::vector<PlanePolygon> polyList;
    std::vector<PlanePolygon> nullList;
    for (int i = 0; i < pNode->polygons.size(); i++)
    {
        const PlanePolygon& poly = pNode->polygons[i];
        if (poly.splane.IsSimilarlyOrientation(pNode->partition))
        {
            ClassifyPolygon(pNode->left, poly, polyList, nullList);
            nullList.clear();
            if (polyList.size() == 0)
                continue;
            for (int i = 0; i < polyList.size(); i++)
            {
                 ClassifyPolygon(pNode->right, polyList[i], nullList, polys);
            }
          
        }
        else {
            ClassifyPolygon(pNode->right, poly, polyList, nullList);
            nullList.clear();
            if (polyList.size() == 0)
                continue;
            for (int i = 0; i < polyList.size(); i++)
            {
                 ClassifyPolygon(pNode->left, polyList[i], nullList, polys);
            }
 
        }
        nullList.clear();
        polyList.clear();
    }
    CollectPolygons(pNode->left, polys);
     CollectPolygons(pNode->right, polys);
}

void BSPTree1::FormSubHyperPlane(const Box3& bbox)
{
    GenSubHyperPlane(bbox, mpRoot);
}

void BSPTree1::GenSubHyperPlane(const Box3& bbox, BSPTreeNode* pNode)
{
    if (pNode == NULL)
        return ; 
    GenNodeSubHyperPlane(bbox, pNode);
    GenSubHyperPlane(bbox, pNode->left);
    GenSubHyperPlane(bbox, pNode->right);
}

void BSPTree1::GenNodeSubHyperPlane(const Box3& bbox, BSPTreeNode* pNode)
{
    if (pNode == NULL || pNode->IsCell() )
        return; 
    PlaneMesh mesh(bbox);
	float4 color;
    mesh.AddPolygon(pNode->partition, color, bbox); 
    PlanePolygon poly = mesh.Ploygons()[0];
    if (poly.bplanes.size()  <3)
    {
        assert(0 );
    }
    BSPTreeNode* pCurrentNode = pNode ;
    while (pCurrentNode->parent)
    {
        PlanePolygon front, back;
        front.Clear();
	    back.Clear();
        auto relation = poly.ClipByPlane( pCurrentNode->parent->partition,  front, back);
        if (relation == On)
            continue;
        if (pCurrentNode->parent->left == pCurrentNode)
        {     
            int a = front.bplanes.size();
            assert (a >=3);
            // on left child
            poly= front;
       
        }
        else {
            int b = back.bplanes.size();
            assert (b >=3);
            poly = back;
 
        }
        pCurrentNode = pCurrentNode->parent;
    }
    assert (poly.bplanes.size() >=3);
    pNode->bplanes = poly.bplanes;

 
}

void BSPTree1::Negate()
{
    NegateTree(mpRoot);
}

void BSPTree1::NegateTree(BSPTreeNode* pNode)
{
    if (pNode == NULL)
        return ; 
    if (pNode->IsCell())
    {
        if (pNode->relation == In)
            pNode->relation = Out;
        else 
            pNode->relation = In;
        return;
    }
    else
    {
		pNode->partition.Negate();
        for (int i = 0 ; i < pNode->polygons.size(); i++)
        {
            assert ( pNode->polygons[i].bplanes.size() >=3);
            pNode->polygons[i].Negate();
        }

    }

	auto ptr = pNode->left;
	pNode->left = pNode->right;
	pNode->right = ptr;

    NegateTree(pNode->left);
    NegateTree(pNode->right);
}

BSPTree1* BSPTree1::Merge(const BSPTree1* bTree, BSPTree::SET_OP op) const
{
	assert(mpRoot && bTree && bTree->mpRoot);
	
	BSPTree1 *newTree = new BSPTree1;
	assert(newTree);

	newTree->mpRoot = Merge(mpRoot, bTree->mpRoot, op);
	if (!newTree->mpRoot)
	{
		delete newTree;
		return nullptr;
	}
	//mergeFile.close();
	return newTree;
}

BSPTree1::BSPTreeNode *BSPTree1::Merge(const BSPTreeNode *nodeA, const BSPTreeNode *nodeB, BSPTree::SET_OP op) const
{
	static int counto = 0;
	counto ++;
	int count = counto;
	//mergeFile << count << "   merge****************************\n";
	if (nodeA->IsCell()) 
	{
		BSPTreeNode* res = MergeTreeWithCell(nodeB, nodeA, op);
		//mergeFile << count << " merge____________________________\n\n\n\n";
		return res;
	}
	if (nodeB->IsCell())
	{
		BSPTreeNode* res = MergeTreeWithCell(nodeA, nodeB, op);
	//	mergeFile << count << " merge____________________________\n\n\n\n";
		return res;
	}

	BSPTreeNode *newNode(nullptr);
	BSPTreeNode *splitLeftTree(nullptr), *splitRightTree(nullptr);
	SplitBSPTree(nodeA, nodeB, newNode, splitLeftTree, splitRightTree);


	//mergeFile << *splitLeftTree << std::endl;
	//mergeFile << *(nodeB->left);

	//mergeFile << count << " fen ge fu\n\n";

	//mergeFile << *splitRightTree << std::endl;
	//mergeFile << *(nodeB->right);

	newNode->left = Merge(splitLeftTree, nodeB->left, op);
	newNode->right = Merge(splitRightTree, nodeB->right, op);
	//mergeFile << *(newNode->right);
	//mergeFile << count << " fen ge fu2\n\n";

	//mergeFile << *(newNode);

	//mergeFile << count << " merge____________________________\n\n\n\n";

	return newNode;
}

// Tree and Cell are defined in the same region
BSPTree1::BSPTreeNode *BSPTree1::MergeTreeWithCell(const BSPTreeNode* tree, const BSPTreeNode* cell, BSPTree::SET_OP op) const
{
	//mergeFile << "merge with cell ......................\n";
	//mergeFile << *tree << *cell;
	if (cell->relation == In)
	{
		switch (op)
		{
		case GS::BSPTree::OP_NONE:
			return nullptr;
			break;
		case GS::BSPTree::OP_UNION:
			return cell->copy();
			break;
		case GS::BSPTree::OP_INTERSECT:
			return tree->copy();
			break;
		default:
			return nullptr;
			break;
		}
	}
	if (cell->relation == Out)
	{
		switch (op)
		{
		case GS::BSPTree::OP_NONE:
			return nullptr;
			break;
		case GS::BSPTree::OP_UNION:
			return tree->copy();
			break;
		case GS::BSPTree::OP_INTERSECT:
			return cell->copy();
			break;
		default:
			return nullptr;
			break;
		}
	}
	else throw "BSPTree: Error cell pos relation!\n";
}

// 3 output msut be initialized by nullptr.
void BSPTree1::SplitBSPTree(const BSPTreeNode* tg, 
						   const BSPTreeNode* bp,
						   BSPTreeNode *&onNode,
						   BSPTreeNode *&leftTree, 
						   BSPTreeNode *&rightTree) const
{
	if (tg->IsCell())
	{
		leftTree = new BSPTreeNode;
		leftTree->relation = tg->relation;

		rightTree = new BSPTreeNode;
		rightTree->relation = tg->relation;

		onNode = new BSPTreeNode(*bp, 0);

		return;
	}

	PlanePolygon target;
	target.splane = tg->partition;
	target.bplanes = tg->bplanes;

	PlanePolygon front, back;
	auto relation = target.ClipByPlane(bp->partition, front, back);

	switch (relation)
	{
	case GS::Behind:
		rightTree = new BSPTreeNode(*tg, 0);

		if (tg->partition.IsSimilarlyOrientation(bp->partition))
		{
			SplitBSPTree(tg->left, bp, onNode, 
				leftTree, rightTree->left);
			rightTree->right = tg->right->copy();
		}
		else
		{
			SplitBSPTree(tg->right, bp, onNode, 
				leftTree, rightTree->right);
			rightTree->left = tg->left->copy();
		}
		break;

	case GS::On:
		onNode = new BSPTreeNode(*bp, 0);
		onNode->polygons = tg->polygons;
		std::copy(bp->polygons.begin(), bp->polygons.end(),
			std::back_inserter(onNode->polygons));

		if (tg->partition.IsSimilarlyOrientation(bp->partition))
		{
			leftTree = tg->left->copy();
			rightTree = tg->right->copy();
		}
		else
		{
			leftTree = tg->right->copy();
			rightTree = tg->left->copy();
		}
		break;

	case GS::Front :
		leftTree = new BSPTreeNode(*tg, 0);

		if (tg->partition.IsSimilarlyOrientation(bp->partition))
		{
			SplitBSPTree(tg->right, bp, onNode, 
				leftTree->right, rightTree);
			leftTree->left = tg->left->copy();
		}
		else
		{
			SplitBSPTree(tg->left, bp, onNode, 
				leftTree->left, rightTree);
			leftTree->right = tg->right->copy();
		}
		break;

	case GS::Straddling:
	{
		BSPTreeNode *ll, *lr, *rl, *rr, *tmp;
		ll = lr = rl = rr = tmp = nullptr;

		onNode = new BSPTreeNode(*bp, 0);
		leftTree = new BSPTreeNode;
		rightTree = new BSPTreeNode;

		rightTree->partition = leftTree->partition = tg->partition;
		leftTree->bplanes = front.bplanes;
		rightTree->bplanes = back.bplanes;

		for (int i = 0; i < tg->polygons.size(); ++i)
		{
			front.Clear();
			back.Clear();
			tg->polygons[i].ClipByPlane(bp->partition, front, back);
			int size1 = front.bplanes.size();
			int size2 = back.bplanes.size();
			if (front.bplanes.size() > 0) leftTree->polygons.push_back(front);
			if (back.bplanes.size() > 0) rightTree->polygons.push_back(back);
		}

		SplitBSPTree(tg->left, bp, tmp, ll, lr);
		SplitBSPTree(tg->right, bp, tmp, rl, rr);

		leftTree->left = ll;
		leftTree->right = rl;

		rightTree->left = lr;
		rightTree->right = rr;

		break;
	}
	default:
		assert(0);
		break;
	}
}



}