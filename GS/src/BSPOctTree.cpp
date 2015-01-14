#include "BSPOctTree.h"
#include <vld.h>
#include <ctime>
#include <string>

namespace GS
{


static inline double3 normalize(double3 &v, float3 &center, float3 &scale)
{
	return (v-center)/scale*2.0;
}

static inline double3 normalize(const float3 &v, float3 &center, float3 &scale)
{
	return (v-center)/scale*2.0;
}

static inline double3 denormalize(double3 &v, float3 &center, float3 &scale)
{
	return v/2.0*scale+center;
}

static inline void normalizeBox3(const Box3& target, const Box3& env, Box3& output)
{
    auto scale = env.Diagonal();
    auto center = env.Center();
    
    output.Set((target.Min()-center)/scale*2.0, 
        (target.Max()-center)/scale*2.0);
}

static inline double staticFilter(double x, double factor)
{
    return round(x*factor)/factor;
}

static inline void staticFilter(Box3& box, double factor)
{
    double3 minimum(box.Min());
    double3 maximum(box.Max());

    minimum.x = staticFilter(minimum.x, factor);
    minimum.y = staticFilter(minimum.y, factor);
    minimum.z = staticFilter(minimum.z, factor);

    maximum.x = staticFilter(maximum.x, factor);
    maximum.y = staticFilter(maximum.y, factor);
    maximum.z = staticFilter(maximum.z, factor);
}

static inline void staticFilterP(Box3& box, unsigned p)
{
    const double factor = pow(2.0, p);
    staticFilter(box, factor);
}

/// positive in, negative out
static inline bool isPointInBoxBias(const Box3& box, const double3& point)
{
    if (point.x > box.Max().x) return false;
    if (point.y > box.Max().y) return false;
    if (point.z > box.Max().z) return false;

    if (point.x <= box.Min().x) return false;
    if (point.y <= box.Min().y) return false;
    if (point.z <= box.Min().z) return false;

    return true;
}

static void getGravityCenter(FixedPlanePolygon* poly, double3& res)
{
    res.x = res.y = res.z = 0.0;
    for (int i = 0 ; i < poly->bplanes.size(); i++)
    {
        int prevPtIdx = i == 0? poly->bplanes.size() -1 : i -1;
		res += InexactComputePoint(poly->splane, poly->bplanes[prevPtIdx], poly->bplanes[i] );
    }
    res /= poly->bplanes.size();
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////SimpleMesh/////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

SimpleMesh::SimpleMesh(const BaseMesh* mesh, int para)
{
    switch (para)
    {
    case 0:
        {
            int n = (int)mesh->VertexCount();
            for (int i = 0; i < n; i++)
                mVertex.push_back(mesh->Vertex(i).pos);

            n = (int)mesh->PrimitiveCount();
            for (int i = 0; i < n; i++)
            {
                auto &index = mesh->mpMeshImp->mTriangle[i].VertexId;
                mTriangle.emplace_back(index[0], index[1], index[2]);
            }         
        }
        break;
    case PARA_NEGATE:
        {
            int n = (int)mesh->VertexCount();
            for (int i = 0; i < n; i++)
                mVertex.push_back(mesh->Vertex(i).pos);

            n = (int)mesh->PrimitiveCount();
            for (int i = 0; i < n; i++)
            {
                auto &index = mesh->mpMeshImp->mTriangle[i].VertexId;
                mTriangle.emplace_back(index[2], index[1], index[0]);
            }
        }
    default:
        break;
    }

}



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////NormalMesh/////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


NormalMesh::NormalMesh(const BaseMesh* mesh, int para):
    SimpleMesh(mesh, para)
    , mCurAABB(mesh->AABB())
    , m_bCoordNormalized(false)
{
}

void NormalMesh::NormalizeCoord(const Box3* bbox)
{
    assert(!m_bCoordNormalized);

    m_bCoordNormalized = true;
    mOrigAABB = mCurAABB;

    if (bbox) mTransAABB = *bbox;
    else mTransAABB = mCurAABB;

	auto center = mTransAABB.Center();
	auto scale = mTransAABB.Diagonal();
    mCurAABB.Set(normalize(mCurAABB.Min(), center, scale), 
        normalize(mCurAABB.Max(), center, scale));

	for (auto &v: mVertex)
		v = normalize(v, center, scale);
}
	
void NormalMesh::DenormalizeCoord()
{
    assert(m_bCoordNormalized);

    m_bCoordNormalized = false;
    mCurAABB = mOrigAABB;

	auto center = mTransAABB.Center();
	auto scale = mTransAABB.Diagonal();

	for (auto &v: mVertex)
		v = denormalize(v, center, scale);
}

void NormalMesh::FilterVertex(unsigned p)
{
    const double factor = pow(2.0, p); 
    for (auto& itr: mVertex)
    {
        itr.x = staticFilter(itr.x, factor);
        itr.y = staticFilter(itr.y, factor);
        itr.z = staticFilter(itr.z, factor);
    }
}



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////BSPOctree//////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

static void GSOutputTimeLog(const wchar_t* ch)
{
	std::wstring debug;
	debug += ch;
	debug += std::to_wstring(clock());
	debug += L"\n";
	OutputDebugString(debug.c_str());
}

BSPOctree::~BSPOctree(void)
{
}

void BSPOctree::BSPOperation(BaseMesh *mesh1, BaseMesh *mesh2, BaseMesh** output)
{
    Box3 bbox = mesh1->AABB();
    bbox.IncludeBox(mesh2->AABB());

    NormalMesh *myMesh1 = new NormalMesh(mesh1), *myMesh2;

    if (FixedBSPTree::OP_DIFFERENCE == mOperation)
        myMesh2 = new NormalMesh(mesh2, NormalMesh::PARA_NEGATE);
    else myMesh2 = new NormalMesh(mesh2);

    myMesh1->NormalizeCoord(&bbox);
    myMesh1->FilterVertex(11);
    myMesh2->NormalizeCoord(&bbox);
    myMesh2->FilterVertex(11);
    
    normalizeBox3(mesh1->AABB(), bbox, mAABB1);
    normalizeBox3(mesh2->AABB(), bbox, mAABB2);
    staticFilterP(mAABB1, 11);
    staticFilterP(mAABB2, 11);

    auto t0 = clock();
    // initialize the para of the first cell
    PolygonPtrList pool1, pool2;
    GSOutputTimeLog(L"init: ");

    auto &ver1 = myMesh1->Vertex();
    auto &tri1 = myMesh1->Triangle();
    int i = 0;
    for (auto &titr: tri1)
    {
        mMesh1.emplace_back(ver1[titr[0]], ver1[titr[1]], ver1[titr[2]]);
        mMesh1.back().index = i;
        mMesh1.back().mpMesh = mesh1;
        mMesh1.back().mTriId = i;
        pool1.push_back(i++);
    }
    delete myMesh1;

    auto &ver2 = myMesh2->Vertex();
    auto &tri2 = myMesh2->Triangle();
    i = 0;
    for (auto &titr: tri2)
    {
        mMesh2.emplace_back(ver2[titr[0]], ver2[titr[1]], ver2[titr[2]]);
        mMesh2.back().mpMesh = mesh2;
        mMesh2.back().mTriId = i;
        pool2.push_back(i++);
    }
    delete myMesh2;

    for (auto &meshe: mMesh1) meshe.pMeshData = &meshe;
    for (auto &meshe: mMesh2) meshe.pMeshData = &meshe;

    Box3 initBox(Box3::BOX_MIRROR_SCALE);

    mesh1->NormalizeCoord(&bbox);
    mesh2->NormalizeCoord(&bbox);

    GSOutputTimeLog(L"start Iteration: ");
    PerformIteration(pool1, pool2, initBox, &mpRoot);

    if (*output) delete *output;
    auto &outputPtr = *output;

    GSOutputTimeLog(L"end Iteration: ");
    outputPtr = CollectPolygons(mesh1, mesh2);

    GSOutputTimeLog(L"end collection: ");
    mesh1->DenormalizeCoord();
    mesh2->DenormalizeCoord();
    outputPtr->DenormalizeCoord(&bbox);
    outputPtr->GenAABB(false);

    auto t1 = clock();
    long t = t1-t0;
    wchar_t ch[32];
    wsprintf(ch, L"time: %d", t);
    //MessageBox(0, ch, L"time", 0);
}

BaseMesh* BSPOctree::CollectPolygons(BaseMesh* mesh1, BaseMesh* mesh2)
{
    FixedPlaneMesh* pMesh = new FixedPlaneMesh;
    std::vector<PolygonObj*> polyBuffer;

    for (auto &itr3: mPlanePolygon) pMesh->AddPolygon(itr3); // collect carved mesh

    // get all non-intersect nodes
    std::vector<OctLeafNode*> leaves;
    //if (flag) GetLeafNodes(mpRoot, leaves, eNormal | eCritical);
    //else GetLeafNodes(mpRoot, leaves, eNormal);
    GetLeafNodes(mpRoot, leaves, eNormal | eCritical);

    size_t t = leaves.size();
    wchar_t ch[32];
    wsprintf(ch, L"leaves: %u\n", t);
    OutputDebugString(ch);

    size_t a, b, c;
    a = b = c = 0;
    for (auto pLeaf: leaves)
    {
        if (pLeaf->polygons.empty()) continue;
        BaseMesh* belong = pLeaf->polygons[0]->mpMesh;
        BaseMesh* compareTarget = (belong == mesh1)?mesh2:mesh1;

        int isCellInMesh = -1;
        assert(pLeaf->type != eIntered);
        if (pLeaf->type == eNormal) isCellInMesh = false;
        else {a++;isCellInMesh = (pmrInside == PointInPolyhedron(pLeaf->bbox.Center(), compareTarget));}

        if (!NeedInserted(isCellInMesh, (mesh1 == compareTarget)?MINUEND:SUBTRAHEND)) continue;

        auto &bbox = pLeaf->bbox;
        FixedPlane bounds[] = {FixedPlane(double3(1,0,0),-bbox.Min().x)
                    , FixedPlane(double3(-1,0,0),bbox.Max().x)
                    , FixedPlane(double3(0,1,0),-bbox.Min().y)
                    , FixedPlane(double3(0,-1,0),bbox.Max().y)
                    , FixedPlane(double3(0,0,1),-bbox.Min().z)
                    , FixedPlane(double3(0,0,-1),bbox.Max().z)};

        for (auto poly: pLeaf->polygons)
        {
            if (poly->mpSharedData->meObjType != eIntered) polyBuffer.push_back(poly);
            else
            {
                b++;
                auto orig = poly->pMeshData->OrigPolygon;
                bool bNeedInsert = true;
                for (int i = 0; i < 6; i++)
                {
                    orig.ClipByPlaneNoFront(bounds[i]);
                    if (orig.Size() == 0)
                    {
                        bNeedInsert = false;
                        break;
                    }
                }
                if (bNeedInsert) pMesh->AddPolygon(orig);
            }
        }
    }
    wsprintf(ch, L"in&out: %u, clip: %u\n", a, b);
    OutputDebugString(ch);
    GSOutputTimeLog(L"end leaves traverse: ");
    BaseMesh* result = pMesh->ToBaseMesh();
    delete pMesh;
    std::hash_map<IndexPair, bool, IndexPairCompare> sharedObjMap;
    for (auto poly: polyBuffer)
        poly->AddPolygon(*result, sharedObjMap, eIntered);

    return result;
}

bool BSPOctree::NeedInserted(bool isCellInMesh, int flag)
{
    return LookUpRelation(isCellInMesh, flag);
}

// must be critical cell
void BSPOctree::PerformIteration(PolygonPtrList& mesh1, PolygonPtrList& mesh2
                                 , Box3& bbox, OctTreeNode **node)
{
    if ((mesh1.size() + mesh2.size()) < 17)
    {
        PerformBoolean(mesh1, mesh2, bbox, node);
        return;
    }

    Box3 childBBox[8];
    bool bCriticalCells[8];
    memset(bCriticalCells, 0 , 8);

    SplitSpaceByXYZ(bbox, childBBox);
    DetermineCriticalCell(childBBox, bCriticalCells);

    PolygonPtrList XYZSplits1[8], XYZSplits2[8];
    for (auto index: mesh1)
    {
        auto &pos = mMesh1[index].Position;
        for (int i = 0; i < 8; i++)
        {
            //if (!bCriticalCells[i]) continue;
            if (TestTriangleAABBOverlap(pos[0], pos[1], pos[2], childBBox[i]))
            {
                XYZSplits1[i].push_back(index);
            }
        }
    }

    for (auto index: mesh2)
    {
        auto &pos = mMesh2[index].Position;
        for (int i = 0; i < 8; i++)
        {
            //if (!bCriticalCells[i]) continue;
            if (TestTriangleAABBOverlap(pos[0], pos[1], pos[2], childBBox[i]))
            {
                XYZSplits2[i].push_back(index);
            }
        }
    }

    // for containment test, jxd
    auto &pNode = *node;
    pNode = new OctTreeNode;
    pNode->bbox = bbox;
    pNode->type = eIntered;
    // end

    for (int i = 0; i < 8; i++)
    {
        if (!bCriticalCells[i])/* continue;*/
        {
            // for containment test, jxd
            OctLeafNode *pLeaf = new OctLeafNode;
            pLeaf->bbox = childBBox[i];
            pLeaf->type = eNormal;
            FillOctreeLeafNode(XYZSplits1[i], XYZSplits2[i], pLeaf);
            pNode->child[i] = pLeaf;
            // end
        }
        else
        {
            if (XYZSplits1[i].size()*XYZSplits2[i].size() == 0)
            {
                // for containment test, jxd
                OctLeafNode *pLeaf = new OctLeafNode;
                pLeaf->bbox = childBBox[i];
                pLeaf->type = eCritical;
                FillOctreeLeafNode(XYZSplits1[i], XYZSplits2[i], pLeaf);
                pNode->child[i] = pLeaf;
                // end
            }
            else PerformIteration(XYZSplits1[i], XYZSplits2[i], childBBox[i], &(pNode->child[i]));
        }

    }
}

void BSPOctree::SplitSpaceByXYZ(const Box3& bbox,  Box3 childBoxes[])
{
    vec3<float> minOffset, maxOffset;
    const double factor = pow(2.0, 11);
    double3 step = double3(
        staticFilter(bbox.Diagonal().x* 0.5, factor)
        , staticFilter(bbox.Diagonal().y*0.5, factor)
        ,  staticFilter(bbox.Diagonal().z*0.5, factor));

    for (int i = 0; i < 8 ; i++)
    {
        maxOffset.z = i & 1 ?  0 : -step.z; 
        maxOffset.y = i & 2 ?  0 : -step.y;
        maxOffset.x = i & 4 ?  0 : -step.x;
        minOffset.z = i & 1 ?  step.z : 0; 
        minOffset.y = i & 2 ?  step.y : 0;
        minOffset.x = i & 4 ?  step.x : 0;
        childBoxes[i].Set(bbox.Min() + minOffset, bbox.Max()+ maxOffset);
    }
}

void BSPOctree::DetermineCriticalCell(const Box3 cells[], bool bCriticals[])
{
    for (int i = 0 ; i < 8 ; i++)
    {
        bCriticals[i] = false ;
        const Box3& box = cells[i];
        int nInterected = 0; 
        if (box.IntersectBias(mAABB1) && box.IntersectBias(mAABB2))
            bCriticals[i] = true;
    }
}

void BSPOctree::PerformBoolean(PolygonPtrList& mesh1, PolygonPtrList& mesh2, const Box3& bbox, OctTreeNode** node)
{
    FixedPlane bounds[] = {FixedPlane(double3(1,0,0),-bbox.Min().x)
                            , FixedPlane(double3(-1,0,0),bbox.Max().x)
                            , FixedPlane(double3(0,1,0),-bbox.Min().y)
                            , FixedPlane(double3(0,-1,0),bbox.Max().y)
                            , FixedPlane(double3(0,0,1),-bbox.Min().z)
                            , FixedPlane(double3(0,0,-1),bbox.Max().z)};

    FixedPlaneMesh myMesh1, myMesh2;
    FixedPlanePolygon poly;
    std::list<FixedPlanePolygon*> polyPool;

    // for containment test, jxd
    OctLeafNode* pLeaf = new OctLeafNode;
    pLeaf->bbox = bbox;
    // end

    for (auto index: mesh1)
    {
        MeshData *data = &(mMesh1[index]);
        /// convert simple mesh into plane based mesh
        if (!data->bSplitted)
        {
            data->bSplitted = true;
            data->OrigPolygon = FixedPlanePolygon(data->Position[0],
                data->Position[1], data->Position[2]);
            FixedPlanePolygon *root = new FixedPlanePolygon(data->OrigPolygon);
            data->Fragments.push_back(root);
        }

        // get the inside polygons: myMesh1
        poly = data->OrigPolygon;
        bool bNeedInsert = true;
        for (int i = 0; i < 6; i++)
        {
            poly.ClipByPlaneNoFront(bounds[i]);
            if (poly.Size() == 0)
            {
                bNeedInsert = false;
                break;
            }
        }
        if (bNeedInsert) myMesh1.AddPolygon(poly);
        poly.Clear();
    }
    if (myMesh1.PrimitiveCount() == 0)
    {
        pLeaf->type = eCritical;
        FillOctreeLeafNode(mesh1, mesh2, pLeaf);
        *node = pLeaf;
        return;
    }

    for (auto index: mesh2)
    {
        /// convert simple mesh into plane based mesh
        MeshData *data = &(mMesh2[index]);
        if (!data->bSplitted)
        {
            data->bSplitted = true;
            data->OrigPolygon = FixedPlanePolygon(data->Position[0],
                data->Position[1], data->Position[2]);
            FixedPlanePolygon *root = new FixedPlanePolygon(data->OrigPolygon);
            data->Fragments.push_back(root);
        }

        // get the inside polygons: myMesh2
        poly = data->OrigPolygon;
        bool bNeedInsert = true;
        for (int i = 0; i < 6; i++)
        {
            poly.ClipByPlaneNoFront(bounds[i]);
            if (poly.Size() == 0)
            {
                bNeedInsert = false;
                break;
            }
        }
        if (bNeedInsert) myMesh2.AddPolygon(poly);
        poly.Clear();
    }
    if (myMesh2.PrimitiveCount() == 0)
    {
        pLeaf->type = eCritical;
        FillOctreeLeafNode(mesh1, mesh2, pLeaf);
        *node = pLeaf;
        return;
    }
    FixedBSPTree *tree1, *tree2;
    tree1 = myMesh1.ToBSPTree();
    tree1->FormSubHyperPlane(bbox);
    tree2 = myMesh2.ToBSPTree();
    tree2->FormSubHyperPlane(bbox);

    auto op = mOperation;
    if (mOperation == FixedBSPTree::OP_DIFFERENCE)
    {
        op = FixedBSPTree::OP_INTERSECT;
    }

    FixedBSPTree *mergeTree = tree1->Merge(tree2, op);
    delete tree1;
    delete tree2;

    mergeTree->GetPolygons(mPlanePolygon);
    delete mergeTree;

    pLeaf->type = eIntered;
    FillOctreeLeafNode(mesh1, mesh2, pLeaf);
    *node = pLeaf;
}

bool BSPOctree::CheckValid(const double3& point, BaseMesh* mesh, int flag) // flag = 0, means that mesh is mesh1
{
    //auto relation = mesh->RelationWith(point, double3(0, 0, 1));
    auto relation = PointInPolyhedron(point, mesh);
    bool judge = (relation == pmrInside);

    assert(relation == pmrInside || relation == pmrOutside);
    return LookUpRelation(judge, flag);
}

bool BSPOctree::LookUpRelation(bool judge, bool flag)
{
    switch (mOperation)
    {
    case FixedBSPTree::OP_UNION:
        if (judge) return false;
        else return true;
        break;
    case FixedBSPTree::OP_INTERSECT:
        if (judge) return true;
        else return false;
        break;
    case FixedBSPTree::OP_DIFFERENCE:
        if (flag ^ judge) return true;
        else return false;
        break;
    default:
        assert(0);
        return false;
        break;
    }
}

void BSPOctree::FillOctreeLeafNode(PolygonPtrList& poly1, PolygonPtrList& poly2, OctLeafNode* node)
{
    assert(node);
    node->polygons.clear();
    auto type = node->type;

    for (auto index: poly1)
    {
        auto ptr = mMesh1[index].Clone();
        ptr->SetObjTypeFlag(type);
        node->polygons.push_back(ptr);
    }

    for (auto index: poly2)
    {
        auto ptr = mMesh2[index].Clone();
        ptr->SetObjTypeFlag(type);
        node->polygons.push_back(ptr);
    }
}

} // namespace GS