#include "core/window/window.h"

int main(int argc, char* argv[])
{
	Win** window_pp = global::get_window_pp();
	*window_pp = new Win();
	Win& window = **window_pp;

	global::gui::init();

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

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		global::gui::end_frame();

		window.swap_buffers();
	}

	global::gui::shutdown();

	delete *global::get_window_pp();

	return 0;
}