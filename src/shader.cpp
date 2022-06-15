#include "shader.h"

static std::string loadShader(const std::string& filename);
static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);
static GLuint createShader(const std::string& text, GLenum shaderType);




Shader::Shader(const std::string& filename){
	m_program = glCreateProgram();

	m_vs = createShader(loadShader(filename + ".vs"), GL_VERTEX_SHADER); 
	m_fs = createShader(loadShader(filename + ".fs"), GL_FRAGMENT_SHADER);

	glAttachShader(m_program, m_vs);
	glAttachShader(m_program, m_fs);

	glLinkProgram(m_program);
	CheckShaderError(m_program, GL_LINK_STATUS, true, "Error: Program linking failed");

	glValidateProgram(m_program);
	CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Error: Program validating failed");
};

Shader::~Shader(){
	glDetachShader(m_program, m_vs);
	glDetachShader(m_program, m_fs);
	glDeleteShader(m_fs);
	glDeleteShader(m_vs);
	glDeleteProgram(m_program);
}

void Shader::bind(){
	glUseProgram(m_program);
}

static GLuint createShader(const std::string& shader_src, GLenum shaderType){
	unsigned int shader = glCreateShader(shaderType);

	if(shader == 0) std::cerr << "Error: Shader creation failed" << std::endl;

	const char * src = shader_src.c_str();

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error: Shader compilation failed");
	return shader;
}

static std::string loadShader(const std::string& filename){
	std::ifstream file(filename);

	std::string line, output;

	output = "";
	if(!file.is_open()) std::cerr << "Error: Failed opening file " <<  filename << std::endl;

	while(file.good()){
		getline(file, line);
		output.append(line + "\n");
	}
	return output;
}
static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
{
    GLint success = 0;
    GLchar error[1024] = { 0 };

    if(isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

    if(success == GL_FALSE)
    {
        if(isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}
