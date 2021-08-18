#include "core/window/window.h"

#include "xyapi/common.h"
#include "xyapi/gl/shader.h"
#include "graphics/opengl/vao.h"

#include "common.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"

#ifndef COMPILE_SHADERS
int main(int argc, char* argv[])
{
	arguments(argc, argv);

	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

	Shader shader(default_vert, default_frag, { "u_Float" });

	struct vec2
	{
		float x = 0.1f;
		float y = 0.2f;
	} pos;

	shader.bind();
		shader.set_uniform_vec2("u_Float", &pos.x);
	shader.unbind();

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
#endif