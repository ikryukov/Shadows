#pragma once
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "Vector.hpp"

#include <math.h>

#include <string>
#include <vector>
#include <sstream>
#include <istream>

const float EPS = 1e-7f;

struct float3_t
{
	float x,y,z;
}; 

struct float2_t
{
	float u,v;
};

struct Vertex {
    vec3 Position;
	vec3 Normal;
    vec4 Color;
	vec2 TexCoord;
	Vertex() {}
	~Vertex() {}
	
};

struct ObjModel {
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	GLuint m_vertexBuffer;
	GLuint m_indexBuffer;
	
	GLuint m_vertexCount;
	GLuint m_indexCount;
	
	void createVBO()
	{
		// Create the VBO for the vertices.
		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
		
		// Create the VBO for indices
		glGenBuffers(1, &m_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
		
	}
	
	ObjModel() {}
	~ObjModel() {}
};

class ObjLoader
{
public:
	ObjLoader() {}

	void Load(std::string& filename, ObjModel& model);

	std::vector<Vertex>& GetVertexData() { return m_vertices; }
	std::vector<unsigned short>& GetIndexData() { return m_indices; }

private:
	void ParseFace(std::string& str);
	std::vector<float3_t> m_positions;
	std::vector<float3_t> m_normals;
	std::vector<float2_t> m_texcoords;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned short> m_indices;
};
