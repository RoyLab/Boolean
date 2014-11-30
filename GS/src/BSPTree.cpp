#include <assert.h>
#include "BSPTree.h"
#include "PlaneMesh.h"
#include <cmath>
#include <bitset>
#include <list>
#include <stack>

namespace GS{

struct LeafPlanePolygon:
	public PlanePolygon
{
	OrigPlanePolygon* parent;

	bool operator==(const LeafPlanePolygon& other) const
	{
		// wr, hard to explain.
		return parent == other.parent;
	}

};

struct OrigPlanePolygon:
	public PlanePolygon
{
	std::list<LeafPlanePolygon> subPolygon;
};


BSPTree::BSPTreeNode *BSPTree::BSPTreeNode::copy() const
{
	BSPTreeNode *newNode = new BSPTreeNode(*this, 0);

	if (left)
		newNode->left = left->copy();
	
	if (right)
		newNode->right = right->copy();

	return newNode;
}

BSPTree::BSPTreeNode::BSPTreeNode(const BSPTreeNode &other, int flag)
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


BSPTree::BSPTreeNode *BSPTree::BSPTreeIterator::getNext()
{
	if (deque.size() == 0) return nullptr;
	++deque.back().mark;
	BSPTreeNode *cur(deque.back().node);
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

int BSPTree::BSPTreeIterator::genNodeId() const
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


const BSPTree::BSPTreeNode *BSPTree::ConstBSPTreeIterator::getNext()
{
	if (deque.size() == 0) return nullptr;
	++deque.back().mark;
	const BSPTreeNode *cur(deque.back().node);
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

int BSPTree::ConstBSPTreeIterator::genNodeId() const
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
// BSPTree Implementation 
//////////////////////////////////////////////////////////////////////////////

BSPTree::BSPTree()
    :mpRoot(NULL)
{
}

BSPTree::~BSPTree()
{
    DeleteChilds(mpRoot);
}


void BSPTree::DeleteChilds(BSPTreeNode* pNode)
{
    if (pNode != NULL)
    {
        DeleteChilds(pNode->left);
        DeleteChilds(pNode->right);
        delete pNode;
    }
    pNode = NULL;
      
}


void BSPTree::BuildBSPTree( std::vector<PlanePolygon>& polygons)
{
    if (polygons.size() > 1)
        mpRoot = BuildTree(polygons);
    else if (polygons.size()  == 1){
        PlaneMap planes;
        mpRoot = BuildTree(polygons[0], planes);
        planes.clear();
    }
}

PosRelation BSPTree::ClassifyPoint(const vec3<double>& p, BSPTreeNode* pNode) const
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


BSPTree::BSPTreeNode* BSPTree::BuildTree(PlanePolygon& polygon, PlaneMap& planes)
{

    const Plane<double>& bp = PickSplittingPlane(polygon, planes);
    BSPTreeNode* pNode = new BSPTreeNode;
    pNode->partition = bp;
    PlanePolygon front;
    PlanePolygon back;
    std::vector<Plane<double> > coincidents; 
    SplitPloygonsWithPlane(polygon, bp, planes, front, back, coincidents);
    for (int i = 0 ; i < coincidents.size(); i++)
    {
        PlanePolygon poly;
        poly.splane = coincidents[i];
        pNode->polygons.push_back(poly);
    }
    bool bSameOrient = bp.IsSimilarlyOrientation( pNode->polygons[0].splane);
   
    if (HomogeneousRegion(front, planes ))
    {
        pNode->left = new BSPTreeNode;
        if (bSameOrient)
            pNode->left->relation = In; 
        else 
            pNode->left->relation = Out;
    }else 
        pNode->left = BuildTree(front, planes);
    if (HomogeneousRegion(back, planes ))
    {
        pNode->right = new BSPTreeNode;
        if (bSameOrient)
            pNode->right->relation = Out; 
        else 
            pNode->right->relation = In;
    }else 
        pNode->right = BuildTree(back,planes);
  
    coincidents.clear();
    return pNode;
}




BSPTree::BSPTreeNode* BSPTree::BuildTree(std::vector<PlanePolygon>& polygons)
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
    if (backs.empty())
    {
        pNode->right = new BSPTreeNode;
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



void BSPTree::SplitPloygonsWithPlane( PlanePolygon& polygon, const Plane<double>& bp, PlaneMap& planes,
                                      PlanePolygon& front, PlanePolygon& back, std::vector<Plane<double> >& coincidents)
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
 

void BSPTree::SplitPloygonsWithPlane( std::vector<PlanePolygon>& polygons, const Plane<double>& bp, 
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
 

Plane<double> BSPTree::PickSplittingPlane( std::vector<PlanePolygon>& polygons) const 
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

bool BSPTree::HomogeneousRegion(const PlanePolygon& poly, PlaneMap& planes)
{
    for (int i = 0; i < poly.bplanes.size(); i++)
    {
        if (planes.find(poly.bplanes[i]) == planes.end())
            return false ; 
    }
    return true ;
}

Plane<double> BSPTree::PickSplittingPlane( PlanePolygon& polygon, PlaneMap& planes)
{

    const float K = 0.8f;
    Plane<double> bestPlane;
    float bestScore = FLT_MAX;
    int nSize = polygon.bplanes.size();
    for (int i = 0 ; i< nSize; i++)
    {
        int numFront = 0 ; 
        int numBack = 0; 
        int numStraddling = 0;
        const Plane<double>& bp = polygon.bplanes[i];
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

std::ofstream mergeFile;
unsigned int count;
//3D VERSION
BSPTree* BSPTree::Merge(const BSPTree* bTree, SET_OP op) const
{
	mergeFile.open("D:\\mergeDebug1.txt");
	assert(mpRoot && bTree && bTree->mpRoot);
	
	BSPTree *newTree = new BSPTree;
	assert(newTree);

	newTree->mpRoot = Merge(mpRoot, bTree->mpRoot, op);
	if (!newTree->mpRoot)
	{
		delete newTree;
		return nullptr;
	}
	mergeFile << "END";
	mergeFile.close();
	return newTree;
}

BSPTree::BSPTreeNode *BSPTree::Merge(const BSPTreeNode *nodeA, const BSPTreeNode *nodeB, SET_OP op) const
{
	static int counto = 0;
	counto ++;
	int count = counto;
	mergeFile << count << "   merge****************************\n";
	if (nodeA->IsCell()) 
	{
		BSPTreeNode* res = MergeTreeWithCell(nodeB, nodeA, op);
		mergeFile << count << " merge____________________________\n\n\n\n";
		return res;
	}
	if (nodeB->IsCell())
	{
		BSPTreeNode* res = MergeTreeWithCell(nodeA, nodeB, op);
		mergeFile << count << " merge____________________________\n\n\n\n";
		return res;
	}

	BSPTreeNode *newNode(nullptr);
	BSPTreeNode *splitLeftTree(nullptr), *splitRightTree(nullptr);
	SplitBSPTree(nodeA, nodeB, newNode, splitLeftTree, splitRightTree);


	mergeFile << *splitLeftTree << std::endl;
	mergeFile << *(nodeB->left);

	mergeFile << count << " fen ge fu\n\n";

	mergeFile << *splitRightTree << std::endl;
	mergeFile << *(nodeB->right);

	newNode->left = Merge(splitLeftTree, nodeB->left, op);
	newNode->right = Merge(splitRightTree, nodeB->right, op);
	//mergeFile << *(newNode->right);
	mergeFile << count << " fen ge fu2\n\n";

	mergeFile << *(newNode);

	mergeFile << count << " merge____________________________\n\n\n\n";

	return newNode;
}

// Tree and Cell are defined in the same region
BSPTree::BSPTreeNode *BSPTree::MergeTreeWithCell(const BSPTreeNode* tree, const BSPTreeNode* cell, SET_OP op) const
{
	mergeFile << "merge with cell ......................\n";
	mergeFile << *tree << *cell;
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
   
std::ofstream collectFile;
void BSPTree::GetPolygons(std::vector<PlanePolygon>& polys)
{
    collectFile.open("D:\\collectDebug1.txt");

     //CollectPolygons( mpRoot,  polys);
	std::vector<LeafPlanePolygon*> nullVector;
	mpRoot->FormBoundary(&nullVector, &nullVector, &polys);
	for (auto  itr: polys)
	{
		collectFile << itr.splane;
		collectFile << "blpanes\n";
		for (auto itrj: itr.bplanes)
		{
			collectFile << itrj;
		}
		collectFile <<  "\n\n\n";
	}

	collectFile.close();
}

void BSPTree::CollectPolygons(const BSPTree::BSPTreeNode* pNode,  std::vector<PlanePolygon>& polys)
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

void BSPTree::BSPTreeNode::FormBoundary(std::vector<LeafPlanePolygon*> *inPool,
						   std::vector<LeafPlanePolygon*> *outPool,
						   std::vector<PlanePolygon> *boundary) const
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
	std::vector<OrigPlanePolygon> pool(polygons.size());
	std::vector<LeafPlanePolygon*> leftInPool, rightInPool, leftOutPool, rightOutPool;
	PlanePolygon leftPolygon, rightPolygon;
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

			polyList.push_back(LeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = leftPolygon.bplanes;
			leftInPool.push_back(&polyList.back());

			polyList.push_back(LeafPlanePolygon());
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

			polyList.push_back(LeafPlanePolygon());
			polyList.back().parent = itr->parent;
			polyList.back().splane = itr->splane;
			polyList.back().bplanes = leftPolygon.bplanes;
			leftOutPool.push_back(&polyList.back());

			polyList.push_back(LeafPlanePolygon());
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
		pool[i].color = polygons[i].color;

		pool[i].subPolygon.push_back(LeafPlanePolygon());
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
			bItr.color = bItr.parent->color;
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


void BSPTree::FormSubHyperPlane(const Box3& bbox)
{
	BSPTreeIterator itr(this);
	while (true)
	{
		auto node = itr.getNext();
		if (!node) break;
		if (node->IsCell()) continue;

		auto oPoly = CreatePolygon(node->partition, bbox);
		PlanePolygon front, back;

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

PlanePolygon BSPTree::CreatePolygon(const Plane<double> &plane,
									const Box3& bbox) const
{
	PlaneMesh mesh(bbox);
	float4 color;
	mesh.AddPolygon(plane, color, bbox); 
	return (mesh.Ploygons())[0];
}

// 3 output msut be initialized by nullptr.
void BSPTree::SplitBSPTree(const BSPTreeNode* tg, 
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


void BSPTree::Negate()
{
    NegateTree(mpRoot);
}

void BSPTree::NegateTree(BSPTreeNode* pNode)
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

void BSPTree::ClassifyPolygon(const BSPTree::BSPTreeNode* pNode,   const PlanePolygon& poly,
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

void BSPTree::OutputDebugInfo(char* filename)
{
	auto out = std::ofstream(filename);
	out << (*this);
	out.close();
}

std::ostream& operator<<(std::ostream& out, BSPTree& tree)
{
	out << *(tree.mpRoot);
	return out;
}

std::ostream& operator<<(std::ostream& out, const BSPTree::BSPTreeNode& node)
{
	BSPTree::ConstBSPTreeIterator itr(&node);
	out << "///////////////////////////////////////////////////\n";
	out << "tree:\n";
	//out << "tree: " << ++count << std::endl;
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