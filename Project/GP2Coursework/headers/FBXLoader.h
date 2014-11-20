#ifndef FBXLoader_H
#define FBXLoader_H

//Headers
#include <fbxsdk.h>
#include <string>

//Our headers
#include "Vertex.h"

//Forward declaration of gameobject
class GameObject;

GameObject * loadFBXFromFile(const std::string& filename); 

void processNode(FbxNode *node, GameObject *rootGo);

void processAttribute(FbxNodeAttribute * attribute, GameObject * go);

void processMesh(FbxMesh * mesh, GameObject *go);

void processMeshNormals(FbxMesh * mesh, Vertex * verts, int numVerts);

void processMeshTextureCoords(FbxMesh * mesh, Vertex * verts, int numVerts);

void calculateTagentAndBinormals(Vertex * verts, int numVerts, int * indices, int numIndices);

#endif