#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "IRenderingEngine.hpp"
#include "Matrix.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include "ObjLoader.h"
#include "MathUtils.h"
#include "ShaderUtils.h"

#ifndef GL_TEXTURE_COMPARE_MODE_EXT
#   define GL_TEXTURE_COMPARE_MODE_EXT      0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC_EXT
#   define GL_TEXTURE_COMPARE_FUNC_EXT      0x884D
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE_EXT
#   define GL_COMPARE_REF_TO_TEXTURE_EXT    0x884E
#endif

using namespace std;

struct Light {
	vec3 Position;
	Light() {}
};

struct RT {
	GLuint rbo;
	GLuint fbo;
	RT() {}
};

class RenderingEngine2 : public IRenderingEngine {
public:
    RenderingEngine2();
    void Initialize(int width, int height);
    void Render();
    void UpdateAnimation(float timeStep) {}
    void OnRotate(DeviceOrientation newOrientation) {}
    void OnFingerUp(vec2 location);
    void OnFingerDown(vec2 location);
    void OnFingerMove(vec2 oldLocation, vec2 newLocation);
	void SetResourcePath(std::string& path);
	void SetPivotPoint(float x, float y);
private:

	void shadowPass();
	void mainPass();
	
    GLfloat m_rotationAngle;
    GLfloat m_scale;
    vec2 m_pivotPoint;
    
	GLuint m_simpleProgram;
	GLuint m_quadProgram;
	GLuint m_shadowMapProgram;
	
    GLuint m_framebuffer;
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
	GLuint m_textureShadow;
	GLuint m_fboShadow;
	GLuint m_rboShadow;
	// uniforms for main pass
	GLint uniformModelviewMain;
	GLint uniformProjectionMain;
	
	// attribs for main pass
	GLint attribPositionMain;
	
	// uniforms for shadow pass
	GLint uniformModelviewShadow;
	GLint uniformProjectionShadow;
	GLint uniformLightMatrixShadow;
	GLint uniformShadowMapTextureShadow;

	// attribs for main pass
	GLint attribPositionShadow;
	GLint attribNormalShadow;
	GLint attribColorShadow;
	GLint attribTexCoordShadow;
	
	mat4 rotation;
    mat4 scale;
    mat4 translation;
	mat4 modelviewMatrix;
	mat4 projectionMatrix;
	
	mat4 lightProjectionMatrix;
	mat4 lightModelviewMatrix;
	
	vec2 screen;
	ivec2 shadowmapSize;
	ObjModel teapot;
	ObjModel palm;
	ObjModel plane;
	vector<ObjModel> objects;
	
	string resourcePath;
	Light m_light;
};

void RenderingEngine2::SetResourcePath(std::string& path)
{
	resourcePath = path;
}

void RenderingEngine2::SetPivotPoint(float x, float y)
{
	m_pivotPoint = vec2(x, y);
}

std::auto_ptr<IRenderingEngine> CreateRenderer2()
{
    return std::auto_ptr<IRenderingEngine>(new RenderingEngine2());
}

RenderingEngine2::RenderingEngine2() : m_rotationAngle(0), m_scale(1)
{
    // Create & bind the color buffer so that the caller can allocate its space.
    glGenRenderbuffers(1, &m_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
}

void RenderingEngine2::Initialize(int width, int height)
{
	printf("x = %f, y = %f\n", m_pivotPoint.x, m_pivotPoint.y);
	string filename = string("teapot.obj");
	string file = resourcePath + string("/") + filename;
	ObjLoader loader;
	
	loader.Load(file, teapot);
	teapot.createVBO();

	file = resourcePath + string("/") + string("tree.obj");
	loader.Load(file, palm);
	palm.createVBO();
    
	file = resourcePath + string("/") + string("plane.obj");
	loader.Load(file, plane);
	plane.createVBO();
	
	objects.push_back(plane);
	//objects.push_back(teapot);
	objects.push_back(palm);
	
    // Create the depth buffer.
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    // Create the framebuffer object; attach the depth and color buffers.
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    
    // Bind the color buffer for rendering.
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	
	
	screen.x = width;
	screen.y = height;
	
	shadowmapSize.x = 1024;
	shadowmapSize.y = 1024;

	string shaderPath = resourcePath + string("/Simple.vert");
	string vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Simple.frag");
	string fragmentShaderSource = loadShaderFromFile(shaderPath);
	
    m_simpleProgram = BuildProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
	
	shaderPath = resourcePath + string("/Quad.vert");
	vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Quad.frag");
	fragmentShaderSource = loadShaderFromFile(shaderPath);
	
	m_quadProgram = BuildProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

	shaderPath = resourcePath + string("/Shadow.vert");
	vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Shadow.frag");
	fragmentShaderSource = loadShaderFromFile(shaderPath);
	
	m_shadowMapProgram = BuildProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
	
	// set uniforms
    uniformModelviewMain = glGetUniformLocation(m_simpleProgram, "Modelview");
	uniformProjectionMain = glGetUniformLocation(m_simpleProgram, "Projection");
	
	uniformModelviewShadow = glGetUniformLocation(m_shadowMapProgram, "Modelview");
	uniformProjectionShadow = glGetUniformLocation(m_shadowMapProgram, "Projection");
	uniformLightMatrixShadow = glGetUniformLocation(m_shadowMapProgram, "lightMatrix");
	uniformShadowMapTextureShadow = glGetUniformLocation(m_shadowMapProgram, "shadowMapTex");
	
	// set attribs
	attribPositionMain = glGetAttribLocation(m_simpleProgram, "Position");
	
	attribPositionShadow = glGetAttribLocation(m_shadowMapProgram, "Position");
    attribColorShadow = glGetAttribLocation(m_shadowMapProgram, "SourceColor");
	attribNormalShadow = glGetAttribLocation(m_shadowMapProgram, "Normal");
	attribTexCoordShadow = glGetAttribLocation(m_shadowMapProgram, "TexCoord");

	m_light.Position = vec3(10, 10, 10);
	
	//Create the shadow map texture
	glGenTextures(1, &m_textureShadow);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	
	// Create the depth texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowmapSize.x, shadowmapSize.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	
	// Set the textures parameters
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenFramebuffers(1, &m_fboShadow);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureShadow, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("init: ");
		printf("failed to make complete framebuffer object %x\n", status);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine2::shadowPass()
{
	GLenum status;
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Shadow pass: ");
		printf("failed to make complete framebuffer object %x\n", status);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	
	lightProjectionMatrix = VerticalFieldOfView(90.0, (shadowmapSize.x + 0.0) / shadowmapSize.y, 0.1, 1000.0);
	lightModelviewMatrix = LookAt(vec3(0,4,7), vec3(0.0, 0.0, 0.0), vec3(0, 4, -7));
	glCullFace(GL_FRONT);
	glUseProgram(m_simpleProgram);
	glUniformMatrix4fv(uniformProjectionMain, 1, 0, lightProjectionMatrix.Pointer());
    glUniformMatrix4fv(uniformModelviewMain, 1, 0, lightModelviewMatrix.Pointer());
	glViewport(0, 0, shadowmapSize.x - 2, shadowmapSize.y - 2);
	
	GLsizei stride = sizeof(Vertex);
	
	const GLvoid* bodyOffset = 0;
	for (int i = 0; i < objects.size(); ++i)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].m_indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].m_vertexBuffer);
		
		glVertexAttribPointer(attribPositionMain, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*) offsetof(Vertex, Position));
		
		glEnableVertexAttribArray(attribPositionMain);
		
		glDrawElements(GL_TRIANGLES, objects[i].m_indexCount, GL_UNSIGNED_SHORT, bodyOffset);
		
		glDisableVertexAttribArray(attribPositionMain);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
}

void RenderingEngine2::mainPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Main pass: ");
		printf("failed to make complete framebuffer object %x\n", status);
	}
	
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelviewMatrix = scale * rotation * translation * LookAt(vec3(0,8,7), vec3(0.0, 0.0, 0.0), vec3(0, 8, -7));
	projectionMatrix = VerticalFieldOfView(45.0, (screen.x + 0.0) / screen.y, 0.1, 1000.0);
    mat4 offsetLight = mat4::Scale(0.5f) * mat4::Translate(0.5, 0.5, 0.5);
	
	mat4 lightMatrix = lightModelviewMatrix * lightProjectionMatrix * offsetLight;
	glUseProgram(m_shadowMapProgram);
	glUniformMatrix4fv(uniformLightMatrixShadow, 1, 0, lightMatrix.Pointer());
	glUniformMatrix4fv(uniformProjectionShadow, 1, 0, projectionMatrix.Pointer());
    glUniformMatrix4fv(uniformModelviewShadow, 1, 0, modelviewMatrix.Pointer());
	
	glViewport(0, 0, screen.x, screen.y);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	glUniform1i(uniformShadowMapTextureShadow, 0);
	
	GLsizei stride = sizeof(Vertex);
	
	const GLvoid* bodyOffset = 0;
	for (int i = 0; i < objects.size(); ++i)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].m_indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].m_vertexBuffer);
		
		glVertexAttribPointer(attribPositionShadow, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*) offsetof(Vertex, Position));
		glVertexAttribPointer(attribColorShadow, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*) offsetof(Vertex, Color));
		glVertexAttribPointer(attribNormalShadow, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*) offsetof(Vertex, Normal));
		glVertexAttribPointer(attribTexCoordShadow, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*) offsetof(Vertex, TexCoord));
		
		glEnableVertexAttribArray(attribPositionShadow);
		glEnableVertexAttribArray(attribNormalShadow);
		glEnableVertexAttribArray(attribColorShadow);
		glEnableVertexAttribArray(attribTexCoordShadow);
		
		glDrawElements(GL_TRIANGLES, objects[i].m_indexCount, GL_UNSIGNED_SHORT, bodyOffset);
		
		glDisableVertexAttribArray(attribColorShadow);
		glDisableVertexAttribArray(attribPositionShadow);
		glDisableVertexAttribArray(attribNormalShadow);
		glDisableVertexAttribArray(attribTexCoordShadow);
	}
}

void RenderingEngine2::Render()
{

    rotation = mat4::RotateY(m_rotationAngle);
    scale = mat4::Scale(m_scale);
    translation = mat4::Translate(0, 0, 0);

	// Shadow pass

	shadowPass();
	 
	// Main pass
	
	mainPass();
}

void RenderingEngine2::OnFingerUp(vec2 location)
{
    m_scale = 1.0f;
}

void RenderingEngine2::OnFingerDown(vec2 location)
{
    m_scale = 1.5f;
	printf("TOUCH x = %f, y = %f\n", location.x, location.y);
    OnFingerMove(location, location);
}

void RenderingEngine2::OnFingerMove(vec2 previous, vec2 location)
{
    vec2 direction = vec2(location - m_pivotPoint).Normalized();
    
    // Flip the Y axis because pixel coords increase towards the bottom.
    direction.y = -direction.y;
    
    m_rotationAngle = std::acos(direction.y) * 180.0f / 3.14159f;
    if (direction.x > 0)
        m_rotationAngle = -m_rotationAngle;
}
