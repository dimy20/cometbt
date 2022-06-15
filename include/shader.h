#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <GL/glew.h>

#define NUM_SHADERS 2

class Shader{
	public:
		Shader(const std::string& filename);
		void bind();
		~Shader();
	private:
		unsigned int m_program;
		unsigned int m_vs; /*vertex shader*/
		unsigned int m_fs; /*fragment shader*/
};
