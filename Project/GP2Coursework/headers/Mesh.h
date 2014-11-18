#ifndef Mesh_h
#define Mesh_h

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>

#include "Component.h"

class Mesh:public Component
{
public:
    Mesh();
    ~Mesh();
    
    void init();
    void destroy();
    void bind();
    
    void copyVertexData(int count,int stride,void ** data);
    void copyIndexData(int count,int stride,void ** data);
    
    int getVertexCount();
    int getIndexCount();
protected:
private:
    int m_VertexCount;
    int m_IndexCount;
    GLuint m_VBO;
    GLuint m_EBO;
    GLuint m_VAO;
    
};

#endif
