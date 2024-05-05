#include "MeshPipeline.hpp"

#include <unordered_map>

#include "../Utils/PipelineBuilder.hpp"
#include "../VulkanContext.hpp"

namespace shaders
{
static constexpr auto vertex_shader = vk::utils::ResolveShaderPath("triangle_vert.spv");
static constexpr auto fragment_shader = vk::utils::ResolveShaderPath("triangle_frag.spv");
} // namespace shaders


/// @brief hash function for StaticMeshData
template<>
struct std::hash<StaticMesh>
{
  size_t operator()(StaticMesh const & mesh) const noexcept
  {
    size_t hash = 0;
    Core::utils::hash_combine(hash, mesh.vertices_count);
    Core::utils::hash_combine(hash, mesh.vertices);
    return hash;
  }
};

/// @brief equality operator for StaticMeshData
bool operator==(const StaticMesh & m1, const StaticMesh & m2)
{
  return std::memcmp(&m1, &m2, sizeof(StaticMesh)) == 0;
}


/// @brief pipeline for StaticMeshData
struct MeshPipeline final : public IPipeline
{
  /// @brief constructor
  /// @param ctx - vulkan context
  /// @param renderPass - vk::RenderPass
  /// @param subpass_index - index of subpass in render pass
  MeshPipeline(const VulkanContext & ctx, const IRenderer & renderer,
               const vk::RenderPass & renderPass, uint32_t subpass_index);

  /// @brief some logic in the beginning of processing (f.e. some preparations to rendering)
  /// @param buffer - command buffer
  /// @param viewport - settings for pipeline
  void BeginProcessing(const vk::CommandBuffer & buffer,
                       const vk::Rect2D & viewport) const override;

  /// @brief Some logic in the end of processing (f.e. flush all commands in buffer)
  /// @param buffer - command buffer
  void EndProcessing(const vk::CommandBuffer & buffer) const override;

  /// @brief pipeline-specific function to process one drawable object
  /// @param buffer - command buffer
  /// @param mesh - drawable data
  void ProcessObject(const vk::CommandBuffer & buffer, size_t frame_index,
                     const StaticMesh & mesh) const;

private:
  const VulkanContext & context; ///< vulkan context
  const IRenderer & renderer;
  std::unique_ptr<vk::utils::Pipeline> pipeline = nullptr;
  mutable float timer = 0.0;


  using BuffersPair = std::pair<BufferGPU, BufferGPU>;
  mutable std::unordered_map<StaticMesh, BuffersPair> cache; ///< cache of bufferized geometry

private:
  MeshPipeline(const MeshPipeline &) = delete;
  MeshPipeline & operator=(const MeshPipeline &) = delete;
};


// ------------------------ Implementation -----------------------

/// @brief constructor, initialize all vulkan objects
/// @param ctx
MeshPipeline::MeshPipeline(const VulkanContext & ctx, const IRenderer & renderer,
                           const vk::RenderPass & renderPass, uint32_t subpass_index)
  : context(ctx)
  , renderer(renderer)
{
  vk::utils::PipelineBuilder builder{ctx};
  pipeline = builder.SetShaderAPI<StaticMesh>()
               .AttachShader(vk::ShaderStageFlagBits::eVertex, shaders::vertex_shader.c_str())
               .AttachShader(vk::ShaderStageFlagBits::eFragment, shaders::fragment_shader.c_str())
               .Build(renderer, renderPass, subpass_index);

  pipeline->GetUniformBinding(0).Alloc(4, true);
}

void MeshPipeline::BeginProcessing(const vk::CommandBuffer & buffer, const vk::Rect2D & vp) const
{
  vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
  VkViewport viewport{};
  viewport.x = static_cast<float>(vp.offset.x);
  viewport.y = static_cast<float>(vp.offset.y);
  viewport.width = static_cast<float>(vp.extent.width);
  viewport.height = static_cast<float>(vp.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(buffer, 0, 1, &viewport);

  VkRect2D scissor = vp;
  vkCmdSetScissor(buffer, 0, 1, &scissor);

  //vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0,
  //                        1, &descr_layout, 0, nullptr);
}

void MeshPipeline::ProcessObject(const vk::CommandBuffer & buffer, size_t frame_index,
                                 const StaticMesh & mesh) const
{
  auto cache_it = cache.find(mesh);
  if (cache_it == cache.end())
  {
    size_t size = mesh.vertices_count * (sizeof(glVec2) + sizeof(glVec3));
    auto vert_buffer =
      context.GetMemoryManager().AllocBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    {
      auto ptr = vert_buffer.Map();
      size_t offset = 0;
      std::memcpy(reinterpret_cast<char *>(ptr.get()) + offset, mesh.vertices,
                  mesh.vertices_count * sizeof(glVec2));
      offset += mesh.vertices_count * sizeof(glVec2);
      std::memcpy(reinterpret_cast<char *>(ptr.get()) + offset, mesh.colors,
                  mesh.vertices_count * sizeof(glVec3));
    }
    vert_buffer.Flush();

    BufferGPU ind_buffer;
    if (mesh.indices_count > 0)
    {
      ind_buffer = context.GetMemoryManager().AllocBuffer(mesh.indices_count * sizeof(uint32_t),
                                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
      {
        auto ptr = ind_buffer.Map();
        std::memcpy(ptr.get(), mesh.indices, mesh.indices_count * sizeof(uint32_t));
      }
      ind_buffer.Flush();
    }
    cache_it =
      cache.insert(cache.end(), std::make_pair(mesh, std::make_pair(std::move(vert_buffer),
                                                                    std::move(ind_buffer))));
  }

  float t = std::sin(timer);
  timer += 0.001;
  if (frame_index == 0)
  {
    pipeline->GetUniformBinding(0).Upload(&t, sizeof(float));
  }

  const bool hasIndices = static_cast<vk::Buffer>(cache_it->second.second) != VK_NULL_HANDLE;
  VkBuffer vertexBuffers[] = {static_cast<vk::Buffer>(cache_it->second.first),
                              static_cast<vk::Buffer>(cache_it->second.first)};
  VkDeviceSize offsets[] = {0, mesh.vertices_count * sizeof(glVec2)};
  vkCmdBindVertexBuffers(buffer, 0, 2, vertexBuffers, offsets);
  pipeline->GetUniformBinding(0).Bind(buffer, pipeline->GetPipelineLayout(), frame_index);

  if (hasIndices)
  {
    vkCmdBindIndexBuffer(buffer, static_cast<vk::Buffer>(cache_it->second.second), 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(buffer, static_cast<uint32_t>(mesh.indices_count), 1, 0, 0, 0);
  }
  else
    vkCmdDraw(buffer, static_cast<uint32_t>(mesh.vertices_count), 1, 0, 0);
}

void MeshPipeline::EndProcessing(const vk::CommandBuffer & buffer) const
{
}

// ----------------- Pipeline template specializations -------------------------

template<>
void ProcessWithPipeline<StaticMesh>(const IPipeline & pipeline, size_t frame_index,
                                     const vk::CommandBuffer & buffer, const StaticMesh & obj)
{
  assert(typeid(pipeline) == typeid(MeshPipeline));
  static_cast<const MeshPipeline &>(pipeline).ProcessObject(buffer, frame_index, obj);
}

template<>
vk::SubpassDescription SubpassDescriptionBuilder<StaticMesh>::Get() noexcept
{
  VkSubpassDescription result{};
  static std::vector<VkAttachmentReference> attachmentsRef;
  if (attachmentsRef.empty())
  {
    auto && colorAttachment = attachmentsRef.emplace_back(VkAttachmentReference{});
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  result.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  result.colorAttachmentCount = static_cast<uint32_t>(attachmentsRef.size());
  result.pColorAttachments = attachmentsRef.data();
  return result;
}

template<>
std::vector<VkVertexInputBindingDescription> ShaderAPIBuilder<StaticMesh>::BuildBindings() noexcept
{
  std::vector<VkVertexInputBindingDescription> bindings;
  bindings.reserve(1);
  {
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(glVec2);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings.emplace_back(binding);
  }
  {
    VkVertexInputBindingDescription binding{};
    binding.binding = 1;
    binding.stride = sizeof(glVec3);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings.emplace_back(binding);
  }
  return bindings;
}

template<>
std::vector<VkVertexInputAttributeDescription> ShaderAPIBuilder<
  StaticMesh>::BuildAttributes() noexcept
{
  std::vector<VkVertexInputAttributeDescription> attributes;
  attributes.reserve(2);
  { // position attribute
    VkVertexInputAttributeDescription attribute{};
    attribute.binding = 0;
    attribute.location = 0;
    attribute.format = VK_FORMAT_R32G32_SFLOAT;
    attribute.offset = 0;
    attributes.emplace_back(attribute);
  }
  { // color attribute
    VkVertexInputAttributeDescription attribute;
    attribute.binding = 1;
    attribute.location = 1;
    attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute.offset = 0;
    attributes.emplace_back(attribute);
  }
  return attributes;
}

template<>
std::vector<VkDescriptorSetLayoutBinding> ShaderAPIBuilder<
  StaticMesh>::BuildDescriptorsLayout() noexcept
{
  std::vector<VkDescriptorSetLayoutBinding> descriptor_sets;
  auto && uboLayoutBinding = descriptor_sets.emplace_back(VkDescriptorSetLayoutBinding{});
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr; // Optional, for images

  return descriptor_sets;
}


template<>
std::vector<VkDescriptorPoolSize> ShaderAPIBuilder<StaticMesh>::BuildPoolAllocationInfo() noexcept
{
  std::vector<VkDescriptorPoolSize> sizes;
  auto && uniforms_size = sizes.emplace_back();
  uniforms_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniforms_size.descriptorCount = 1;
  return sizes;
}

// -----------------------------------------------------------------------

std::unique_ptr<IPipeline> CreateMeshPipeline(const VulkanContext & ctx, const IRenderer & renderer,
                                              const vk::RenderPass & renderPass,
                                              uint32_t subpass_index)
{
  return std::make_unique<MeshPipeline>(ctx, renderer, renderPass, subpass_index);
}
