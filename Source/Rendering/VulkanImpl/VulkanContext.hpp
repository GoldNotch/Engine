#pragma once
#ifdef USE_WINDOW_OUTPUT
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
//#else //TODO UNIX
#endif
#endif // USE_WINDOW_OUTPUT

#include <filesystem>

#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>
#include <StaticString.hpp>

#include "../RenderingSystem.h"
#include "Types.hpp"


/// @brief context is object contains vulkan logical device. Also it provides access to vulkan functions
///			If rendering system uses several GPUs, you should create one context for each physical device
struct VulkanContext final
{
  /// @brief constructor
  explicit VulkanContext(const usRenderingOptions & opts);
  /// @brief destructor
  ~VulkanContext() noexcept;

  /// @brief returns dispatch_table, so user could write ctx->{any vulkan function}
  const vkb::DispatchTable * operator->() const { return &dispatch_table; }

  /// @brief Get renderer
  IRenderer * GetRenderer() const { return renderer.get(); }
  const vkb::Instance & GetInstance() const & { return vulkan_instance; }
  const vkb::Device & GetDevice() const & { return device; }
  const vkb::PhysicalDevice & GetGPU() const & { return choosen_gpu; }

  // TODO: Make it global functions
  /// @brief returns queue of specific type. doesn't own it
  std::pair<uint32_t, vk::Queue> GetQueue(vkb::QueueType type) const;
  /// @brief creates semaphore, doesn't own it
  vk::Semaphore CreateVkSemaphore() const;
  /// @brief creates fence, doesn't own it
  vk::Fence CreateFence(bool locked = false) const;
  /// @brief creates command pool for thread, doesn't own it
  vk::CommandPool CreateCommandPool(uint32_t queue_family_index) const;
  /// @brief creates command buffer in pool, doesn't own it
  vk::CommandBuffer CreateCommandBuffer(vk::CommandPool pool) const;


private:
  vkb::Instance vulkan_instance;
  vkb::PhysicalDevice choosen_gpu;
  vkb::Device device;
  vkb::DispatchTable dispatch_table;

  std::unique_ptr<IRenderer> renderer;

private:
  VulkanContext(const VulkanContext &) = delete;
  VulkanContext & operator=(const VulkanContext &) = delete;
};


namespace vk::utils
{
template<std::size_t Size>
constexpr decltype(auto) ResolveShaderPath(const char (&filename)[Size])
{
  return Core::static_string(DATA_PATH) + Core::static_string("/shaders/Vulkan/") +
         Core::static_string(filename);
}
} // namespace vk::utils
