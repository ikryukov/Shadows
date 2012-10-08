#include "ObjLoader.h"

#include <fstream>
#include <algorithm>
#include <assert.h>
#include <cmath>

float abs(float x)
{
	return (x < 0)?-x:x;
}

bool operator==(const Vertex& a, const Vertex& b)
{
	return
	(abs(a.Position.x - b.Position.x) < EPS &&
	 abs(a.Position.y - b.Position.y) < EPS &&
	 abs(a.Position.z - b.Position.z) < EPS &&
	 abs(a.Normal.x - b.Normal.x) < EPS &&
	 abs(a.Normal.y - b.Normal.y) < EPS &&
	 abs(a.Normal.z - b.Normal.z) < EPS &&
	 abs(a.TexCoord.x - b.TexCoord.x) < EPS &&
	 abs(a.TexCoord.y - b.TexCoord.y) < EPS);
}

void ObjLoader::Load(std::string& filename, ObjModel& model)
{
	printf("start load obj\n");
	std::ifstream m_stream(filename);
	//m_stream = inp;
	std::string line;
	while(std::getline(m_stream, line, '\n'))
	{
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "v")
		{
			float3_t pos;
			iss >> pos.x >> pos.y >> pos.z;
			m_positions.push_back(pos);
		}
		else if (token == "vn")
		{
			float3_t norm;
			iss >> norm.x >> norm.y >> norm.z;
			m_normals.push_back(norm);
		
		}
		else if (token == "vt")
		{
			float2_t tex;
			iss >> tex.u >> tex.v;
			m_texcoords.push_back(tex);
		}
		else if (token == "f")
		{
			std::string tmp;
			std::getline(iss, tmp, '\n');
			ParseFace(tmp);
		}
		else
		{
			continue;
		}

	}
	model.vertices = m_vertices;
	model.indices = m_indices;
	model.m_indexCount = model.indices.size();
	model.m_vertexBuffer = model.vertices.size();
	printf("finish load obj\n");
	printf("vertices count: %ld\n", model.vertices.size());
	printf("indices count: %ld\n", model.indices.size());
	m_indices.clear();
	m_vertices.clear();
}

void ObjLoader::ParseFace(std::string& str)
{
	int count = std::count(str.begin(), str.end(), '/');
	bool hasTex = std::strstr(str.c_str(), "//") == NULL;
	bool hasNormal = count > 3;
	
	if ( count > 0 )
	{
		std::replace(str.begin(), str.end(), '/', ' ');
		std::istringstream ss(str);

		for (int i=0;i<3;++i)
		{
			unsigned int vi, ni, ti;
			ss >> vi;
			if (hasTex) ss >> ti;
			if (hasNormal) ss >> ni;

			Vertex v;
			v.Color = vec4(0.2, 0.4, 0.5, 1.0);
			v.Position.x = m_positions[vi-1].x;
			v.Position.y = m_positions[vi-1].y;
			v.Position.z = m_positions[vi-1].z;

			if (hasTex)
			{
				v.TexCoord.x = m_texcoords[ti-1].u;
				v.TexCoord.y = m_texcoords[ti-1].v;
			}
			else
			{
				v.TexCoord.x = 0;
				v.TexCoord.y = 0;
			}

			if (hasNormal)
			{
				v.Normal.x = m_normals[ni-1].x;
				v.Normal.y = m_normals[ni-1].y;
				v.Normal.z = m_normals[ni-1].z;
			}
			else
			{
				v.Normal.x = 0;
				v.Normal.y = 0;
				v.Normal.z = 0;
			}
			std::vector<Vertex>::iterator it = std::find(m_vertices.begin(), m_vertices.end(), v);
			if (it == m_vertices.end())
			{
				m_vertices.push_back(v);
				m_indices.push_back(m_vertices.size() - 1);
			}
			else
			{
				m_indices.push_back(it - m_vertices.begin());
			}
		}
	}
	else
	{
		std::istringstream ss(str);
		unsigned int v;
		for (int i=0;i<3;++i)
		{
			ss >> v;
			m_indices.push_back(v-1);
		}
	}

}