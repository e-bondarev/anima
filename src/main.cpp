#include "core/window/window.h"

#include "graphics/opengl/shader.h"
#include "graphics/opengl/vao.h"

const char* vs_code = R""""(

#version 440 core

layout (location = 0) in vec2 in_Position;

void main()
{
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

)"""";

const char* fs_code = R""""(

#version 440 core

out vec4 out_Color;

void main()
{
	out_Color = vec4(0.0, 0.0, 1.0, 1.0);
}

)"""";

int main(int argc, char* argv[])
{
	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

	Shader shader(vs_code, fs_code);

	std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f } },
		{ {  0.5f, -0.5f } },
		{ {  0.5f,  0.5f } },
		{ { -0.5f,  0.5f } }
	};

	VAO vao(vertices, Vertex::GetLayout(), { 0, 1, 2, 2, 3, 0 });

	while (window.is_running())
	{
		window.poll_events();
		
		global::gui::begin_frame();

			ImGui::ShowDemoWindow();		

			ImGui::Render();

			int display_w, display_h;
			glfwGetFramebufferSize(window.get_handle(), &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClear(GL_COLOR_BUFFER_BIT);

			shader.bind();

				vao.bind();
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
				vao.unbind();

			shader.unbind();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		global::gui::end_frame();

		window.swap_buffers();
	}

	global::gui::shutdown();

	delete *global::get_window_pp();

	return 0;
}