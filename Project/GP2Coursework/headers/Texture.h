#ifndef TEXTURE_H
#define TEXTURE_H

//Headers
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>

GLuint convertSDLSurfaceToGLTexture(SDL_Surface * surface);

GLuint loadTextureFromFile(const std::string& filename);

//this shouldn't be used for realtime use, build on startup!
GLuint loadTextureFromFont(const std::string& fontFilename, int pointSize, const std::string& text);

//Skybox
void loadCubeMapSide(const std::string& filename, GLenum cubeSide);

#endif