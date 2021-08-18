#include "core/window/window.h"

#include "xyapi/common.h"
#include "xyapi/gl/shader.h"
#include "xyapi/gl/vao.h"

#include "common.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"

struct Vertex
{
    glm::vec2 position;

    inline static std::vector<VertexBufferLayout> GetLayout() 
	{
        return 
		{
            { 2, sizeof(Vertex), offsetof(Vertex, position) },
        };
    }
};

#ifndef COMPILE_SHADERS
int main(int argc, char* argv[])
{
	arguments(argc, argv);

	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

	Shader shader(default_vert, default_frag, { "u_Float" });
	
	std::vector<Vertex> vertices = 
	{
		{ { -0.5f, -0.5f } },
		{ {  0.5f, -0.5f } },
		{ {  0.5f,  0.5f } },
		{ { -0.5f,  0.5f } }
	};

	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	VAO vao;
	vao.bind();
		vao.add_vbo(VBO::Type::Array, VBO::Usage::Static, vertices.size(), sizeof(Vertex), &vertices[0], Vertex::GetLayout());
		vao.add_vbo(VBO::Type::Indices, VBO::Usage::Static, indices.size(), sizeof(uint32_t), &indices[0]);

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
				vao.get_index_buffer()->bind();
					glDrawElements(GL_TRIANGLES, vao.get_index_buffer()->get_index_count(), GL_UNSIGNED_INT, nullptr);
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