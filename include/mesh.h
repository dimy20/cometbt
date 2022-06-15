#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

class Vertex{
	public:
		Vertex(const glm::vec3& pos){
			m_pos = pos;
		}
	private:
		glm::vec3 m_pos;
};

class Mesh{
	public:
		Mesh(Vertex * vertices, int size);
		~Mesh();
		void draw();
	private:
		enum{
			POSITION_VB,
			NUM_BUFFERS
		};
		GLuint m_vertexArrayObject;
		GLuint m_vertexArrayBuffers[NUM_BUFFERS];
		unsigned int m_drawCount;
};
