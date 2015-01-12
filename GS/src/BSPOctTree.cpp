#include "BSPOctTree.h"

namespace GS
{


static inline double3 normalize(double3 &v, float3 &center, float3 &scale)
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


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////SimpleMesh/////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

SimpleMesh::SimpleMesh(const BaseMesh* mesh)
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



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////NormalMesh/////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


NormalMesh::NormalMesh(const BaseMesh* mesh):
    SimpleMesh(mesh)
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

    mCurAABB = Box3(Box3::BOX_MIRROR);
	auto center = mTransAABB.Center();
	auto scale = mTransAABB.Diagonal();

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

BSPOctree::~BSPOctree(void)
{
}

void BSPOctree::BSPOperation(const BaseMesh *mesh1, const BaseMesh *mesh2, BaseMesh** output)
{
    Box3 bbox = mesh1->AABB();
    bbox.IncludeBox(mesh2->AABB());

    NormalMesh myMesh1(mesh1), myMesh2(mesh2);
    myMesh1.NormalizeCoord(&bbox);
    myMesh1.FilterVertex(11);
    myMesh2.NormalizeCoord(&bbox);
    myMesh2.FilterVertex(11);
    
    normalizeBox3(mesh1->AABB(), bbox, mAABB1);
    normalizeBox3(mesh2->AABB(), bbox, mAABB2);
    staticFilterP(mAABB1, 11);
    staticFilterP(mAABB2, 11);

    // initialize the para of the first cell
    PolygonPtrList pool1, pool2;

    auto &ver1 = myMesh1.Vertex();
    auto &tri1 = myMesh1.Triangle();
    int i = 0;
    for (auto &titr: tri1)
    {
        mMesh1.emplace_back(ver1[titr[0]], ver1[titr[1]], ver1[titr[2]]);
        pool1.push_back(i++);
    }

    auto &ver2 = myMesh2.Vertex();
    auto &tri2 = myMesh2.Triangle();
    i = 0;
    for (auto &titr: tri2)
    {
        mMesh2.emplace_back(ver2[titr[0]], ver2[titr[1]], ver2[titr[2]]);
        pool2.push_back(i++);
    }

    Box3 initBox(Box3::BOX_MIRROR_SCALE);
    PerformIteration(pool1, pool2, initBox);

    FixedPlaneMesh *resultMesh = new FixedPlaneMesh;


    // collect affected polygons
    for (auto &itr: mMesh1)
    {
        if (itr.bSplitted)
        {
            for (auto &itr2: itr.Fragments)
            {
                resultMesh->AddPolygon(*itr2);
                delete itr2;
            }
        }
    }

    for (auto &itr: mMesh2)
    {
        if (itr.bSplitted)
        {
            for (auto &itr2: itr.Fragments)
            {
                resultMesh->AddPolygon(*itr2);
                delete itr2;
            }
        }
    }

    for (auto &itr3: mPlanePolygon)
    {
        resultMesh->AddPolygon(itr3);
    }

    if (*output) delete *output;
    *output = resultMesh->ToBaseMesh();
    delete resultMesh;

    auto &outputPtr = *output;

    // add un-touched triangles
    for (auto &itr: mMesh1)
    {
        if (!itr.bSplitted)
        {
            outputPtr->AddTriangle(itr.Position);
        }
    }

    for (auto &itr: mMesh2)
    {
        if (!itr.bSplitted)
        {
            outputPtr->AddTriangle(itr.Position);
        }
    }

    outputPtr->DenormalizeCoord(&bbox);
}

// must be critical cell
void BSPOctree::PerformIteration(PolygonPtrList& mesh1, PolygonPtrList& mesh2
                                 , Box3& bbox)
{
    if ((mesh1.size() + mesh2.size()) < 17)
    {
        PerformBoolean(mesh1, mesh2, bbox);
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
            if (!bCriticalCells[i]) continue;
            if (isPointInBoxBias(childBBox[i], pos[0]) ||
                isPointInBoxBias(childBBox[i], pos[1]) ||
                isPointInBoxBias(childBBox[i], pos[2]))
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
            if (!bCriticalCells[i]) continue;
            if (isPointInBoxBias(childBBox[i], pos[0]) ||
                isPointInBoxBias(childBBox[i], pos[1]) ||
                isPointInBoxBias(childBBox[i], pos[2]))
            {
                XYZSplits2[i].push_back(index);
            }
        }
    }

    for (int i = 0; i < 8; i++)
    {
        if (!bCriticalCells[i]) continue;
        PerformIteration(XYZSplits1[i], XYZSplits2[i], childBBox[i]);
    }
}

void BSPOctree::SplitSpaceByXYZ(const Box3& bbox,  Box3 childBoxes[])
{
    vec3<float> minOffset, maxOffset; 
    double3 step = double3(
        staticFilter(bbox.Diagonal().x* 0.5, 11)
        , staticFilter(bbox.Diagonal().y*0.5, 11)
        ,  staticFilter(bbox.Diagonal().z*0.5, 11));

    for (int i = 0; i < 8 ; i++)
    {
        maxOffset.x = i & 1 ?  0 : -step.x; 
        maxOffset.y = i & 2 ?  0 : -step.y;
        maxOffset.z = i & 4 ?  0 : -step.z;
        minOffset.x = i & 1 ?  step.x : 0; 
        minOffset.y = i & 2 ?  step.y : 0;
        minOffset.z = i & 4 ?  step.z : 0;
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

void BSPOctree::PerformBoolean(PolygonPtrList& mesh1, PolygonPtrList& mesh2, const Box3& bbox)
{
    OutputDebugString(L"Critial");

    FixedPlane bounds[] = {FixedPlane(double3(1,0,0),-bbox.Min().x)
                            , FixedPlane(double3(-1,0,0),bbox.Max().x)
                            , FixedPlane(double3(0,1,0),-bbox.Min().y)
                            , FixedPlane(double3(0,-1,0),bbox.Max().y)
                            , FixedPlane(double3(0,0,1),-bbox.Min().z)
                            , FixedPlane(double3(0,0,-1),bbox.Max().z)};

    FixedPlane boundsNegate[] = {FixedPlane(double3(-1,0,0),-bbox.Min().x)
                            , FixedPlane(double3(1,0,0),bbox.Max().x)
                            , FixedPlane(double3(0,-1,0),-bbox.Min().y)
                            , FixedPlane(double3(0,1,0),bbox.Max().y)
                            , FixedPlane(double3(0,0,-1),-bbox.Min().z)
                            , FixedPlane(double3(0,0,1),bbox.Max().z)};

    FixedPlaneMesh myMesh1, myMesh2;
    FixedPlanePolygon poly;
    std::list<FixedPlanePolygon*> polyPool;

    // we know the first element of fragments is the whole plane
    // we first clip from the second element to the end, producing remaining fragments
    // then we find the fragments that perform bsp merge.

    /// convert simple mesh into plane based mesh
    for (auto index: mesh1)
    {
        MeshData *data = &(mMesh1[index]);
        if (!data->bSplitted)
        {
            data->bSplitted = true;
            data->OrigPolygon = FixedPlanePolygon(data->Position[0],
                data->Position[1], data->Position[2]);
            FixedPlanePolygon *root = new FixedPlanePolygon(data->OrigPolygon);
            data->Fragments.push_back(root);
        }

        // calculate the outer polygons: Fragments
        auto &frag = data->Fragments;
        for (auto itr = frag.begin(); itr != frag.end(); )
        {
            bool bRemove = false;
            for (int i = 0; i < 6; i++)
            {
                if (polyPool.empty() || polyPool.back()->Size() != 0)
                    polyPool.push_back(new FixedPlanePolygon);
                (*itr)->ClipByPlaneNoFront(bounds[i], *(polyPool.back()));
                if ((*itr)->Size() == 0)
                {
                    itr = frag.erase(itr);
                    bRemove = true;
                    break;
                }
            }
            if (!bRemove) itr++;
        }
        if (!polyPool.empty() && polyPool.back()->Size() == 0) polyPool.pop_back();
        frag.swap(polyPool);
        polyPool.clear();

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

    for (auto index: mesh2)
    {
        MeshData *data = &(mMesh2[index]);
        if (!data->bSplitted)
        {
            data->bSplitted = true;
            data->OrigPolygon = FixedPlanePolygon(data->Position[0],
                data->Position[1], data->Position[2]);
            FixedPlanePolygon *root = new FixedPlanePolygon(data->OrigPolygon);
            data->Fragments.push_back(root);
        }

        // calculate the outer polygons: Fragments
        auto &frag = data->Fragments;
        for (auto itr = frag.begin(); itr != frag.end();)
        {
            bool bRemove = false;
            for (int i = 0; i < 6; i++)
            {
                if (polyPool.empty() || polyPool.back()->Size() != 0)
                    polyPool.push_back(new FixedPlanePolygon);
                (*itr)->ClipByPlaneNoFront(bounds[i], *(polyPool.back()));
                if ((*itr)->Size() == 0)
                {
                    itr = frag.erase(itr);
                    bRemove = true;
                    break;
                }
            }
            if (!bRemove) itr++;
        }

        if (!polyPool.empty() && polyPool.back()->Size() == 0) polyPool.pop_back();
        frag.swap(polyPool);
        polyPool.clear();

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

    FixedBSPTree *tree1, *tree2;
    tree1 = myMesh1.ToBSPTree();
    tree1->FormSubHyperPlane(bbox);
    //tree1->OutputDebugInfo("d:\\what1.c");
    tree2 = myMesh2.ToBSPTree();
    tree2->FormSubHyperPlane(bbox);
    //tree1->OutputDebugInfo("d:\\what2.c");

    auto op = mOperation;
    if (mOperation == FixedBSPTree::OP_DIFFERENCE)
    {
        op = FixedBSPTree::OP_INTERSECT;
        tree2->Negate();
    }

    FixedBSPTree *mergeTree = tree1->Merge(tree2, op);
    delete tree1, tree2;

    mergeTree->GetPolygons(mPlanePolygon);
    delete mergeTree;
}


} // namespace GS