#include "tbb\tick_count.h"
#include "tbb/spin_mutex.h"
#include "tbb\parallel_for.h"
#include "tbb\parallel_reduce.h"
#include "CSGTree.h"
#include "OctTree.h"

namespace GS{

CSGTreeNode::CSGTreeNode()
    :mpNoneCriticalMesh(NULL)
{
}

CSGTreeNode::~CSGTreeNode()
{
	mInfoList.clear();
    
}

//CSGTreeNode Implementation 
 bool CSGTreeNode::RelationTest(CSGMesh* pMesh, int TriId, bool bReverse , int Relations)
 {
	 return (RelationWith(pMesh, TriId, bReverse) & Relations) >0 ;
 }

 int  CSGTreeNode::RelationWith(CSGMesh* pMesh, int TriId, bool bReverse)
 {
	 int result = DoRelationWith(pMesh, TriId, bReverse);
	 if (!bReverse)
		 return result;
	 if (result & TMR_SAME)
		 return ((result ^ TMR_SAME) | TMR_OPPOSITE);
	 else if (result & TMR_OPPOSITE)
		 return ((result ^ TMR_OPPOSITE) | TMR_SAME);
	 return result;

 }

 CSGMesh* CSGTreeNode::CreateResultMesh()
 {
	 CSGMesh* mesh = new CSGMesh(true);
	 for (int i = 0 ; i< mInfoList.size(); i++)
	 {
		 CSGGroupInfo& info = mInfoList[i];
         for (int j = 0; j < info.Size(); j++)
         {
		     const int* VertexId = info.pMesh->TriangleInfo(info.TriIds[j]).VertexId;
		     VertexInfo  v1 = info.pMesh->Vertex(VertexId[0]);
		     VertexInfo  v2 = info.pMesh->Vertex(VertexId[1]);
		     VertexInfo  v3 = info.pMesh->Vertex(VertexId[2]);

		     if (info.bReverse)
		     {
			     v3.normal = -  v3.normal;
			     v2.normal = - v2.normal;
			     v1.normal = -v1.normal;
			     mesh->Add(v3, v2, v1);

             }
             else 
                 mesh->Add(v1, v2, v3);
         }

	 }
     if (mpNoneCriticalMesh)
     {
        for (int i = 0; i < mpNoneCriticalMesh->PrimitiveCount(); i++) 
		{
			const TriInfo&  info= mpNoneCriticalMesh->TriangleInfo(i);
			mesh->Add(mpNoneCriticalMesh->Vertex(info.VertexId[0]) , mpNoneCriticalMesh->Vertex(info.VertexId[1]), mpNoneCriticalMesh->Vertex(info.VertexId[2]));
			        
		}
        delete mpNoneCriticalMesh;
        mpNoneCriticalMesh = nullptr;
     }
     
	 return mesh;
 }
 //CSGLeafNode Implementation 

 CSGLeafNode::CSGLeafNode(CSGExprNode*  node,  std::hash_map<int, CSGRecMesh*>& recMeshDict, OctTree* pTree )
	 :mpRecMesh(NULL)
     ,mpTree(pTree)
 {
	 BaseMesh* pMesh = node->GetMesh();
	 if (recMeshDict.find(pMesh->GetID()) == recMeshDict.end())
	 {
		mpRecMesh = new CSGRecMesh(node->GetMesh());
		recMeshDict[pMesh->GetID()]= mpRecMesh;

	 }
 }


 CSGLeafNode::~CSGLeafNode()
 {
	 if (mpRecMesh !=NULL)
		 delete mpRecMesh; 
	 mpRecMesh= NULL;
 }

 void CSGLeafNode::Solve()
 {
     mInfoList.clear();
     mInfoList.reserve(mpRecMesh->mpTriMesh->GetTriangesGroupCount());
     for( int i = 0; i< mpRecMesh->mpTriMesh->GetTriangesGroupCount(); i++)
     {
         const GroupRec& rec = mpRecMesh->mpTriMesh->GetTriangleGroupRec(i);
         CSGGroupInfo info(mpRecMesh->mpTriMesh, rec.bUnified, false);
         int PrimitiveCount = rec.end - rec.start +1 ;
         info.TriIds.reserve(PrimitiveCount );
         for(int j = rec.start; j <= rec.end; j++) 
             info.TriIds.push_back(j);
        mInfoList.push_back(info);
     }
     mpNoneCriticalMesh = mpRecMesh->mpNoneCriticalMesh;
     mpRecMesh->mpNoneCriticalMesh = NULL;
 }

 void CSGLeafNode::GetAllMesh(RecMeshDict& CSGRecMeshDict)
 {
	 if(CSGRecMeshDict.find(mpRecMesh) ==CSGRecMeshDict.end() )
		CSGRecMeshDict[mpRecMesh] = true;
 }

int CSGLeafNode::DoRelationWith(CSGMesh* pMesh, int TriId, bool bReverse)
{
	Point3D point = pMesh->CenterOfGravity(TriId);
	Vec3D   normal = pMesh->TriangleInfo(TriId).Normal;
    PointMeshRelation  relation = pmrUnknown;
    if (mpTree)
        relation = mpTree->PointInPolyhedron(point, mpRecMesh->mpBaseMesh);
    else 
	    relation = mpRecMesh->mpBaseMesh->RelationWith(point, normal);
	switch (relation)
	{
		case pmrInside:    return TMR_INSIDE;
		case pmrSame :     return TMR_SAME;
		case pmrOpposite : return TMR_OPPOSITE;
		case pmrOutside:   return TMR_OUTSIDE;
		default :          assert(0);
	}
	return TMR_UNKNOWN;
}


//CSGOpNode Implementation
CSGOpNode::CSGOpNode(const int (&relation)[4][4], CSGTreeNode* leftNode, CSGTreeNode* rightNode)
	:mRelation(relation)
	,mpLeftNode(leftNode)
	,mpRightNode(rightNode)
{
}

CSGOpNode::~CSGOpNode()
{
	if (mpLeftNode)
		delete mpLeftNode;
	if (mpRightNode)
		delete mpRightNode;
}


void CSGOpNode::LeftRelationTest::operator()(const tbb::blocked_range<int>& r) 
{
    int leftRs = mpOpNode->LeftRelations();
    CSGGroupInfo& info = mpOpNode->mpLeftNode->mInfoList[mGroupID];
    mNewInfo.bReverse = info.bReverse;
    mNewInfo.bUnified = info.bUnified;
    mNewInfo.pMesh = info.pMesh;
	for (int i = r.begin(); i != r.end(); i++)
	{
        if (!info.bUnified)
        {
            if(mpOpNode->mpRightNode->RelationTest(info.pMesh,info.TriIds[i], info.bReverse,  leftRs))
                mNewInfo.TriIds.push_back(info.TriIds[i]);
        } 

    }
   
}

void CSGOpNode::LeftRelationTest::join( const LeftRelationTest& left )
{
    if (mNewInfo.Size() == 0)
        mNewInfo.TriIds = left.mNewInfo.TriIds;
    else if (left.mNewInfo.Size() )
        std::copy(left.mNewInfo.TriIds.begin(), left.mNewInfo.TriIds.end(), std::back_inserter(mNewInfo.TriIds));
    
}

void CSGOpNode::RightRelationTest::operator()(const tbb::blocked_range<int>& r) 
{
    int RRs = mpOpNode->RightRelations();
    CSGGroupInfo& info = mpOpNode->mpRightNode->mInfoList[mGroupID];
    mNewInfo.bReverse = mpOpNode->ReverseRight()? !info.bReverse :info.bReverse;
    mNewInfo.bUnified = info.bUnified;
    mNewInfo.pMesh = info.pMesh;
    for (int i = r.begin(); i != r.end(); i++)
    {
    
        if (!info.bUnified)
        {
            if(mpOpNode->mpLeftNode->RelationTest(info.pMesh, info.TriIds[i], info.bReverse,  RRs))
                mNewInfo.TriIds.push_back( info.TriIds[i]);
        }
    }
}

void CSGOpNode::RightRelationTest::join( const RightRelationTest& right )
{
    if (mNewInfo.Size() == 0)
        mNewInfo.TriIds = right.mNewInfo.TriIds;
    else if (right.mNewInfo.Size() )
        std::copy(right.mNewInfo.TriIds.begin(), right.mNewInfo.TriIds.end(), std::back_inserter(mNewInfo.TriIds));
    
}

void CSGOpNode::Solve()
{
   
	mpLeftNode->Solve();
	mpRightNode->Solve();
    int number = mpLeftNode->mInfoList.size() + mpRightNode->mInfoList.size();
    mInfoList.reserve(number);

 
   // tbb::parallel_for(tbb::blocked_range<int>(0,mpLeftNode->mInfoList.size(), 100), LeftRelationTest(this));
   // tbb::parallel_for(tbb::blocked_range<int>(0,mpRightNode->mInfoList.size(),100), RightRelationTest(this));
   /*         tbb::tick_count t1 = tbb::tick_count::now();
    WCHAR buffer [100];
    ::swprintf_s(buffer, 100, L"Time %f Sec", (t1-t0).seconds());
    ::MessageBox(NULL, buffer, L"Warning", MB_OK); */


    int LRs = LeftRelations();
	int RRs = RightRelations();
	for (int i = 0; i< mpLeftNode->mInfoList.size(); i++)
	{
		CSGGroupInfo& info = mpLeftNode->mInfoList[i];
        if (!info.bUnified)
        {
            LeftRelationTest lrt(this, i);
           // tbb::parallel_reduce(tbb::blocked_range<int>(0,info.Size(), 640), lrt);
            tbb::parallel_reduce(tbb::blocked_range<int>(0,info.Size()), lrt , tbb::auto_partitioner());
            if(lrt.mNewInfo.Size())
                mInfoList.push_back(lrt.mNewInfo);
        }else if (mpRightNode->RelationTest(info.pMesh, info.TriIds[0], info.bReverse,  LRs))
                mInfoList.push_back(info);

    }
    for (int i = 0; i< mpRightNode->mInfoList.size(); i++)
	{
		CSGGroupInfo& info = mpRightNode->mInfoList[i];
        if (!info.bUnified)
        {
            RightRelationTest rrs(this, i);
            //tbb::parallel_reduce(tbb::blocked_range<int>(0,info.Size(), 640), rrs);
            tbb::parallel_reduce(tbb::blocked_range<int>(0,info.Size()), rrs, tbb::auto_partitioner());
             if (rrs.mNewInfo.Size())
                mInfoList.push_back(rrs.mNewInfo);

        }
        else if (mpLeftNode->RelationTest(info.pMesh, info.TriIds[0], info.bReverse,  RRs))
        {
               CSGGroupInfo newInfo(info.pMesh, info.bUnified, info.bReverse);
               newInfo.TriIds = info.TriIds;
               if (ReverseRight())
                  newInfo.bReverse = !info.bReverse;
              mInfoList.push_back(newInfo);
        }
    }

    /*int LRs = LeftRelations();
	int RRs = RightRelations();
	for (int i = 0; i< mpLeftNode->mInfoList.size(); i++)
	{
		CSGGroupInfo& info = mpLeftNode->mInfoList[i];
        if (!info.bUnified)
        {
            CSGGroupInfo newInfo(info.pMesh, info.bUnified, info.bReverse);
            for (int j= 0;  j < info.Size(); j ++)
            {
                if(mpRightNode->RelationTest(info.pMesh, info.TriIds[j], info.bReverse,  LRs))
                    newInfo.TriIds.push_back( info.TriIds[j]);
            }
            if (newInfo.Size())

                mInfoList.push_back(newInfo);


        }else if (mpRightNode->RelationTest(info.pMesh, info.TriIds[0], info.bReverse,  LRs))
                mInfoList.push_back(info);

    }
    for (int i = 0; i< mpRightNode->mInfoList.size(); i++)
	{
		CSGGroupInfo& info = mpRightNode->mInfoList[i];
        CSGGroupInfo newInfo(info.pMesh, info.bUnified, info.bReverse);
        bool bAdd  = false;
        if (!info.bUnified)
        {
           
            for (int j= 0;  j < info.Size(); j ++)
            {
                if(mpLeftNode->RelationTest(info.pMesh, info.TriIds[j], info.bReverse,  RRs))
                    newInfo.TriIds.push_back( info.TriIds[j]);
        
            }
            if (newInfo.Size())
                bAdd = true;

        }else if (mpLeftNode->RelationTest(info.pMesh, info.TriIds[0], info.bReverse,  RRs))
        {
               newInfo.TriIds = info.TriIds;
               bAdd = true;
        }
        if (bAdd)
        {
            if (ReverseRight())
                newInfo.bReverse = !info.bReverse;
            mInfoList.push_back(newInfo);
        }

    }
*/
     
	
}
	
int CSGOpNode::DoRelationWith(CSGMesh* pMesh, int TriId, bool bReverse)
{
    int leftIndex  = GetTableIndex(mpLeftNode->RelationWith(pMesh, TriId, bReverse));
	int rightIndex = GetTableIndex(mpRightNode->RelationWith(pMesh, TriId, bReverse));
	return mRelation[leftIndex][rightIndex];
}

// CSGUnionNode Implementation 
CSGUnionNode::CSGUnionNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode)
	:CSGOpNode(UnionRelationTable, leftNode, rightNode)
{
}

void CSGUnionNode::Solve()
{
    CSGOpNode::Solve();
    mpNoneCriticalMesh = mpLeftNode->mpNoneCriticalMesh;
    if (mpNoneCriticalMesh == nullptr )
    {
        mpNoneCriticalMesh =  mpRightNode->mpNoneCriticalMesh;
        return ; 
    } 
    if (mpRightNode->mpNoneCriticalMesh == nullptr)
        return ; 
    for (int i = 0; i < mpRightNode->mpNoneCriticalMesh->PrimitiveCount(); i++) 
	{
		const TriInfo&  info= mpRightNode->mpNoneCriticalMesh->TriangleInfo(i);
		mpNoneCriticalMesh->Add(mpRightNode->mpNoneCriticalMesh->Vertex(info.VertexId[0]) , 
              mpRightNode->mpNoneCriticalMesh->Vertex(info.VertexId[1]), mpRightNode->mpNoneCriticalMesh->Vertex(info.VertexId[2]));	        
	}

     delete mpRightNode->mpNoneCriticalMesh;
     mpRightNode->mpNoneCriticalMesh = nullptr;
}

int CSGUnionNode::LeftRelations()
{
	return (TMR_OUTSIDE | TMR_SAME);
}

int CSGUnionNode::RightRelations()
{
	return TMR_OUTSIDE;
}

// CSGIntersectNode Implementation
CSGIntersectNode::CSGIntersectNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode)
	:CSGOpNode(IntersectRelationTable, leftNode, rightNode)
{
}


int CSGIntersectNode::LeftRelations()
{
	return ( TMR_INSIDE | TMR_SAME);
}

int CSGIntersectNode::RightRelations()
{
	return TMR_INSIDE;
}

void CSGIntersectNode::Solve()
{
     CSGOpNode::Solve();

     if (mpRightNode->mpNoneCriticalMesh )
         delete mpRightNode->mpNoneCriticalMesh;
     mpRightNode->mpNoneCriticalMesh = nullptr;
     if (mpLeftNode->mpNoneCriticalMesh )
         delete mpLeftNode->mpNoneCriticalMesh;
     mpLeftNode->mpNoneCriticalMesh = nullptr;
}

//// CSGDiffNode Implementation
CSGDiffNode::CSGDiffNode(CSGTreeNode* leftNode, CSGTreeNode* rightNode)
	:CSGOpNode(DiffRelationTable, leftNode, rightNode)
{
}

void CSGDiffNode::Solve()
{
     CSGOpNode::Solve();
     mpNoneCriticalMesh = mpLeftNode->mpNoneCriticalMesh;
     if (mpRightNode->mpNoneCriticalMesh )
         delete mpRightNode->mpNoneCriticalMesh;
     mpRightNode->mpNoneCriticalMesh = nullptr;
}

int CSGDiffNode::LeftRelations() 
{
	return ( TMR_OUTSIDE | TMR_OPPOSITE);
}

int CSGDiffNode::RightRelations()
{
	return TMR_INSIDE;
}
//////////////////////////////////////////////////////

CSGTree::~CSGTree()
{
	Clear();
}


void CSGTree::AddExpr(CSGExprNode* node )
{
	if(mRoots.find(node) == mRoots.end())
		mRoots[node] = NULL;
}


void CSGTree::Evaluate()
{
	CSGRecMeshPair testPair;
	std::map<CSGRecMeshPair, bool> InterCarveDict;
 
	RecMeshDict AllMesh;
      
	BuildTrees();
      
   
	std::map<CSGExprNode*, CSGTreeNode*>::iterator iter = mRoots.begin();
	for(;iter!=mRoots.end();++iter)
	{
		if(iter->second)
		{
			iter->second->GetAllMesh(AllMesh);
			std::vector<CSGRecMesh*> recMeshList(AllMesh.size());
			RecMeshDict::iterator recMeshIter= AllMesh.begin();
			for (int i = 0; recMeshIter != AllMesh.end(); i++,recMeshIter++)
			{
				recMeshList[i] = recMeshIter->first;
			}
   			for(int i = 0 ; i < recMeshList.size()-1; i++)
			{
				for (int j = i+1; j < recMeshList.size(); j++)
				{
					testPair.first = recMeshList[i];
					testPair.second = recMeshList[j];
					if (InterCarveDict.find(testPair) == InterCarveDict.end())
						InterCarveDict[testPair] = true;
				}
			}
			recMeshList.clear();
		}
	}

	std::map<CSGRecMeshPair, bool>::iterator pairIter = InterCarveDict.begin();
	for (; pairIter !=InterCarveDict.end(); pairIter++ )
		pairIter->first.first->InterCarve(*pairIter->first.second);

     
	std::hash_map<int, CSGRecMesh*>::iterator it = mCSGRecMesh.begin();
	for(;it!=mCSGRecMesh.end();++it)
	{
		it->second->Triangulate();
		it->second->ClearCarveMesh();
	}
   

	iter = mRoots.begin();

	for(;iter!=mRoots.end();++iter)
	{
		iter->second->Solve();
	}

	AllMesh.clear();
	InterCarveDict.clear();

}


void CSGTree::Evaluate1()
{
	CSGRecMeshPair testPair;
	std::map<CSGRecMeshPair, bool> InterCarveDict;
 
	RecMeshDict AllMesh;// dictionary from recmesh to bool
    mpTree = new OctTree();  
	BuildTrees();
    tbb::tick_count t0 = tbb::tick_count::now(); 
  
	std::map<CSGExprNode*, CSGTreeNode*>::iterator iter = mRoots.begin();
	for(;iter!=mRoots.end();++iter)
    {
        if(iter->second)
        {
            iter->second->GetAllMesh(AllMesh);
            std::vector<CSGRecMesh*> recMeshList(AllMesh.size());
            RecMeshDict::iterator recMeshIter= AllMesh.begin();
            for (int i = 0; recMeshIter != AllMesh.end(); i++,recMeshIter++)
            {
                recMeshList[i] = recMeshIter->first;
            }
            std::vector<BaseMesh*> meshes(recMeshList.size());
            for (int i = 0; i < recMeshList.size(); i++)
            {
                meshes[i] = recMeshList[i]->mpBaseMesh;
            }
            

            //OctTree* pTree = new OctTree();
            mpTree->BuildOctTree(meshes);
            std::vector<BaseMesh*> noneCriticalMeshes;
            std::vector<CSGMesh*> csgmeshes;
             
            mpTree->GenMeshesFromCells(noneCriticalMeshes, eNormal);
     
            mpTree->CarveCriticalCelles(csgmeshes);
       
           // delete pTree;
            for (int i = 0 ; i < recMeshList.size(); i++)
            {
                if (csgmeshes.size() && csgmeshes[i])
                    recMeshList[i]->mpCarveMesh = csgmeshes[i];
                
              if (noneCriticalMeshes.size() && noneCriticalMeshes[i])
                {
                    recMeshList[i]->mpNoneCriticalMesh = noneCriticalMeshes[i];
                }
            }
            noneCriticalMeshes.clear();
			recMeshList.clear();
            csgmeshes.clear();
		}
	}

       
	std::hash_map<int, CSGRecMesh*>::iterator it = mCSGRecMesh.begin();
	for(;it!=mCSGRecMesh.end();++it)
	{
		it->second->Triangulate();
		it->second->ClearCarveMesh();
	}
     
    //tbb::tick_count t0 = tbb::tick_count::now();
    // WCHAR buffer [100];
    //::swprintf_s(buffer, 100, L"Time %f mec", (t1-t0).seconds());
    //::MessageBox(NULL, buffer, L"Warning", MB_OK); 
	iter = mRoots.begin();

	for(;iter!=mRoots.end();++iter)
	{
		iter->second->Solve();
	}

	AllMesh.clear();
	InterCarveDict.clear();
     
    delete mpTree;
    mpTree= nullptr;
    tbb::tick_count t1 = tbb::tick_count::now();
     WCHAR buffer [100];
    ::swprintf_s(buffer, 100, L"Time %f Sec", (t1-t0).seconds());
    ::MessageBox(NULL, buffer, L"Warning", MB_OK); 
}

/*procedure TCSGTree.Clear;
begin
  FCSGRecMesh.Clear;
  FRoots.Clear;
end;*/

void CSGTree::Clear()
{
	std::map<CSGExprNode*, CSGTreeNode*>::iterator iter = mRoots.begin();
	for(;iter!=mRoots.end();++iter)
	{
		delete iter->second;
		delete iter->first;
	}
	mRoots.clear();
	/*std::hash_map<int, CSGRecMesh*>::iterator recMeshIter= mCSGRecMesh.begin();
	for(; recMeshIter != mCSGRecMesh.end(); recMeshIter++)
		delete recMeshIter->second;*/
	mCSGRecMesh.clear();
    if (mpTree)
        delete mpTree;
    mpTree = nullptr;
}

CSGMesh* CSGTree::GetResultCopy(CSGExprNode* node )
{
    
	if(mRoots.find(node)!= mRoots.end())

		return  mRoots[node]->CreateResultMesh();
      
	return NULL;
}

CSGTreeNode* CSGTree::DoBuild(CSGExprNode* node)
{
	if (node == NULL)
		return NULL;
	if (node->GetMesh())
        return new CSGLeafNode(node, mCSGRecMesh, mpTree);
	CSGOpNode* TreeOpNode = NULL; 
	switch (node->BoolOperation())
	{
		case CSGUnion: 
			TreeOpNode = new CSGUnionNode(DoBuild(node->LeftNode()), DoBuild(node->RightNode()));
		    break; 
		case CSGIntersect:
			TreeOpNode = new CSGIntersectNode(DoBuild(node->LeftNode()), DoBuild(node->RightNode()));
			break;
		case CSGDiff:
			TreeOpNode = new CSGDiffNode(DoBuild(node->LeftNode()), DoBuild(node->RightNode()));
			break;
		default :
			assert(0);
	}
    return TreeOpNode;
}

void CSGTree::BuildTrees()
{
	std::map<CSGExprNode*, CSGTreeNode*>::iterator iter = mRoots.begin();
	for(;iter!=mRoots.end();++iter)
	{
		mRoots[iter->first] = DoBuild(iter->first);
	}

}

};