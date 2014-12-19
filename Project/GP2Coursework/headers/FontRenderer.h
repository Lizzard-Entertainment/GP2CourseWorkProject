//
//  FontRenderer.h
//  GP2BaseCode
//
//  Created by Brian on 18/11/2014.
//  Copyright (c) 2014 Glasgow Caledonian University. All rights reserved.
//

#ifndef FontRenderer_h
#define FontRenderer_h

#include <string>
#include <vector>
#include <glm/glm.hpp>
using glm::mat4;
using glm::vec3;
using glm::vec2;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <SDL_opengl.h>


struct FontVertex
{
    vec3 position;
    vec2 texCoords;
};

class Font;

class FontRenderer
{
public:
    FontRenderer();
    ~FontRenderer();
    
	void init(int windowWidth, int windowHeight, const std::string fontFilename, int fontSize, std::string& VertexShaderLocation, std::string& FragmentShaderLocation);
    void destroy();
    
    void drawString(int x,int y,const std::string& text);
private:

    Font *m_CurrentFont;

	mat4 m_OrthoMatrix;
	mat4 m_ViewMatrix;
    GLuint m_FontVB;
    GLuint m_FontEB;
    
    std::vector<FontVertex> m_Vertices;
    
    GLuint m_ShaderProgram;
    
    const static FontVertex m_QuadVertices[3];
    const static int m_QuadIndices[3];

};


#endif
