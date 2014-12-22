#include "Font.h"
#include "Texture.h"
#include <sstream>

Font::Font()
{
    m_Font=NULL;
    m_FontAtlas=0;
	m_FontWidth = 0;
	m_FontHeight = 0;
	m_AtlasHeight = 0;
	m_AtlasWidth = 0;
}

Font::~Font()
{
    
}

bool Font::init(const std::string& filename,int pointSize)
{
    m_Font = TTF_OpenFont(filename.c_str(),pointSize);
    if (!m_Font)
	{
        std::cout << "Unable to load font " << filename << " " << TTF_GetError();
        return false;
	}
    //string stream to hold all characters of in ascii
	std::stringstream strStream;
	//run through ascii characters, ignore 0 - 32 as these characters are 'non-printable'
    //http://www.asciitable.com/
	for (int i = 33; i < 255; i++)
	{
		strStream << (char)i;
	}
    
    TTF_SizeText(m_Font,"a",&m_FontWidth,&m_FontHeight);

	createTextureFromString(strStream.str());

    return true;
}

void Font::destroy()
{
    if (m_FontAtlas)
    {
        glDeleteTextures(1, &m_FontAtlas);
    }
    if (m_Font)
    {
        TTF_CloseFont(m_Font);
    }
}

int Font::getFontWidth()
{
    return m_FontWidth;
}

int Font::getFontHeight()
{
    return m_FontHeight;
}

int Font::getAtlasWidth()
{
    return m_AtlasWidth;
}

int Font::getAtlasHeight()
{
    return m_AtlasHeight;
}

GLuint Font::getAtlas()
{
    return m_FontAtlas;
}

void Font::createTextureFromString(const std::string& text)
{
	SDL_Surface *textSurface = TTF_RenderText_Blended(m_Font, text.c_str(), { 255, 255, 255, 255 });
	if (!textSurface)
	{
		std::string errorMsg = TTF_GetError();
		return;
	}
    
	m_FontAtlas=convertSDLSurfaceToGLTexture(textSurface);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_AtlasWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_AtlasHeight);
}