#include "mesh.h"

Mesh::Mesh(Vertex * vertices, int n_vertices){
	m_drawCount = n_vertices;

	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);

	glGenBuffers(NUM_BUFFERS, m_vertexArrayBuffers);
	/*use this one*/
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[POSITION_VB]);
	glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof(*vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glBindVertexArray(0);
};

Mesh::~Mesh(){
	glDeleteVertexArrays(1, &m_vertexArrayObject);
};

void Mesh::draw(){
	glBindVertexArray(m_vertexArrayObject);

	glDrawArrays(GL_TRIANGLES, 0, m_drawCount);

	glBindVertexArray(0);
};
