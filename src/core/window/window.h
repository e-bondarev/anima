#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

class Win
{
public:
	Win();
	~Win();

	GLFWwindow* get_handle();
	bool is_running() const;
	
	void poll_events() const;
	void swap_buffers() const;

private:
	GLFWwindow* handle;
};

namespace global
{
	Win** get_window_pp();
	Win* get_window_p();
	Win& get_window();
	
	namespace gui
	{
		void init();
		void shutdown();

		void begin_frame();
		void end_frame();
	}
}