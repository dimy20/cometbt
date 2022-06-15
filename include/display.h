#pragma once

#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

class Display{
	public:
		Display(int width, int height, const std::string& title);
		~Display();

		void Update();
		bool isClosed();
	public:
		int m_width;
		int m_height;

	private:
		SDL_Window * m_window;
		SDL_GLContext m_glContext;
	private:
		bool m_isClosed;
};
