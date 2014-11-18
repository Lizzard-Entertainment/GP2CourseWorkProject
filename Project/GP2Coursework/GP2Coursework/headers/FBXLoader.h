#ifndef FBXLoader_H
#define FBXLoader_H

#include <fbxsdk.h>
#include <string>
#include "Vertex.h"

class GameObject;

GameObject * loadFBXFromFile(const std::string& filename); 

void processNode(FbxNode *node, GameObject *rootGo);
void processAttribute(FbxNodeAttribute * attribute, GameObject * go);

void processMesh(FbxMesh * mesh, GameObject *go);

void processMeshNormals(FbxMesh * mesh, Vertex * verts, int numVerts);

void processMeshTextureCoords(FbxMesh * mesh, Vertex * verts, int numVerts);

#endif