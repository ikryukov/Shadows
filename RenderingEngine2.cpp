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

#define STRINGIFY(A)  #A
#include "Shadows/Shaders/Simple.frag"
#include "Shadows/Shaders/Simple.vert"

#include "Shadows/Shaders/Quad.frag"
#include "Shadows/Shaders/Quad.vert"

using namespace std;

mat4 VerticalFieldOfView(float degrees, float aspectRatio, float near, float far)
{
	float top = near * std::tan(degrees * Pi / 360.0f);
	float bottom = -top;
	float left = bottom * aspectRatio;
	float right = top * aspectRatio;
	return mat4::Frustum(left, right, bottom, top, near, far);
}

static mat4 LookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	vec3 z = (eye - target).Normalized();
	vec3 x = up.Cross(z).Normalized();
	vec3 y = z.Cross(x).Normalized();
	mat4 m;
	m.x = vec4(x.x, x.y, x.z, 0);
	m.y = vec4(y.x, y.y, y.z, 0);
	m.z = vec4(z.x, z.y, z.z, 0);
	m.w = vec4(0, 0, 0, 1);
	vec4 eyePrime = m * vec4(-eye.x , -eye.y, -eye.z, 1.0);
	m = m.Transposed();
	m.w = eyePrime;
	return m;
}

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
    void Render() const;
    void UpdateAnimation(float timeStep) {}
    void OnRotate(DeviceOrientation newOrientation) {}
    void OnFingerUp(vec2 location);
    void OnFingerDown(vec2 location);
    void OnFingerMove(vec2 oldLocation, vec2 newLocation);
	void SetResourcePath(std::string& path);
	void SetPivotPoint(float x, float y);
private:
	void renderModel(const GLuint program, const ObjModel& model) const;
    GLuint BuildShader(const char* source, GLenum shaderType) const;
    GLuint BuildProgram(const char* vShader, const char* fShader) const;
    GLfloat m_rotationAngle;
    GLfloat m_scale;
    vec2 m_pivotPoint;
    GLuint m_simpleProgram;
	GLuint m_quadProgram;
    GLuint m_framebuffer;
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
	GLuint m_textureShadow;
	GLuint m_fboShadow;
	GLuint m_rboShadow;
	
	vec2 screen;
	ObjModel obj;
	ObjModel quad;
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

IRenderingEngine* CreateRenderer2()
{
    return new RenderingEngine2();
}

RenderingEngine2::RenderingEngine2() : m_rotationAngle(0), m_scale(1)
{
    // Create & bind the color buffer so that the caller can allocate its space.
    glGenRenderbuffers(1, &m_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
}

void RenderingEngine2::Initialize(int width, int height)
{
    //m_pivotPoint = ivec2(width / 2, height / 2);
	printf("x = %f, y = %f\n", m_pivotPoint.x, m_pivotPoint.y);
	string filename = string("cornell_box.obj");
	string file = resourcePath + string("/") + filename;
	ObjLoader loader;
	
	loader.Load(file, obj);
	obj.createVBO();

	file = resourcePath + string("/") + string("quad.obj");
	loader.Load(file, quad);
	quad.createVBO();
    
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
    //glEnable(GL_CULL_FACE);
	
	screen.x = width;
	screen.y = height;
	
    m_simpleProgram = BuildProgram(SimpleVertexShader, SimpleFragmentShader);
	m_quadProgram = BuildProgram(QuadVertexShader, QuadFragmentShader);
	
    glUseProgram(m_simpleProgram);
    
    // Set the projection matrix.
    GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
	mat4 projectionMatrix = VerticalFieldOfView(45, (width + 0.0) / height, 0.1, 1000.0);
    glUniformMatrix4fv(projectionUniform, 1, 0, projectionMatrix.Pointer());
	m_light.Position = vec3(10, 10, 10);
	
	//Create the shadow map texture
	glGenTextures(1, &m_textureShadow);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	
	// Create the depth texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	
	// Set the textures parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
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

void RenderingEngine2::Render() const
{

    mat4 rotation = mat4::Rotate(m_rotationAngle);
    mat4 scale = mat4::Scale(m_scale);
    mat4 translation = mat4::Translate(0, 0, -7);
    GLint modelviewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
	GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
	
	mat4 modelviewMatrix = scale * rotation * translation * LookAt(vec3(0,0,7), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0));


	GLenum status;
	mat4 projectionMatrix;
	
	// Shadow pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Shadow pass: ");
		printf("failed to make complete framebuffer object %x\n", status);
	}
	
	glClear(GL_DEPTH_BUFFER_BIT);
	// TODO change to shadowmap size
	projectionMatrix = VerticalFieldOfView(90.0, (screen.x + 0.0) / screen.y, 0.1, 1000.0);
	modelviewMatrix = scale * rotation * translation * LookAt(vec3(5, 5, 5), vec3(0.0, 0.0, 0.0), vec3(-5, -5, 5));
	
	glUniformMatrix4fv(projectionUniform, 1, 0, projectionMatrix.Pointer());
    glUniformMatrix4fv(modelviewUniform, 1, 0, modelviewMatrix.Pointer());
	
	renderModel(m_simpleProgram, obj);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	 
	
	
	// Main pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Main pass: ");
		printf("failed to make complete framebuffer object %x\n", status);
	}

    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelviewMatrix = scale * rotation * translation * LookAt(vec3(0,4,7), vec3(0.0, 0.0, 0.0), vec3(0, 7, 4));
	projectionMatrix = VerticalFieldOfView(90.0, (screen.x + 0.0) / screen.y, 0.1, 1000.0);
    
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	glUniform1i(glGetUniformLocation(m_quadProgram, "sShadow"), 0);
	renderModel(m_quadProgram, quad);
	
}

void RenderingEngine2::renderModel(const GLuint program, const ObjModel &model) const
{
	glUseProgram(program);
	GLuint positionSlot = glGetAttribLocation(program, "Position");
    GLuint colorSlot = glGetAttribLocation(program, "SourceColor");
	GLuint normalSlot = glGetAttribLocation(program, "Normal");
	GLuint texSlot = glGetAttribLocation(program, "TexCoord");
	GLsizei stride = sizeof(Vertex);
	
	const GLvoid* normalOffset = (GLvoid*) (sizeof(vec3));
    const GLvoid* colorOffset = (GLvoid*) (sizeof(vec3) * 2);
    const GLvoid* texOffset = (GLvoid*) (sizeof(vec3) * 2 + sizeof(vec4));
	const GLvoid* bodyOffset = 0;
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.m_indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, model.m_vertexBuffer);
	
	glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, colorOffset);
    glVertexAttribPointer(normalSlot, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
    glVertexAttribPointer(texSlot, 2, GL_FLOAT, GL_FALSE, stride, texOffset);
	
	glEnableVertexAttribArray(positionSlot);
	glEnableVertexAttribArray(normalSlot);
    glEnableVertexAttribArray(colorSlot);
	glEnableVertexAttribArray(texSlot);
	
    glDrawElements(GL_TRIANGLES, model.m_indexCount, GL_UNSIGNED_SHORT, bodyOffset);
	
    glDisableVertexAttribArray(colorSlot);
    glDisableVertexAttribArray(positionSlot);
	glDisableVertexAttribArray(normalSlot);
	glDisableVertexAttribArray(texSlot);
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

GLuint RenderingEngine2::BuildShader(const char* source, GLenum shaderType) const
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return shaderHandle;
}

GLuint RenderingEngine2::BuildProgram(const char* vertexShaderSource,
                                      const char* fragmentShaderSource) const
{
    GLuint vertexShader = BuildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return programHandle;
}
