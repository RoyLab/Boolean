#include <assert.h>
#include <string>
#include "ObjParser.h"
#include "FbxParser.h"


SceneParser* SceneParser::GetParser(const char* pFileName)
{
	assert(pFileName);
	std::string sName = pFileName;
	int nPos = sName.find_last_of('.');
	if (nPos!= -1)
	{
		std::string extName = sName.substr(nPos+1);
		if (extName =="fbx")
			return new FbxParser;
		else if (extName =="obj")
			return new ObjParser;
	}
	return NULL;
	
}