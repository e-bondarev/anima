#include "window.h"

Win::Win()
{
	glfwInit();
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_SAMPLES, 4);

	handle = glfwCreateWindow(640, 480, "Anima", nullptr, nullptr);
	glfwMakeContextCurrent(handle);
	//glfwMaximizeWindow(handle);
    glfwSwapInterval(1);

	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

Win::~Win()
{
	glfwDestroyWindow(handle);
	glfwTerminate();
}

bool Win::is_running() const
{
	return !glfwWindowShouldClose(handle);
}
	
void Win::poll_events() const
{
	glfwPollEvents();
}

void Win::swap_buffers() const
{
	glfwSwapBuffers(handle);
}

GLFWwindow* Win::get_handle()
{
	return handle;
}

namespace global
{
	Win* win;

	Win** get_window_pp()
	{
		return &win;
	}

	Win* get_window_p()
	{
		return win;
	}

	Win& get_window()
	{
		return *win;
	}
	
	namespace gui
	{
		void init()
		{
			GLFWwindow* handle = get_window().get_handle();
			
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplGlfw_InitForOpenGL(handle, true);
			ImGui_ImplOpenGL3_Init();
		}

		void shutdown()
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void begin_frame()
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void end_frame()
		{    	
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
	}
}