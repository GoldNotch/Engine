#include <Graphics.h>

#define LOGGING_IMPLEMENTATION
#include <Logging.hpp>
IMPLEMENT_LOGGING_API(Graphics, GRAPHICS_API)
#include "GraphicsSystem.hpp"

/// @brief create and init rendering system
/// @param config
GRAPHICS_API GraphicsSystem CreateRenderingSystem(GraphicsSystemConfig config)
{
  io::Log(LogStatus::US_LOG_INFO, 0, "Create rendering system");
  return std::make_unique<Graphics::System>(config).release();
}

/// @brief destroy all objects and clear all resources in rendering system
GRAPHICS_API void DestroyRenderingSystem(GraphicsSystem system)
{
  io::Log(LogStatus::US_LOG_INFO, 0, "Destroy rendering system");
  delete reinterpret_cast<Graphics::System *>(system);
}

/// @brief Invalidates render resources like framebuffers (call it when change resolution)
GRAPHICS_API void Invalidate(GraphicsSystem system)
{
  reinterpret_cast<Graphics::System *>(system)->Invalidate();
}

/// @brief render scene
/// @param scene
GRAPHICS_API void RenderAll(GraphicsSystem system, const RenderSceneHandler * scenes, size_t count)
{
  reinterpret_cast<Graphics::System *>(system)
    ->RenderAll(reinterpret_cast<const Graphics::IScene *>(scenes), count);
}

/// @brief gets render scene container
GRAPHICS_API RenderSceneHandler AcquireRenderScene(GraphicsSystem system)
{
  return reinterpret_cast<Graphics::System *>(system)->AcquireRenderScene();
}

/// @brief pushes mesh data into scene
GRAPHICS_API void RenderScene_PushStaticMesh(RenderSceneHandler scene, StaticMesh mesh)
{
  reinterpret_cast<Graphics::Scene *>(scene)->PushStaticMesh(mesh);
}
