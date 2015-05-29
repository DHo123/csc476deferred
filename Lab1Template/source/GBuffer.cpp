#include "GBuffer.h"

GBuffer::GBuffer(int width, int height) :
m_width(width),
m_height(height)
{
}

GBuffer::~GBuffer()
{
}

void GBuffer::FBO_init() {
	// Generate and bind Normal Texture
	glGenTextures(1, &m_normalsTex);
	glBindTexture(GL_TEXTURE_2D, m_normalsTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// Generate and bind Diffuse Texture
	glGenTextures(1, &m_diffuseTex);
	glBindTexture(GL_TEXTURE_2D, m_diffuseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate and bind Frame Buffer Object
	glGenFramebuffers (1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	// Attach Normal and Diffuse Textures to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normalsTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_diffuseTex, 0);


	// Generate and bind Depth Buffer
	glGenTextures(1, &m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
	
	// Generate and bind final texture w/ lights
	glGenTextures(1, &m_light);
	glBindTexture(GL_TEXTURE_2D, m_light);
  	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_light, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("ERROR\n");

	// Restore default FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::start() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

void GBuffer::geometryPass() {
	// Bind FBO for drawing
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, draw_bufs);

	// Enable writing to depth buffer
	glDepthMask(GL_TRUE);

	// Clear buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Turn on depth test to reduce rendered objects
	glEnable(GL_DEPTH_TEST);

	// Draw everything here
}

void GBuffer::geometryFinish(){
	// Stop acquiring and unbind the FBO
	glDepthMask(GL_FALSE);
}

void GBuffer::stencil(GLuint stencil) {
	glUseProgram(stencil);

	// Unbind buffer for stencil test
	glDrawBuffer(GL_NONE);

	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Use stencil buffer ONLY for depth test
	glStencilFunc(GL_ALWAYS, 0, 0);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
}

//void GBuffer::draw(Camera *camera, glm::vec3 wagonPos);


void GBuffer::lightPass(GLuint light) {
	//--------------- Bind for Light Pass ---------------------------
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_normalsTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_diffuseTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	//------------------------ End Binding --------------------------

	//-------------------Point Light Pass-----------------------------

	glUseProgram(light);
	// Set Eye World Position
	//Camera->getPosition();

	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glDisable(GL_DEPTH_TEST);

	//Blend light layer with diffuse, normal, etc.
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	
	// Render same sphere
	glUniform1i(GLSL::getUniformLocation(light, "normal_texture"), 0);
	glUniform1i(GLSL::getUniformLocation(light, "diffuse_texture"), 1);
}

void GBuffer::lightFinish() {
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glUseProgram(0);
}

void GBuffer::dirLightPass(GLuint dirLight) {
	//--------------- Bind for Light Pass ---------------------------
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_normalsTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_diffuseTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	//------------------------ End Binding --------------------------

	glUseProgram(dirLight);

	glDisable(GL_DEPTH_TEST);

	//Blend light layer with diffuse, normal, etc.
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	//------------------------Render quad----------------------------
	glUniform1i(GLSL::getUniformLocation(dirLight, "normal_texture"), 0);

	glUniform1i(GLSL::getUniformLocation(dirLight, "diffuse_texture"), 1);
	//---------------------------------------------------------------

}

void GBuffer::activate() {

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_light);
}

void GBuffer::moveToScreen() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, m_width, m_height,
		0, 0, m_width/2, m_height/2, GL_COLOR_BUFFER_BIT, GL_NEAREST);


	glReadBuffer(GL_COLOR_ATTACHMENT1);
/*
    glBlitFramebuffer(0, 0, m_width, m_height,
        0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
*/
	glBlitFramebuffer(0, 0, m_width, m_height,
		m_width/2, 0, m_width , m_height/2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glReadBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, m_width, m_height,
		m_width/2, m_height/2, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

//Normally
/*
	
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT2);
*/

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

}

