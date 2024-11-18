#pragma once
#include <algorithm>
#include <list>
#include <memory>
#include <vector>

#include <Context.hpp>
#include <Scene.h>
#include <StaticString.hpp>

namespace Graphics
{

template<std::size_t Size>
constexpr decltype(auto) ResolveShaderPath(const wchar_t (&filename)[Size])
{
#define WC(x) L##x
  return Core::static_wstring(WC("Data")) + Core::static_wstring(L"/Shaders/") +
         Core::static_wstring(filename);
#undef WC
}

/*
* Scene is a container for drawable objects.
* Internaly it's just command buffer
* System can create some kinds of Scene, and user can fill it in multithreaded
* But drawcall is single-threaded
*/

struct IScene
{
  virtual ~IScene() = default;
  virtual RHI::ICommandBuffer & Draw() & noexcept = 0;
  virtual void SetViewport(uint32_t width, uint32_t height) = 0;
};

struct Scene : public IScene
{
  Scene(const RHI::IContext & ctx)
    : m_owner(ctx)
    , m_swapchain(ctx.GetSwapchain())
    , m_defaultFBO(ctx.GetSwapchain().GetDefaultFramebuffer())
  {
    m_buffer = ctx.GetSwapchain().CreateCommandBuffer();

    m_meshPipeline = ctx.CreatePipeline(ctx.GetSwapchain().GetDefaultFramebuffer(), 0);
    m_meshPipeline->AttachShader(RHI::ShaderType::Vertex,
                                 ResolveShaderPath(L"triangle_vert.spv").c_str());
    m_meshPipeline->AttachShader(RHI::ShaderType::Fragment,
                                 ResolveShaderPath(L"triangle_frag.spv").c_str());

    m_meshPipeline->AddInputBinding(0, sizeof(glVec2), RHI::InputBindingType::VertexData);
    m_meshPipeline->AddInputAttribute(0, 0, 0, 2, RHI::InputAttributeElementType::FLOAT);
    m_meshPipeline->AddInputBinding(1, sizeof(glVec3), RHI::InputBindingType::VertexData);
    m_meshPipeline->AddInputAttribute(1, 1, 0, 3, RHI::InputAttributeElementType::FLOAT);

    m_ubo = m_meshPipeline->DeclareUniform("t", 0, RHI::ShaderType::Fragment, sizeof(float));
    m_meshPipeline->Invalidate();
  }

  void PushStaticMesh(const StaticMesh & mesh)
  {
    m_meshes.push_back(mesh);
    m_vertices =
      m_owner.AllocBuffer(mesh.vertices_count * sizeof(glVec2), RHI::BufferGPUUsage::VertexBuffer);
    {
      auto && data = m_vertices->Map();
      std::memcpy(data.get(), mesh.vertices, mesh.vertices_count * sizeof(glVec2));
    }
    m_colors =
      m_owner.AllocBuffer(mesh.vertices_count * sizeof(glVec3), RHI::BufferGPUUsage::VertexBuffer);
    {
      auto && data = m_colors->Map();
      std::memcpy(data.get(), mesh.colors, mesh.vertices_count * sizeof(glVec3));
    }
    m_indices =
      m_owner.AllocBuffer(mesh.indices_count * sizeof(uint32_t), RHI::BufferGPUUsage::IndexBuffer);
    {
      auto && data = m_indices->Map();
      std::memcpy(data.get(), mesh.indices, mesh.indices_count * sizeof(uint32_t));
    }

    invalid = true;
  }
  void SetViewport(uint32_t width, uint32_t height) override
  {
    m_extent = {width, height};
    invalid = true;
  }

  RHI::ICommandBuffer & Draw() & noexcept override
  {
    // set uniforms
    static float t = 0.0f;
    float g = std::sin(t);
    t += 0.001;
    if (auto && map = m_ubo->Map())
    {
      std::memcpy(map.get(), &g, sizeof(float));
    }

    Invalidate();
    return *m_buffer;
  }

  void Invalidate()
  {
    if (invalid)
    {
      m_buffer->Reset();
      m_buffer->BeginWriting(m_defaultFBO, *m_meshPipeline);
      auto [w, h] = m_extent;
      m_buffer->SetViewport(static_cast<float>(w), static_cast<float>(h));
      m_buffer->SetScissor(0, 0, w, h);
      for (auto && mesh : m_meshes)
      {
        m_buffer->BindVertexBuffer(0, *m_vertices);
        m_buffer->BindVertexBuffer(1, *m_colors);
        m_buffer->BindIndexBuffer(*m_indices, RHI::IndexType::UINT32);
        m_buffer->DrawIndexedVertices(6, 1);
      }
      m_buffer->EndWriting();
    }
    invalid = false;
  }

private:
  const RHI::IContext & m_owner;
  const RHI::ISwapchain & m_swapchain;
  const RHI::IFramebuffer & m_defaultFBO;

  std::vector<StaticMesh> m_meshes;
  std::unique_ptr<RHI::ICommandBuffer> m_buffer;
  std::unique_ptr<RHI::IPipeline> m_meshPipeline;
  std::unique_ptr<RHI::IBufferGPU> m_vertices;
  std::unique_ptr<RHI::IBufferGPU> m_colors;
  std::unique_ptr<RHI::IBufferGPU> m_indices;
  RHI::IBufferGPU * m_ubo;
  bool invalid : 1 = true;
  std::pair<uint32_t, uint32_t> m_extent;
};

struct System final
{
  explicit System(const GraphicsSystemConfig & config)
    : m_context(RHI::CreateContext(RHI::SurfaceConfig{config.hWnd, config.hInstance}))
  {
  }

  ~System() { m_context->WaitForIdle(); }

  void Invalidate()
  {
    m_context->GetSwapchain().Invalidate();
    auto [w, h] = m_context->GetSwapchain().GetExtent();
    for (auto && scene : m_scenes)
    {
      scene->SetViewport(w, h);
    }
  }

  void DrawMeshes(StaticMesh mesh)
  {
    // this happends before DrawMesh call
    //BufferGPU vbo;
    //BufferGPU ibo;
    //vbo.upload(mesh.vertices, mesh.vertices_count);
    //ibo.upload(mesh.indices, mesh.indices_count);

    //auto && fbo = m_context->CreateFramebuffer();


    //ThreadLocalCommandBuffer buf; //contains second command buffer
    //buf.usePipeline(pipeline);    // like glUseShader
    //buf.bindVBO(vbo);
    //buf.bindIBO(ibo);
    //buf.drawIndexed(mesh.indices_count);
  }

  /// @brief glFlush actually
  /// @param scenes
  /// @param count
  void RenderAll(const IScene * scenes, size_t count)
  {
    auto && swapchain = m_context->GetSwapchain();

    for (auto && scene : m_scenes)
      scene->Draw();
    RHI::ICommandBuffer * buf = swapchain.BeginFrame();
    if (buf)
    {
      for (auto && scene : m_scenes)
        buf->AddCommands(scene->Draw());
      swapchain.EndFrame();
    }
  }

  IScene * AcquireRenderScene() noexcept
  {
    //TODO: don't create new scene. Alloc them once in constructor (one per frame) and use here
    auto && handler = m_scenes.emplace_back(new Scene(*m_context));
    auto [w, h] = m_context->GetSwapchain().GetExtent();
    handler->SetViewport(w, h);
    return handler.get();
  }

private:
  std::unique_ptr<RHI::IContext> m_context;
  std::vector<std::unique_ptr<IScene>> m_scenes;
};

} // namespace Graphics
