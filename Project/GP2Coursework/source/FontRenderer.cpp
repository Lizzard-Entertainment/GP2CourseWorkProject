#include "FontRenderer.h"
#include "Font.h"
#include "Shader.h"

const FontVertex FontRenderer::m_QuadVertices[3]=
{
    {vec3(0.0f,0.0f,1.0f), vec2(0.0f,0.0f)},
    {vec3(100.0f,100.0f,1.0f), vec2(1.0f,1.0f)},
    {vec3(0.0f,100.0f,1.0f), vec2(0.0f,1.0f)}
};

const int FontRenderer::m_QuadIndices[3]= { 0,1,2 };

FontRenderer::FontRenderer()
{
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
    if (m_CurrentFont)
    {
		m_CurrentFont->destroy();
		delete m_CurrentFont;
		m_CurrentFont = NULL;
    }
}

void FontRenderer::init(int windowWidth, int windowHeight, const std::string fontFilename, int fontSize, std::string& VertexShaderLocation, std::string& FragmentShaderLocation)
{
	//Create font
	m_CurrentFont = new Font();
	m_CurrentFont->init(fontFilename, fontSize);
    
    //create an ortho matrix
	m_OrthoMatrix = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 100.0f);
	m_ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    //Create buffer
	glGenBuffers(1, &m_FontVB);
    glBindBuffer(GL_ARRAY_BUFFER,m_FontVB);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(FontVertex), (void**)&m_QuadVertices, GL_DYNAMIC_DRAW);    
    
	glGenBuffers(1, &m_FontEB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_FontEB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(int), (void**)&m_QuadIndices, GL_DYNAMIC_DRAW);
    
    //Create shader
	GLuint vertexShaderProgram = 0;
	vertexShaderProgram = loadShaderFromFile(VertexShaderLocation, VERTEX_SHADER);
    
	GLuint fragmentShaderProgram = 0;
	fragmentShaderProgram = loadShaderFromFile(FragmentShaderLocation, FRAGMENT_SHADER);
    
	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, vertexShaderProgram);
	glAttachShader(m_ShaderProgram, fragmentShaderProgram);
	glLinkProgram(m_ShaderProgram);
	checkForLinkErrors(m_ShaderProgram);
    
	//now we can delete the VS & FS Programs
	glDeleteShader(vertexShaderProgram);
	glDeleteShader(fragmentShaderProgram);
}

void FontRenderer::drawString(int x,int y,const std::string& text)
{	
	glDisable(GL_DEPTH_TEST);
	m_Vertices.clear();

	//for every character generate a translation vector, store in the VB
    //int characterWidth=m_CurrentFont->getFontWidth();
    //int characterHeight=m_CurrentFont->getFontHeight();
    //int atlasWidth=m_CurrentFont->getAtlasWidth();
    //int atlasHeight=m_CurrentFont->getAtlasHeight();
    
	//float offsetX = ((float)characterWidth / (float)atlasWidth) / 2.0f;

    //for(auto charIter=text.begin();charIter!=text.end();charIter++)
    //{
    //    float index=(float)(*charIter);        
    //    float centreX=(float)(characterWidth/atlasWidth)*index;
    //}
    
    //Bind program
    glUseProgram(m_ShaderProgram);

    //copy vertices
    glBindBuffer(GL_ARRAY_BUFFER,m_FontVB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_FontEB);
    
    glBindAttribLocation(m_ShaderProgram, 0, "vertexPosition");
    glBindAttribLocation(m_ShaderProgram, 1, "vertexTexCoords");
    
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(
                          0,  // attribute
                          2,                  // number of elements per vertex, here (x,y)
                          GL_FLOAT,           // the type of each element
                          GL_FALSE,           // take our values as-is
                          sizeof(FontVertex),                  // no extra data between each position
                          0                   // offset of first element
                          );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
                          1,						// attribute
                          2,						// number of elements per vertex, here (x,y)
                          GL_FLOAT,					// the type of each element
                          GL_FALSE,					// take our values as-is
                          sizeof(FontVertex),       // no extra data between each position
                          (void**)sizeof(glm::vec2)	// offset of first element
                          );
    
	//Get uniform locations
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
