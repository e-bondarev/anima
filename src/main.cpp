#include "core/window/window.h"

#include "graphics/opengl/shader.h"
#include "graphics/opengl/vao.h"

#include "common.h"

#include "files/files.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"

void program()
{
	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

	Shader shader(default_vert, default_frag);

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
}

const std::vector<std::string> SHADER_FORMATS = { "vert", "frag" };

bool is_shader(const std::string& format)
{
	for (int i = 0; i < SHADER_FORMATS.size(); i++)
		if (SHADER_FORMATS[i] == format)
			return true;

	return false;
}

void file_callback(const std::string& entry)
{
	const std::string& type = files::get_type(entry);

	if (is_shader(type))
	{
		const std::string content = files::read(entry);

		const std::string header_path = entry + ".h";
		files::write(header_path, content);
	}
}

void directory_callback(const std::string& entry)
{
	files::recursive_loop(entry, directory_callback, file_callback);
}

void compile_shaders()
{
	const std::string path = get_arguments()[0];
	const std::string src = path.substr(0, path.find("build")) + "src";
	
	files::recursive_loop(src, directory_callback, file_callback);
}

int main(int argc, char* argv[])
{
	arguments(argc, argv);

#ifndef COMPILE_SHADERS
	program();
#else
	compile_shaders();
#endif

	return 0;
}