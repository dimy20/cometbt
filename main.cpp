#include <vector>
#include <iostream>
#include <GL/glew.h>
#include "display.h"
#include "shader.h"
#include "mesh.h"

int main(){
	Display display(800, 600, "hell world");
	Shader shader("../basicShader");
	Vertex vertices [] ={
		Vertex(glm::vec3(-0.5,-0.5,0)),
		Vertex(glm::vec3(0,0.5,0)),
		Vertex(glm::vec3(0.5,-0.5,0)),
	};

	Mesh mesh(vertices, 3);

	while(!display.isClosed()){
		glClearColor(0.0f, 0.15f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader.bind();
		mesh.draw();

		display.Update();
	}

	return 0;
}
