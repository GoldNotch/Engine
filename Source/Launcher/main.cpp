#include <cassert>
#include <cstdio>
#include <cstdlib>
//#define GLEW_STATIC
//#include <GL/glew.h>

#include <App.h>
#include <GLFW/glfw3.h>
#include <Graphics.h>
#include <Logging.hpp>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3native.h>

#include "Process.hpp"

GraphicsSystem rendering_system;

void ConsoleLog(LogStatus status, int code, const char * message)
{
  switch (status)
  {
    case US_LOG_INFO:
      std::printf("INFO: %s\n", message);
      break;
    case US_LOG_WARNING:
      std::printf("WARNING: %s\n", message);
      break;
    case US_LOG_ERROR:
      std::printf("ERROR(%i): %s\n", code, message);
      break;
    case US_LOG_FATAL_ERROR:
      std::printf("FATAL_ERROR(%i): %s\nProgram is gonna abort!!!", code, message);
      system("cls");
      std::abort();
      break;
  }
}

void OnResizeWindow(GLFWwindow* window, int width, int height)
{
  Invalidate(rendering_system);
}

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow * window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    std::printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwSetWindowSizeCallback(window, OnResizeWindow);

  Graphics_SetLoggingFunc(ConsoleLog);
  App_SetLoggingFunc(ConsoleLog);

  GraphicsSystemConfig renderOpts;
  renderOpts.gpu_autodetect = true;
#ifdef _WIN32
  renderOpts.hWnd = glfwGetWin32Window(window);
  renderOpts.hInstance = GetModuleHandle(nullptr);
#elif defined(__linux__)
  renderOpts.hWnd = reinterpret_cast<void *>(glfwGetX11Window(window));
  renderOpts.hInstance = glfwGetX11Display();
#endif
  renderOpts.required_gpus = 1;
  rendering_system = CreateRenderingSystem(renderOpts);
  if (!rendering_system)
  {
    std::printf("Failed to init rendering system\n");
    glfwTerminate();
    return -1;
  }

  App::MainProcess app_process;
  app_process.Start();

  RenderSceneHandler scene = AcquireRenderScene(rendering_system);
  app_process.ExecuteWithPause(usApp_InitRenderableScene, scene);
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);


    RenderAll(rendering_system, &scene, 1);
  }

  DestroyRenderingSystem(rendering_system);
  glfwTerminate();
  return 0;
}
