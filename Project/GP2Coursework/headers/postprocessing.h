#ifndef POST_PROCESSING_H
#define POST_PROCESSING_H

//Headers
#include <GL/glew.h>
#include <string>
#include <SDL_opengl.h>
#include <gl/GLU.h>

class PostProcessing
{
public:
	PostProcessing();
	~PostProcessing();

	void init(int width, int height,std::string& vertexShaderFilename,std::string& fragmentShaderFilename);
	void changeFragmentShaderFilename(std::string& fragmentShaderFilename, std::string& path);

	void bind();
	void draw();

	void destroy();

	GLint getUniformVariableLocation(const std::string& name);

private:
	void createShader(std::string& vertexShaderFilename, std::string& fragmentShaderFilename);
	void createFullScreenQuad();
	void createFramebuffer(int width,int height);

	GLuint m_FrameBufferObject;
	GLuint m_DepthBufferObject;
	GLuint m_FBOTexture;

	GLuint m_FullScreenVBO;

	std::string m_VertexShader;

	GLuint m_PostProcessingProgram;
	int m_Width;
	int m_Height;
};

#endif