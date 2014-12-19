//
//  Font.h
//  GP2BaseCode
//
//  Created by Brian on 18/11/2014.
//  Copyright (c) 2014 Glasgow Caledonian University. All rights reserved.
//

#ifndef Font_h
#define Font_h

#include <GL/glew.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_ttf.h>


#include <string>

class Font
{
public:
    Font();
    ~Font();
    
    bool init(const std::string& filename,int pointSize);
    void destroy();
    int getFontWidth();
    int getFontHeight();
    int getAtlasWidth();
    int getAtlasHeight();
    
    GLuint getAtlas();
private:
	void createTextureFromString(const std::string& text);

	TTF_Font * m_Font; 
	GLuint m_FontAtlas;
    
    int m_FontWidth;
    int m_FontHeight;
    
    int m_AtlasWidth;
    int m_AtlasHeight;    
};

#endif
