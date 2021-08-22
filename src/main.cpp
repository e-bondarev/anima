#include "core/window/window.h"

#include "xyapi/common.h"
#include "xyapi/gl/texture.h"
#include "xyapi/gl/shader.h"
#include "xyapi/gl/vao.h"

#include "common.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"

#include "assets/model.h"
#include "assets/image.h"

static glm::mat4 model_matrix;

#ifndef COMPILE_SHADERS
int main(int argc, char* argv[])
{
	arguments(argc, argv);

	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

	Shader shader(default_vert, default_frag, { "u_model", "u_proj" });

	Avatar avatar;
	VAO vao;

	// Load model
	{
		Model model("D:/Dev/anima/assets/models/1.fbx");

		avatar.init(model.bone_map, model.skeleton);

		vao.bind();
			vao.add_vbo(VBO::Type::Array, VBO::Usage::Static, model.vertices.size(), sizeof(Vertex), &model.vertices[0], Vertex::GetLayout());
			vao.add_vbo(VBO::Type::Indices, VBO::Usage::Static, model.indices.size(), sizeof(uint32_t), &model.indices[0]);
	}

	Image image("D:/Dev/anima/assets/textures/1.png");

	Animation animation("D:/Dev/anima/assets/models/1.fbx");

	Texture texture(image.width, image.height, image.data, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, { Texture::set_interpolation(Interpolation::Constant) });

	model_matrix = glm::mat4(1);
	model_matrix = glm::translate(model_matrix, glm::vec3(0, -3, -10));
	model_matrix = glm::scale(model_matrix, glm::vec3(0.01f));

	while (window.is_running())
	{
		window.poll_events();
		
		global::gui::begin_frame();

			ImGui::ShowDemoWindow();

			ImGui::Render();

			int display_w, display_h;
			glfwGetFramebufferSize(window.get_handle(), &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.bind();
			
				static glm::mat4 projection_matrix = glm::mat4(1);
				projection_matrix = glm::perspective(glm::radians(70.0f), static_cast<float>(display_w) / static_cast<float>(display_h), 0.1f, 1000.0f);

				static float time = 0;
				time += 0.1f;
				avatar.calculate_pose(time, animation);

				shader.set_uniform_mat4("u_model", &model_matrix[0][0]);
				shader.set_uniform_mat4("u_proj", &projection_matrix[0][0]);
				shader.set_uniform_mat4("u_bones", &avatar.current_transforms[0][0][0], avatar.get_amount_of_bones());

				vao.bind();
				texture.bind();
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