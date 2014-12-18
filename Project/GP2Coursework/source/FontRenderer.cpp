//
//  FontRenderer.cpp
//  GP2BaseCode
//
//  Created by Brian on 18/11/2014.
//  Copyright (c) 2014 Glasgow Caledonian University. All rights reserved.
//

#include "FontRenderer.h"
#include "Font.h"
#include "Shader.h"

#ifdef _DEBUG && WIN32
const std::string ASSET_PATH = "../assets/";
const std::string SHADER_PATH = "shaders/";
const std::string TEXTURE_PATH = "textures/models/";
const std::string FONT_PATH = "fonts/";
#elif __APPLE__
const std::string ASSET_PATH;
const std::string SHADER_PATH;
const std::string FONT_PATH;
#else
const std::string ASSET_PATH = "/assets/";
const std::string SHADER_PATH = "shaders/";
const std::string FONT_PATH = "fonts/";
#endif

const std::string FontRenderer::DEFAULT_FONT_FILENAME="OratorStd.otf";
const int FontRenderer::DEFAULT_FONT_SIZE=12;
const int FontRenderer::BUFFER_SIZE=1000;
std::string FontRenderer::DEFAULT_VERTEX_SHADER_FILENAME="textureVS.glsl";
std::string FontRenderer::DEFAULT_FRAGMENT_SHADER_FILENAME="textureFS.glsl";

const FontVertex FontRenderer::m_QuadVertices[3]={
    {vec3(0.0f,0.0f,1.0f),vec2(0.0f,0.0f)},
    {vec3(100.0f,100.0f,1.0f),vec2(1.0f,1.0f)},
    {vec3(0.0f,100.0f,1.0f),vec2(0.0f,1.0f)}
};


 const int FontRenderer::m_QuadIndices[3]=
{
    0,1,2
};

FontRenderer::FontRenderer()
{
    m_DefaultFont=NULL;
    m_CurrentFont=NULL;
    m_FontVB=0;
    m_FontEB=0;
    m_ShaderProgram=0;
}


FontRenderer::~FontRenderer()
{
    
}

void FontRenderer::destroy()
{
    if (m_ShaderProgram)
    {
        glDeleteProgram(m_ShaderProgram);
    }
    if (m_FontVB)
    {
        glDeleteBuffers(1,&m_FontVB);
    }
    if (m_FontEB)
    {
        glDeleteBuffers(1,&m_FontEB);
    }
    if (m_DefaultFont)
    {
        m_DefaultFont->destroy();
        delete m_DefaultFont;
        m_DefaultFont=NULL;
    }
}

void FontRenderer::init(int windowWidth,int windowHeight)
{
    const std::string fontFilename=ASSET_PATH+FONT_PATH+DEFAULT_FONT_FILENAME;
    m_DefaultFont=new Font();
    m_DefaultFont->init(fontFilename,DEFAULT_FONT_SIZE);
    m_CurrentFont=m_DefaultFont;
    
    //create an ortho matrix
	m_OrthoMatrix = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 100.0f);
	m_ViewMatrix = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    
    //Create buffer
	glGenBuffers(1, &m_FontVB);
    glBindBuffer(GL_ARRAY_BUFFER,m_FontVB);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(FontVertex), (void**)&m_QuadVertices, GL_DYNAMIC_DRAW);
    
    
	glGenBuffers(1, &m_FontEB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_FontEB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(int), (void**)&m_QuadIndices, GL_DYNAMIC_DRAW);
    
    //Create shader
	std::string vsFilename = ASSET_PATH + SHADER_PATH + DEFAULT_VERTEX_SHADER_FILENAME;
	GLuint vertexShaderProgram = 0;
	vertexShaderProgram = loadShaderFromFile(vsFilename, VERTEX_SHADER);
    
	std::string fsFilename = ASSET_PATH + SHADER_PATH + DEFAULT_FRAGMENT_SHADER_FILENAME;
	GLuint fragmentShaderProgram = 0;
	fragmentShaderProgram = loadShaderFromFile(fsFilename, FRAGMENT_SHADER);
    
	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, vertexShaderProgram);
	glAttachShader(m_ShaderProgram, fragmentShaderProgram);
	glLinkProgram(m_ShaderProgram);
	checkForLinkErrors(m_ShaderProgram);
    
	//now we can delete the VS & FS Programs
	glDeleteShader(vertexShaderProgram);
	glDeleteShader(fragmentShaderProgram);
    
    //Turn on Alpha Blending
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void FontRenderer::drawString(int x,int y,const std::string& text)
{
	glDisable(GL_DEPTH_TEST);
    m_Vertices.clear();
	//for every character generate a translation vector, store in the VB
	vec3 startPosition=vec3(x,y,0.0f);
    int characterWidth=m_CurrentFont->getFontWidth();
    int characterHeight=m_CurrentFont->getFontHeight();
    int atlasWidth=m_CurrentFont->getAtlasWidth();
    int atlasHeight=m_CurrentFont->getAtlasHeight();
    
	float offsetX = ((float)characterWidth / (float)atlasWidth) / 2.0f;

    for(auto charIter=text.begin();charIter!=text.end();charIter++)
    {
        float index=(float)(*charIter);
        
        float centreX=(float)(characterWidth/atlasWidth)*index;
        

        
        /*
        vertex[3].position=glm::vec3(startPosition.x,startPosition.y+characterHeight,2.0f);
        vertex[3].texCoords=glm::vec2(centreX-offsetX,1.0f);
        
        vertex[4].position=glm::vec3(startPosition.x+characterWidth,startPosition.y,2.0f);
        vertex[4].texCoords=glm::vec2(centreX+offsetX,0.0f);
        
        vertex[5].position=glm::vec3(startPosition.x+characterWidth,startPosition.y+characterHeight,2.0f);
        vertex[5].texCoords=glm::vec2(centreX+offsetX,1.0f);
         
        
        for (int i=0;i<3;i++)
        {
            m_Vertices.push_back(vertex[i]);
        }
        
        startPosition.x+=(float)characterWidth;*/
    }
    
    //
    glUseProgram(m_ShaderProgram);
    //copy vertices
    glBindBuffer(GL_ARRAY_BUFFER,m_FontVB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_FontEB);
    
    //glBufferSubData(GL_ARRAY_BUFFER,0,3*sizeof(FontVertex),(void**)&m_QuadVertices);
    
    glBindAttribLocation(m_ShaderProgram, 0, "vertexPosition");
    glBindAttribLocation(m_ShaderProgram,1,"vertexTexCoords");
    
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(
                          0,  // attribute
                          3,                  // number of elements per vertex, here (x,y)
                          GL_FLOAT,           // the type of each element
                          GL_FALSE,           // take our values as-is
                          sizeof(FontVertex),                  // no extra data between each position
                          0                   // offset of first element
                          );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
                          1,  // attribute
                          2,                  // number of elements per vertex, here (x,y)
                          GL_FLOAT,           // the type of each element
                          GL_FALSE,           // take our values as-is
                          sizeof(FontVertex),                  // no extra data between each position
                          (void**)sizeof(glm::vec3)                   // offset of first element
                          );
    
    GLint MVPLocation = glGetUniformLocation(m_ShaderProgram, "MVP");
    GLint textureLocation=glGetUniformLocation(m_ShaderProgram,"texture0");
    
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_CurrentFont->getAtlas());
    
    glUniform1i(textureLocation, 0);
	mat4 MVP = mat4(1.0f);
	glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(MVP));
    
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH_TEST);
}
