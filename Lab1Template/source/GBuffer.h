#pragma once
//#include <gl/glew.h>
//#include <gl/gl.h>
//#include <gl/glu.h>
#include "GLSL.h"
#include "glm/glm.hpp"
#include "Shape.h"
#include <vector>

/**
*	A Frame Buffer Object is used by OpenGL to render into a texture. Specifically this implementation assumes that the
*	rendered model will provide diffuse, position and normal at the same time in a MRT fashion
*/
class GBuffer
{
public:
	// Ctors/Dtors
	GBuffer(int width, int height);
	~GBuffer();

	// Methods
	void    start();
	void	FBO_init();
	void    geometryPass();
	void	geometryFinish();
	void	stencil(GLuint stencil);
        void    lightPass(GLuint light);
	void	lightFinish();
        void    dirLightPass(GLuint dirLight);
	void    activate();
        void    moveToScreen();

private:

	// Variables
	GLuint			m_fbo;
	GLuint			m_diffuseTex;
	GLuint			m_normalsTex;
	GLuint			m_depthBuffer;
	GLuint			m_light;

	unsigned int	m_width; // FBO width
	unsigned int	m_height; // FBO height
};
