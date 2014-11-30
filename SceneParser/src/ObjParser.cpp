#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ObjParser.h"

using namespace std;

ObjParser::~ObjParser()
{
	Release();
}


void ObjParser::Release()
{
	mVertexs.clear();
	mNormals.clear();
	mFaces.clear();
	mGroups.clear();
}

bool ObjParser::Initialize()
{
	return true;
}

bool ObjParser::LoadScene(const char* pFilename)
{
	int vertexCount, textureCount, normalCount, faceCount;
	if (!ReadFileCounts(pFilename, vertexCount, textureCount,normalCount,faceCount))
		return false ;
	mVertexs.reserve(vertexCount);
	mNormals.reserve(normalCount);
	mFaces.reserve(faceCount);
	return ReadSceneData(pFilename);

}

void ObjParser::ProcessMesh(std::vector<GS::BaseMesh*>& meshs)
{
	for (int i = 0 ; i< mGroups.size(); i++)
	{
		GS::BaseMesh* pMesh= new GS::BaseMesh();
		// Assemble verties 
		GS::float4 color(0.67, 0.67, 0.67, 1.0);
		for (int j = mGroups[i].faceStart; j < mGroups[i].faceEnd; j++)
		{
            GS::VertexInfo v1, v2, v3;
            if (mFaces[j].eFaceType == 0)
            {
                v1.pos = mVertexs[mFaces[j].vIndex[2]];
                v2.pos = mVertexs[mFaces[j].vIndex[1]];
                v3.pos = mVertexs[mFaces[j].vIndex[0]];
                v1.normal = v2.normal = v3.normal = GS::NormalOf(v1.pos, v2.pos, v3.pos);
                v1.color = v2.color = v3.color = color;
            }else {
                v1 = GS::VertexInfo(mVertexs[mFaces[j].vIndex[2]],mNormals[mFaces[j].nIndex[2]],color);
                v2 = GS::VertexInfo(mVertexs[mFaces[j].vIndex[1]],mNormals[mFaces[j].nIndex[1]],color);
                v3 = GS::VertexInfo(mVertexs[mFaces[j].vIndex[0]],mNormals[mFaces[j].nIndex[0]],color);
            }
			pMesh->Add(v1, v2, v3);
		}
		if (pMesh->PrimitiveCount())
		{
			pMesh->GenID();
		//	pMesh->GenSurface();
			pMesh->GenAABB(true);
			meshs.push_back(pMesh);
		}else delete pMesh;
		
	}
}

void ObjParser::ProcessScene(std::vector<GS::BaseMesh*>& meshs, std::vector<GS::Light*>& lights,GS::Camera& camera)
{
	ProcessMesh(meshs);
}



bool ObjParser::ReadFileCounts(const char* pFilename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
{
	ifstream fin;
	char input;


	// Initialize the counts.
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;

	// Open the file.
	fin.open(pFilename);

	// Check if it was successful in opening the file.
	if(fin.fail() == true)
	{
		return false;
	}

	// Read from the file and continue to read until the end of the file is reached.
	fin.get(input);
	while(!fin.eof())
	{
		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if(input == 'v')
		{
			fin.get(input);
			if(input == ' ') { vertexCount++; }
			if(input == 't') { textureCount++; }
			if(input == 'n') { normalCount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if(input == 'f')
		{
			fin.get(input);
			if(input == ' ') { faceCount++; }
		}
		
		// Otherwise read in the remainder of the line.
		while(input != '\n')
		{
			fin.get(input);
			if (fin.eof())
			{
				fin.close();
				return true;
			}
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	return true;
}


bool ObjParser::ReadSceneData(const char* pFilename )
{

	ifstream fin;
	char input, input2;


	// Open the file.
	fin.open(pFilename);

	// Check if it was successful in opening the file.
	if(fin.fail() == true)
	{
		return false;
	}
	GS::double3 vertex;
	GS::double3 normal;
	GroupType group;
	FaceType face;
	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	fin.get(input);
	while(!fin.eof())
	{

		if(input == 'v')
		{
			fin.get(input);

			// Read in the vertices.
			if(input == ' ') 
			{ 
				fin >> vertex.x >> vertex.y >> vertex.z;

				// Invert the Z vertex to change to left hand system.
				vertex.z =vertex.z * -1.0f;
				mVertexs.push_back(vertex);
			}

			//// Read in the texture uv coordinates.
			//if(input == 't') 
			//{ 
			//	fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

			//	// Invert the V texture coordinates to left hand system.
			//	texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
			//	texcoordIndex++; 
			//}

			// Read in the normals.
			if(input == 'n') 
			{ 
				
				fin >> normal.x >> normal.y >> normal.z;

				// Invert the Z normal to change to left hand system.
				normal.z = normal.z * -1.0f;
				mNormals.push_back(normal);
			}
		}
		if (input == 'g')
		{ 
			string name;
			do {
				fin.get(input);
			}while (input == ' ');
			while (input != '\n')
			{
				if (input != ' ')
					name+= input;
				fin.get(input);
			}
			if (name.compare("default"))
			{
				if (group.faceStart == -1)
					group.faceStart = mFaces.size();
				else if (group.faceEnd < group.faceStart)
				{
					group.faceEnd = mFaces.size();
				    if (group.faceEnd > group.faceStart) 
						mGroups.push_back(group);
					group.faceStart = mFaces.size();
					group.faceEnd = -1;
				} 

			}
		}
		// Read in the faces.
		if(input == 'f') 
		{
			ReadFaceData(fin);
		}
		//bool bFileEnd = false ;
		//// Read in the remainder of the line.
		//while(input != '\n')
		//{
		//	fin.get(input);
		//	if (fin.eof())
		//	{
		//		bFileEnd = true;
		//		break;
		//	}
		//}
        bool bFileEnd = ReadLineRemainder(fin, input); 
		if (bFileEnd)
			break;
		// Start reading the beginning of the next line.
		//fin.get(input);
	}
	if (group.faceEnd < group.faceStart)
	{
		group.faceEnd  = mFaces.size();
		if (group.faceEnd > group.faceStart)
			mGroups.push_back(group);
	}
	// Close the file.
	fin.close();
	return true;
}


 bool ObjParser::ReadLineRemainder(ifstream& ifs, char& input)
 {
       // Read in the remainder of the line.
    bool bEnd = false ;
    for (;;)
    {
        ifs.get(input);
        if (ifs.eof())
            return true;
        if (input == '\r' || input == '\n')
        {
            bEnd = true;
            continue;
        }
        if (bEnd)
            return false;
    }

 }


void ObjParser::ReadFaceData(std::ifstream& ifs)
{
    char input, input2, input3;
    FaceType face;
    ifs.get(input);
    if(input == ' ')
    {  
        string szLine;
        getline(ifs, szLine);
        istringstream istr(szLine);
        string subStr[3];
        istr >> subStr[0] >> subStr[1] >>subStr[2];
        int pos = -1;
        for (int i = 0 ; i < 3;i++) 
        {
            if ((pos = subStr[i].find("//") )!= string::npos)
            {
                subStr[i][pos++] = ' '; 
                subStr[i][pos] = ' '; 
                face.eFaceType = 2;
            }
            else if ((pos = subStr[i].find('/') )!= string::npos){
                subStr[i][pos] = ' ';
                if ((pos = subStr[i].find('/', pos+1)) != string::npos)
                {
                    subStr[i][pos] = ' ';
                    face.eFaceType = 1;
                }
                else face.eFaceType = 3;
            }else 
                face.eFaceType = 0;
            istr.clear();
            istr.str(subStr[i]);
            istr>> face.vIndex[i];
            if (face.eFaceType == 1)
                istr>> face.tIndex[i]  >> face.nIndex[i] ;
            else if (face.eFaceType == 2)
                istr>> face.tIndex[i];
            else if (face.eFaceType == 3)
                istr>>face.tIndex[i] ;

        }
        mFaces.push_back(face);
    }
}