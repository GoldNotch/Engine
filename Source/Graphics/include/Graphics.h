#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef _WIN32
#if defined(GRAPHICS_STATIC_BUILD)
#define GRAPHICS_API
#elif defined(GRAPHICS_BUILD)
#define GRAPHICS_API __declspec(dllexport)
#else
#define GRAPHICS_API __declspec(dllimport)
#endif

#else
#define GRAPHICS_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <LoggingAPI.h>
  DECLARE_LOGGING_API(Graphics, GRAPHICS_API)
#include "Scene.h"

  /// @brief max count of used gpus in app
  const size_t MaxRequiredGPUs = 1;
  typedef void * Handler;

  /// @brief options for rendering system initializing
  typedef struct
  {
    bool gpu_autodetect;
    size_t required_gpus;
    const char * gpu_name[MaxRequiredGPUs];

    Handler hWnd;
    Handler hInstance;

  } GraphicsSystemConfig;

  /// @brief rendering system handler
  typedef void * GraphicsSystem;

  /// @brief create and init rendering system
  /// @param config
  GRAPHICS_API GraphicsSystem CreateRenderingSystem(GraphicsSystemConfig config);

  /// @brief destroy all objects and clear all resources in rendering system
  GRAPHICS_API void DestroyRenderingSystem(GraphicsSystem system);

  /// @brief Invalidates render resources like framebuffers (call it when change resolution)
  GRAPHICS_API void Invalidate(GraphicsSystem system);

  /// @brief render scene
  /// @param scene
  GRAPHICS_API void RenderAll(GraphicsSystem system, const RenderSceneHandler * scenes,
                              size_t count);

  /// @brief gets render scene container
  GRAPHICS_API RenderSceneHandler AcquireRenderScene(GraphicsSystem system);

  /// @brief pushes mesh data into scene
  GRAPHICS_API void RenderScene_PushStaticMesh(RenderSceneHandler scene, StaticMesh mesh);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_H
