#ifndef _FBX_PARSER_H
#define _FBX_PARSER_H
 
#include <fbxsdk.h>
#include <vector>
#include "SceneParser.h"
class FbxParser : public SceneParser{

public:
	FbxParser();
	virtual ~FbxParser();
	bool Initialize();
	bool LoadScene(const char* pFilename);
	void ProcessScene(std::vector<GS::BaseMesh*>& meshs, std::vector<GS::Light*>& lights,GS::Camera& camera);
	void Release();
private:
	void ProcessNode(FbxNode* pNode, std::vector<GS::BaseMesh*>& meshs, std::vector<GS::Light*>& lights,GS::Camera& camera);
	void ProcessMesh(FbxNode* pNode, std::vector<GS::BaseMesh*>& meshs);
	void ProcessCamera(FbxNode* pNode, GS::Camera& camera);
	void ProcessLight(FbxNode* pNode, std::vector<GS::Light*>& lights);
	void ReadVertex(FbxMesh* pMesh , int ctrlPointIndex , GS::double3& p); 
	void ReadNormal(FbxMesh* pMesh, int ctrlPointIndex , int vertexCounter , GS::double3& normal) ;
    void ReadColor(FbxMesh* pMesh, int ctrlPointIndex,int vertexCounter , GS::float4& color);
private:
	FbxManager* mpFbxManager;
	FbxScene*   mpFbxScene;
};


#endif 