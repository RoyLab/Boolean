#ifndef _OBJ_PARSER_H
#define _OBJ_PARSER_H
#include "SceneParser.h"

class ObjParser : public SceneParser{
typedef struct
{
    int eFaceType; // 0: only vertex  1: vertex , texture  and normal ; 
                  //2: vertex and normal; 3: vertex and texture  
	int vIndex[3];
	int tIndex[3];
	int nIndex[3];
}FaceType;
struct GroupType{
	int faceStart;
	int faceEnd; 
	GroupType()
		:faceStart(-1)
		,faceEnd(-1)
	{}
};
public :
	virtual ~ObjParser();
	bool Initialize();
	bool LoadScene(const char* pFilename) ; 
	void ProcessScene(std::vector<GS::BaseMesh*>& meshs, std::vector<GS::Light*>& lights,GS::Camera& camera);
	void Release() ; 
private :
	void ObjParser::ProcessMesh(std::vector<GS::BaseMesh*>& meshs);
	bool ReadFileCounts(const char* pFilename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount);
	bool ReadSceneData(const char* pFilename );
    void ReadFaceData(std::ifstream& ifs);
    bool ReadLineRemainder(std::ifstream& ifs, char &input);
private : 
    std::vector<GS::double3> mVertexs;
	std::vector<GS::double3> mNormals;
	std::vector<FaceType>  mFaces;
    std::vector<GroupType> mGroups;

};


#endif 