#include <assert.h>
#include "FixedBSPTree.h"
#include "FixedPlaneMesh.h"
#include <cmath>
#include <bitset>
#include <list>
#include <stack>

namespace GS{

struct FixedLeafPlanePolygon:
	public FixedPlanePolygon
{
	FixedOrigPlanePolygon* parent;

	bool operator==(const FixedLeafPlanePolygon& other) const
	{
		// wr, hard to explain.
		return parent == other.parent;
	}

};

struct FixedOrigPlanePolygon:
	public FixedPlanePolygon
{
	std::list<FixedLeafPlanePolygon> subPolygon;
};


FixedBSPTree::FixedBSPTreeNode *FixedBSPTree::FixedBSPTreeNode::copy() const
{
	FixedBSPTreeNode *newNode = new FixedBSPTreeNode(*this, 0);

	if (left)
		newNode->left = left->copy();
	
	if (right)
		newNode->right = right->copy();

	return newNode;
}

FixedBSPTree::FixedBSPTreeNode::FixedBSPTreeNode(const FixedBSPTreeNode &other, int flag)
{
	left = right = nullptr;
	if (other.IsCell())
	{
		relation = other.relation;
	}
	else
	{
		partition =		other.partition;
		bplanes	=		other.bplanes;
		polygons =		other.polygons;

		if (flag == 1)
		{
			left =			other.left;
			right =			other.right;
		}
	}
}


FixedBSPTree::FixedBSPTreeNode *FixedBSPTree::FixedBSPTreeIterator::getNext()
{
	if (deque.size() == 0) return nullptr;
	++deque.back().mark;
	FixedBSPTreeNode *cur(deque.back().node);
	switch (deque.back().mark)
	{
	case 1:
		if (!cur->IsCell())
		{
			deque.push_back(Info(cur->left, 0));
			return getNext();
		}
		else
		{
			deque.back().mark = 2;
			return getNext();
		}
	case 2:
		deque.push_back(Info(cur->right, 0));
		return getNext();
	case 3:
		deque.pop_back();
		return cur;
	default:
		assert(0);
		break;
	}
	return nullptr;
}

int FixedBSPTree::FixedBSPTreeIterator::genNodeId() const
{
	unsigned int id = 1;
	for (auto itr: deque)
	{
		id <<= 1;
		if (itr.mark == 1)
		{
			id += 0;
		}
		else if (itr.mark == 2)
		{
			id += 1;
		}
		else
		{
			assert(0);
		}
	}
	return id;

}


const FixedBSPTree::FixedBSPTreeNode *FixedBSPTree::FixedConstBSPTreeIterator::getNext()
{
	if (deque.size() == 0) return nullptr;
	++deque.back().mark;
	const FixedBSPTreeNode *cur(deque.back().node);
	switch (deque.back().mark)
	{
	case 1:
		if (!cur->IsCell())
		{
			deque.push_back(Info(cur->left, 0));
			return getNext();
		}
		else
		{
			deque.back().mark = 2;
			return getNext();
		}
	case 2:
		deque.push_back(Info(cur->right, 0));
		return getNext();
	case 3:
		deque.pop_back();
		return cur;
	default:
		assert(0);
		break;
	}
	return nullptr;
}

int FixedBSPTree::FixedConstBSPTreeIterator::genNodeId() const
{
	unsigned int id = 1;
	for (auto itr: deque)
	{
		id <<= 1;
		if (itr.mark == 1)
		{
			id += 0;
		}
		else if (itr.mark == 2)
		{
			id += 1;
		}
		else
		{
			assert(0);
		}
	}
	return id;

}

///////////////////////////////////////////////////////////////////////////
// FixedBSPTree Implementation 
//////////////////////////////////////////////////////////////////////////////

FixedBSPTree::FixedBSPTree()
    :mpRoot(NULL)
{
}

FixedBSPTree::~FixedBSPTree()
{
    //DeleteChilds(mpRoot);
    if (mpRoot) delete mpRoot;
}


void FixedBSPTree::DeleteChilds(FixedBSPTreeNode* pNode)
{
    if (pNode != NULL)
    {
        DeleteChilds(pNode->left);
        DeleteChilds(pNode->right);
        delete pNode;
    }
    pNode = NULL;
      
}


void FixedBSPTree::BuildBSPTree( std::vector<FixedPlanePolygon>& polygons)
{
    if (polygons.size() > 1)
        mpRoot = BuildTree(polygons);
    else if (polygons.size()  == 1){
        PlaneMap planes;
        mpRoot = BuildTree(polygons[0], planes);
        planes.clear();
    }
}

PosRelation FixedBSPTree::ClassifyPoint(const double3& p, FixedBSPTreeNode* pNode) const
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


FixedBSPTree::FixedBSPTreeNode* FixedBSPTree::BuildTree(FixedPlanePolygon& polygon, PlaneMap& planes)
{

    const FixedPlane& bp = PickSplittingPlane(polygon, planes);
    FixedBSPTreeNode* pNode = new FixedBSPTreeNode;
    pNode->partition = bp;
    FixedPlanePolygon front;
    FixedPlanePolygon back;
    std::vector<FixedPlane> coincidents; 
    SplitPloygonsWithPlane(polygon, bp, planes, front, back, coincidents);
    for (int i = 0 ; i < coincidents.size(); i++)
    {
        FixedPlanePolygon poly;
        poly.splane = coincidents[i];
        pNode->polygons.push_back(poly);
    }
    bool bSameOrient = bp.IsSimilarlyOrientation( pNode->polygons[0].splane);
   
    if (HomogeneousRegion(front, planes ))
    {
        pNode->left = new FixedBSPTreeNode;
        if (bSameOrient)
            pNode->left->relation = In; 
        else 
            pNode->left->relation = Out;
    }else 
        pNode->left = BuildTree(front, planes);
    if (HomogeneousRegion(back, planes ))
    {
        pNode->right = new FixedBSPTreeNode;
        if (bSameOrient)
            pNode->right->relation = Out; 
        else 
            pNode->right->relation = In;
    }else 
        pNode->right = BuildTree(back,planes);
  
    coincidents.clear();
    return pNode;
}




FixedBSPTree::FixedBSPTreeNode* FixedBSPTree::BuildTree(std::vector<FixedPlanePolygon>& polygons)
{

    const FixedPlane& bp = PickSplittingPlane(polygons);
    FixedBSPTreeNode* pNode = new FixedBSPTreeNode;
    pNode->partition = bp;
    std::vector<FixedPlanePolygon> fronts;
    std::vector<FixedPlanePolygon> backs;
    std::vector<FixedPlanePolygon> coincidents; 
    SplitPloygonsWithPlane(polygons, bp, fronts, backs, coincidents);
    pNode->polygons = coincidents;
    bool bSameOrient = bp.IsSimilarlyOrientation( pNode->polygons[0].splane);
    if (fronts.empty())
    {
        pNode->left = new FixedBSPTreeNode;
        if (bSameOrient)
            pNode->left->relation = In; 
        else 
            pNode->left->relation = Out;
    }else 
        pNode->left = BuildTree(fronts);
    if (backs.empty())
    {
        pNode->right = new FixedBSPTreeNode;
        if (bSameOrient)
            pNode->right->relation = Out; 
        else 
            pNode->right->relation = In;
    }else 
        pNode->right = BuildTree(backs);
    fronts.clear();
    backs.clear();
    coincidents.clear();
    return pNode;
}



void FixedBSPTree::SplitPloygonsWithPlane( FixedPlanePolygon& polygon, const FixedPlane& bp, PlaneMap& planes,
                                      FixedPlanePolygon& front, FixedPlanePolygon& back, std::vector<FixedPlane >& coincidents)
{
     polygon.ClipByPlane(bp, front, back);
     for (int i = 0 ; i < front.bplanes.size(); i++)
     {
         if (front.bplanes[i].IsCoincidence(bp))
         {
             planes[front.bplanes[i]] ++;
             if (front.bplanes[i].IsSimilarlyOrientation(bp))
                 coincidents.push_back(front.bplanes[i]);
         }

     }
     for (int i = 0 ; i < back.bplanes.size(); i++)
     {
         if (back.bplanes[i].IsCoincidence(bp))
         {
             planes[back.bplanes[i]] ++;
             if (back.bplanes[i].IsSimilarlyOrientation(bp))
                 coincidents.push_back(back.bplanes[i]);
         }

     }

}
 

void FixedBSPTree::SplitPloygonsWithPlane( std::vector<FixedPlanePolygon>& polygons, const FixedPlane& bp, 
                                     std::vector<FixedPlanePolygon>& fronts, std::vector<FixedPlanePolygon>& backs, std::vector<FixedPlanePolygon>& coincidents)
{
    FixedPlanePolygon front, back;
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
 

FixedPlane FixedBSPTree::PickSplittingPlane( std::vector<FixedPlanePolygon>& polygons) const 
{
    return polygons[0].splane;
   /* const float K = 0.8f;
    FixedPlane bestPlane;
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

bool FixedBSPTree::HomogeneousRegion(const FixedPlanePolygon& poly, PlaneMap& planes)
{
    for (int i = 0; i < poly.bplanes.size(); i++)
    {
        if (planes.find(poly.bplanes[i]) == planes.end())
            return false ; 
    }
    return true ;
}

FixedPlane FixedBSPTree::PickSplittingPlane( FixedPlanePolygon& polygon, PlaneMap& planes)
{

    const float K = 0.8f;
    FixedPlane bestPlane;
    float bestScore = FLT_MAX;
    int nSize = polygon.bplanes.size();
    for (int i = 0 ; i< nSize; i++)
    {
        int numFront = 0 ; 
        int numBack = 0; 
        int numStraddling = 0;
        const FixedPlane& bp = polygon.bplanes[i];
        if (planes.find(bp) != planes.end())
            continue;
        for (int j = 0 ; j < nSize; j++)
        {
            if ( i == j )
               continue;
             switch(polygon.ClassifyEdgeToPlane(bp, j))
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
        
        float score = K*numStraddling + (1.0 - K)*::abs(numFront - numBack);
        if (score < bestScore)
        {
            bestScore = score;
            bestPlane = bp;
        }
    }
    return bestPlane;
}

//std::ofstream FixedmergeFile;
//unsigned int fixedcount;
//3D VERSION
FixedBSPTree* FixedBSPTree::Merge(const FixedBSPTree* bTree, SET_OP op) const
{
	//FixedmergeFile.open("D:\\mergeDebug.txt");
	assert(mpRoot && bTree && bTree->mpRoot);
	
	FixedBSPTree *newTree = new FixedBSPTree;
	assert(newTree);

	newTree->mpRoot = Merge(mpRoot, bTree->mpRoot, op);
	if (!newTree->mpRoot)
	{
		delete newTree;
		return nullptr;
	}
	//FixedmergeFile.close();
	return newTree;
}

FixedBSPTree::FixedBSPTreeNode *FixedBSPTree::Merge(const FixedBSPTreeNode *nodeA, const FixedBSPTreeNode *nodeB, SET_OP op) const
{
	//static int counto = 0;
	//counto ++;
	//int fixedcount = counto;
	//FixedmergeFile << fixedcount << "   merge****************************\n";
	if (nodeA->IsCell()) 
	{
		FixedBSPTreeNode* res = MergeTreeWithCell(nodeB, nodeA, op);
		//FixedmergeFile << fixedcount << " merge____________________________\n\n\n\n";
		return res;
	}
	if (nodeB->IsCell())
	{
		FixedBSPTreeNode* res = MergeTreeWithCell(nodeA, nodeB, op);
		//FixedmergeFile << fixedcount << " merge____________________________\n\n\n\n";
		return res;
	}

	FixedBSPTreeNode *newNode(nullptr);
	FixedBSPTreeNode *splitLeftTree(nullptr), *splitRightTree(nullptr);
	FixedPlanePolygon tmpbp, tmpfront;
	tmpbp.splane = nodeB->partition;
	tmpbp.bplanes = nodeB->bplanes;
	SplitBSPTree(nodeA, nodeB, tmpbp, newNode, splitLeftTree, splitRightTree);


	//FixedmergeFile << *splitLeftTree << std::endl;
	//FixedmergeFile << *(nodeB->left);

	//FixedmergeFile << fixedcount << " fen ge fu\n\n";

	//FixedmergeFile << *splitRightTree << std::endl;
	//FixedmergeFile << *(nodeB->right);

	newNode->left = Merge(splitLeftTree, nodeB->left, op);
	newNode->right = Merge(splitRightTree, nodeB->right, op);
    delete splitLeftTree;
    delete splitRightTree;
	//FixedmergeFile << fixedcount << " fen ge fu2\n\n";
	//FixedmergeFile << *(newNode);
	//FixedmergeFile << fixedcount << " merge____________________________\n\n\n\n";

	return newNode;
}

// Tree and Cell are defined in the same region
FixedBSPTree::FixedBSPTreeNode *FixedBSPTree::MergeTreeWithCell(const FixedBSPTreeNode* tree, const FixedBSPTreeNode* cell, SET_OP op) const
{
	//FixedmergeFile << "merge with cell ......................\n";
	//FixedmergeFile << *tree << *cell;
	if (cell->relation == In)
	{
		switch (op)
		{
		case GS::FixedBSPTree::OP_NONE:
			return nullptr;
			break;
		case GS::FixedBSPTree::OP_UNION:
			return cell->copy();
			break;
		case GS::FixedBSPTree::OP_INTERSECT:
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
		case GS::FixedBSPTree::OP_NONE:
			return nullptr;
			break;
		case GS::FixedBSPTree::OP_UNION:
			return tree->copy();
			break;
		case GS::FixedBSPTree::OP_INTERSECT:
			return cell->copy();
			break;
		default:
			return nullptr;
			break;
		}
	}
	else throw "FixedBSPTree: Error cell pos relation!\n";
}
   
std::ofstream FixedcollectFile;
void FixedBSPTree::GetPolygons(std::vector<FixedPlanePolygon>& polys)
{
	//FixedcollectFile.open("D:\\collectDebug.txt");
	std::vector<FixedLeafPlanePolygon*> nullVector;
	mpRoot->FormBoundary(&nullVector, &nullVector, &polys);
	//for (auto  itr: polys)
	//{
	//	FixedcollectFile << itr.splane;
	//	FixedcollectFile << "blpanes\n";
	//	for (auto itrj: itr.bplanes)
	//	{
	//		FixedcollectFile << itrj;
	//	}
	//	FixedcollectFile <<  "\n\n\n";
	//}

	//FixedcollectFile.close();
}

void FixedBSPTree::CollectPolygons(const FixedBSPTree::FixedBSPTreeNode* pNode,  std::vector<FixedPlanePolygon>& polys)
{
	FixedPlanePolygon poly;
	poly.splane = pNode->partition;
	poly.bplanes = pNode->bplanes;

    if (pNode == NULL ||pNode->IsCell())
	{
        return ; 
	}

    for (int i = 0 ; i < pNode->polygons.size(); i++)
    {
		if (pNode->polygons[i].bplanes.size() > 0)
		{
			polys.push_back(pNode->polygons[i]);
		}
    }
    CollectPolygons(pNode->left, polys);
    CollectPolygons(pNode->right, polys);
}

void FixedBSPTree::FixedBSPTreeNode::FormBoundary(std::vector<FixedLeafPlanePolygon*> *inPool,
						   std::vector<FixedLeafPlanePolygon*> *outPool,
						   std::vector<FixedPlanePolygon> *boundary) const
{
	if (IsCell())
	{
		if (relation == In)
		{
			for (auto itr: *outPool)
			{
				auto parent = itr->parent;
				itr->parent = nullptr;
				parent->subPolygon.remove(*itr);
			}
		}
		else
		{
			for (auto itr: *inPool)
			{
				auto parent = itr->parent;
				itr->parent = nullptr;
				parent->subPolygon.remove(*itr);
			}
		}
		return;
	}


	//clip in polygons
	std::vector<FixedOrigPlanePolygon> pool(polygons.size());
	std::vector<FixedLeafPlanePolygon*> leftInPool, rightInPool, leftOutPool, rightOutPool;
	FixedPlanePolygon leftPolygon, rightPolygon;
	for (auto itr: *inPool)
	{
		leftPolygon.Clear();
		rightPolygon.Clear();
		itr->ClipByPlane(partition, leftPolygon, rightPolygon);
		if (leftPolygon.bplanes.size() == 0) rightInPool.push_back(itr);
		else if (rightPolygon.bplanes.size() == 0) leftInPool.push_back(itr);
		else
		{
			auto &polyList = itr->parent->subPolygon;
			//itr->ClipByPlane(partition, leftPolygon, rightPolygon);

			polyList.push_back(FixedLeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = leftPolygon.bplanes;
			leftInPool.push_back(&polyList.back());

			polyList.push_back(FixedLeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = rightPolygon.bplanes;
			rightInPool.push_back(&polyList.back());

			itr->parent = nullptr;
			polyList.remove(*itr);
		}
	}

	//clip out polygons
	for (auto itr: *outPool)
	{
		leftPolygon.Clear();
		rightPolygon.Clear();
		itr->ClipByPlane(partition, leftPolygon, rightPolygon);
		if (leftPolygon.bplanes.size() == 0) rightOutPool.push_back(itr);
		else if (rightPolygon.bplanes.size() == 0) leftOutPool.push_back(itr);
		else
		{
			auto &polyList = itr->parent->subPolygon;

			polyList.push_back(FixedLeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = leftPolygon.bplanes;
			leftOutPool.push_back(&polyList.back());

			polyList.push_back(FixedLeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = rightPolygon.bplanes;
			rightOutPool.push_back(&polyList.back());

			itr->parent = nullptr;
			polyList.remove(*itr);
		}
	}

	// clip inner polygons
	for (int i = 0; i < polygons.size(); ++i)
	{
		pool[i].splane = polygons[i].splane;
		//pool[i].color = polygons[i].color;

		pool[i].subPolygon.push_back(FixedLeafPlanePolygon());
		auto leaf = &(pool[i].subPolygon.back());
		leaf->bplanes = polygons[i].bplanes;
		leaf->splane = polygons[i].splane;
		leaf->parent = &(pool[i]);

		if (polygons[i].splane.IsSimilarlyOrientation(partition))
			leftInPool.push_back(leaf);
		else leftOutPool.push_back(leaf);
	}

	// iteration
	left->FormBoundary(&leftInPool, &leftOutPool, boundary);

	// reaccumelate
	for (int i = 0; i < polygons.size(); i++)
	{
		auto orig = &(pool[i]);
		if (orig->splane.IsSimilarlyOrientation(partition))
		{
			for (auto subPolyItr = orig->subPolygon.begin();
				subPolyItr != orig->subPolygon.end(); ++subPolyItr)
			{
				rightOutPool.push_back(&(*subPolyItr));
			}
		}
		else
		{
			for (auto subPolyItr = orig->subPolygon.begin();
				subPolyItr != orig->subPolygon.end(); ++subPolyItr)
			{
				rightInPool.push_back(&(*subPolyItr));
			}
		}
	}

	// iteration right
	right->FormBoundary(&rightInPool, &rightOutPool, boundary);

	for (int i = 0; i < pool.size(); i++)
	{
		if (pool[i].subPolygon.size() == 0) continue;

		for (auto &bItr: pool[i].subPolygon)
		{
		//	bItr.color = bItr.parent->color;
			boundary->push_back(bItr);
			//decltype(bItr.bplanes) rev;
			//auto &norm = boundary->back().bplanes;
			//while (!norm.empty())
			//{
			//	rev.push_back(norm.back());
			//	norm.pop_back();
			//}
			//norm.swap(rev);
		}
	}
}


void FixedBSPTree::FormSubHyperPlane(const Box3& bbox)
{
	FixedBSPTreeIterator itr(this);
	FixedPlanePolygon oPoly;
	auto &minEdge = bbox.Min(), &maxEdge = bbox.Max();
	FixedPlane boxPlanes[] = 
	{
		FixedPlane(double3(1,0,0), -minEdge.x), 
		FixedPlane(double3(-1,0,0), maxEdge.x), 
		FixedPlane(double3(0,1,0), -minEdge.y), 
		FixedPlane(double3(0,-1,0), maxEdge.y), 
		FixedPlane(double3(0,0,1), -minEdge.z), 
		FixedPlane(double3(0,0,-1), maxEdge.z), 
	};	//bbox: x+ x- y+ y- z+ z-
	
	while (true)
	{
		auto node = itr.getNext();
		if (!node) break;
		if (node->IsCell()) continue;

		oPoly.bplanes.clear();
		FixedPlanePolygon::CreatePolygon(node->partition, boxPlanes, oPoly);
		FixedPlanePolygon front, back;

		for (auto qItr: itr.deque)
		{
			front.Clear();
			back.Clear();
			auto relation = oPoly.ClipByPlane(
				qItr.node->partition, front, back);
			switch (qItr.mark)
			{
			case 1: // left child
				oPoly = front;
				break;
			case 2: // right child
				oPoly = back;
				break;
			default:
				assert(0);
				break;
			}
		}
		node->bplanes = oPoly.bplanes;
	}	
}

// 3 output msut be initialized by nullptr.
void FixedBSPTree::SplitBSPTree(const FixedBSPTreeNode* tg, 
						   const FixedBSPTreeNode* bp, const FixedPlanePolygon& bppoly,
						   FixedBSPTreeNode *&onNode,
						   FixedBSPTreeNode *&leftTree, 
						   FixedBSPTreeNode *&rightTree) const
{
	if (tg->IsCell())
	{
		leftTree = new FixedBSPTreeNode;
		leftTree->relation = tg->relation;

		rightTree = new FixedBSPTreeNode;
		rightTree->relation = tg->relation;

		onNode = new FixedBSPTreeNode(*bp, 0);

		return;
	}

	FixedPlanePolygon target;
	target.splane = tg->partition;
	target.bplanes = tg->bplanes;

	FixedPlanePolygon front, back, bpfront, bpback;
	auto relation = target.ClipByPlane(bp->partition, front, back);
	auto bprelation = bppoly.ClipByPlane(tg->partition, bpfront, bpback); 

	switch (relation)
	{
	case GS::Behind:
		{
		rightTree = new FixedBSPTreeNode(*tg, 0);

		assert(bprelation != On && bprelation != Straddling);
		//if (relation == Straddling)
		//{
		//	target.ClipByPlane(tg->partition, front, back);
		//}

		if (bprelation == Front)
		{
			SplitBSPTree(tg->left, bp, bpfront, onNode, 
				leftTree, rightTree->left);
			rightTree->right = tg->right->copy();
		}
		else
		{
			SplitBSPTree(tg->right, bp, bpback, onNode, 
				leftTree, rightTree->right);
			rightTree->left = tg->left->copy();
		}
		break;
		}
	case GS::On:
		onNode = new FixedBSPTreeNode(*bp, 0);
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

	case GS::Front:
		{
		leftTree = new FixedBSPTreeNode(*tg, 0);

		assert(bprelation != On && bprelation != Straddling);
		if (bprelation == Behind)
		{
			SplitBSPTree(tg->right, bp, bpback, onNode, 
				leftTree->right, rightTree);
			leftTree->left = tg->left->copy();
		}
		else
		{
			SplitBSPTree(tg->left, bp, bpfront, onNode, 
				leftTree->left, rightTree);
			leftTree->right = tg->right->copy();
		}
		break;
		}
	case GS::Straddling:
	{
		FixedBSPTreeNode *ll, *lr, *rl, *rr, *tmp;
		ll = lr = rl = rr = tmp = nullptr;

		onNode = new FixedBSPTreeNode(*bp, 0);
		leftTree = new FixedBSPTreeNode;
		rightTree = new FixedBSPTreeNode;

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

		SplitBSPTree(tg->left, bp, bpfront, tmp, ll, lr); delete tmp;
		SplitBSPTree(tg->right, bp, bpback, tmp, rl, rr); delete tmp;

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


void FixedBSPTree::Negate()
{
    NegateTree(mpRoot);
}

void FixedBSPTree::NegateTree(FixedBSPTreeNode* pNode)
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
            pNode->polygons[i].Negate();
        }

    }

	auto ptr = pNode->left;
	pNode->left = pNode->right;
	pNode->right = ptr;

    NegateTree(pNode->left);
    NegateTree(pNode->right);
}

void FixedBSPTree::OutputDebugInfo(char* filename)
{
	auto out = std::ofstream(filename);
	out << (*this);
	out.close();
}

std::ostream& operator<<(std::ostream& out, FixedBSPTree& tree)
{
	out << *(tree.mpRoot);
	return out;
}

std::ostream& operator<<(std::ostream& out, const FixedBSPTree::FixedBSPTreeNode& node)
{
	FixedBSPTree::FixedConstBSPTreeIterator itr(&node);
	out << "///////////////////////////////////////////////////\n";
	out << "tree:\n";
	//out << "tree: " << ++fixedcount << std::endl;
	while (true)
	{
		auto cur = itr.getNext();
		if (!cur) break;
		
		int id = itr.genNodeId();
		out << "Node: " <<  std::bitset<sizeof(int)*8>(id) << std::endl;
		if (cur->IsCell())
		{
			out << "It is a Cell: " << ((cur->relation == In)?"IN\n":"OUT\n");
		}
		else
		{
			out << "x: " << cur->partition.Normal().x << std::endl;
			out << "y: " << cur->partition.Normal().y << std::endl;
			out << "z: " << cur->partition.Normal().z << std::endl;
			out << "d: " << cur->partition.Distance() << std::endl;
			out << "boundingSize: " << cur->bplanes.size() << std::endl;
			for (int i = 0; i < cur->bplanes.size(); ++i)
			{
				out << "x: " << cur->bplanes[i].Normal().x << "\t";
				out << "y: " << cur->bplanes[i].Normal().y << "\t";
				out << "z: " << cur->bplanes[i].Normal().z << "\t";
				out << "d: " << cur->bplanes[i].Distance() << "\n";
			}
			out << std::endl;
		}
		out << std::endl;
	}
	return out;
}

}