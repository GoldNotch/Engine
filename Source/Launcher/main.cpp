#include <cassert>
#include <cstdio>
#include <cstdlib>
//#define GLEW_STATIC
//#include <GL/glew.h>

#include <App.h>
#include <GLFW/glfw3.h>
#include <Logging.hpp>
#include <RenderingSystem.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Process.hpp"

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

  Rendering_SetLoggingFunc(ConsoleLog);
  App_SetLoggingFunc(ConsoleLog);

  RenderingSystemConfig renderOpts;
  renderOpts.gpu_autodetect = true;
  renderOpts.hWindow = glfwGetWin32Window(window);
  renderOpts.hInstance = GetModuleHandle(nullptr);
  renderOpts.required_gpus = 1;
  auto rendering_system = CreateRenderingSystem(renderOpts);
  if (!rendering_system)
  {
    std::printf("Failed to init rendering system\n");
    glfwTerminate();
    return -1;
  }

  App::MainProcess app_process;
  app_process.Start();

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    RenderSceneHandler scene = AcquireRenderScene(rendering_system);
    app_process.ExecuteWithPause(usApp_InitRenderableScene, scene);

    RenderFrame(rendering_system);
  }

  DestroyRenderingSystem(rendering_system);
  glfwTerminate();
  return 0;
}
