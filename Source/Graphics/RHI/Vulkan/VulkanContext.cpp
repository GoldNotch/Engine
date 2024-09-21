#include "VulkanContext.hpp"

#include <Context.hpp>
#include <Logging.hpp>
#include <VkBootstrap.h>

#include "CommandBuffer.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"

// --------------------- Static functions ------------------------------

namespace
{

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData)
{
  using SeverityBitFlag = vk::DebugUtilsMessageSeverityFlagBitsEXT;
  using SeverityFlags = VkDebugUtilsMessageSeverityFlagBitsEXT;
  using TypeBitFlag = vk::DebugUtilsMessageTypeFlagBitsEXT;
  using TypeFlags = VkDebugUtilsMessageTypeFlagsEXT;
  if (messageSeverity == static_cast<SeverityFlags>(SeverityBitFlag::eError) ||
      messageType == static_cast<TypeFlags>(TypeBitFlag::eValidation))
    io::Log(US_LOG_ERROR, 0, pCallbackData->pMessage);
  else if (messageSeverity == static_cast<SeverityFlags>(SeverityBitFlag::eWarning) ||
           messageType == static_cast<TypeFlags>(TypeBitFlag::ePerformance))
    io::Log(US_LOG_WARNING, 0, pCallbackData->pMessage);
  else
    io::Log(US_LOG_INFO, 0, pCallbackData->pMessage);

  return VK_FALSE;
}


vkb::Instance CreateInstance(const char * appName)
{
  vkb::Instance result;
  vkb::InstanceBuilder builder;
  auto inst_ret = builder
                    .set_app_name(appName)
                    .request_validation_layers()
                    .set_debug_callback(VulkanDebugCallback)
                    .build();
  if (!inst_ret || !inst_ret.has_value())
    io::Log(US_LOG_ERROR, 0, "Failed to create Vulkan instance - %s", inst_ret.error().message());
  else
    result = inst_ret.value();
  return result;
}

vk::SurfaceKHR CreateSurface(vkb::Instance inst, const RHI::SurfaceConfig & config)
{
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkResult result;
#ifdef _WIN32
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = reinterpret_cast<HWND>(config.hWnd);
  createInfo.hinstance = reinterpret_cast<HINSTANCE>(config.hInstance);
  result = vkCreateWin32SurfaceKHR(inst, &createInfo, nullptr, &surface);
#else
  VkXlibSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  createInfo.window = reinterpret_cast<Window>(config.hWnd);
  createInfo.dpy = reinterpret_cast<Display *>(config.hInstance);
  result = vkCreateXlibSurfaceKHR(inst, &createInfo, nullptr, &surface);
#endif
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to create window surface!");
  return vk::SurfaceKHR(surface);
}

vkb::PhysicalDevice SelectPhysicalDevice(vkb::Instance inst,
                                         VkSurfaceKHR surface /* = VK_NULL_HANDLE*/)
{
  vkb::PhysicalDeviceSelector selector{inst};
  auto phys_ret = selector.set_surface(surface)
                    .require_present(surface != VK_NULL_HANDLE)
                    .set_minimum_version(1, 3)
                    .select();

  if (!phys_ret)
  {
    io::Log(US_LOG_ERROR, 0, "Failed to select Vulkan Physical Device - %s",
            phys_ret.error().message());
    return vkb::PhysicalDevice{};
  }
  return phys_ret.value();
}

} // namespace


namespace RHI::vulkan
{
struct Context::Impl final
{
  Impl(const char * appName, const SurfaceConfig & config, vk::SurfaceKHR & surface)
  {
    m_instance = CreateInstance("AppName");
    surface = CreateSurface(m_instance, config);
    m_gpu = SelectPhysicalDevice(m_instance, surface);
    vkb::DeviceBuilder device_builder{m_gpu};
    auto dev_ret = device_builder.build();
    if (!dev_ret)
      io::Log(US_LOG_ERROR, 0, "Failed to create Vulkan device - %s", dev_ret.error().message());
    m_device = dev_ret.value();
    m_dispatchTable = m_device.make_table();
  }

  ~Impl()
  {
    vkb::destroy_device(m_device);
    vkb::destroy_instance(m_instance);
  }

  VkDevice GetDevice() const { return m_device; }
  VkInstance GetInstance() const { return m_instance; }
  VkPhysicalDevice GetGPU() const { return m_gpu; }

  std::pair<uint32_t, VkQueue> GetQueue(vkb::QueueType type) const
  {
    auto queue_ret = m_device.get_queue(type);
    auto familly_index = m_device.get_queue_index(type);
    if (!queue_ret)
      throw std::runtime_error("failed to request queue");
    return std::make_pair(familly_index.value(), queue_ret.value());
  }

private:
  vkb::Instance m_instance;
  vkb::PhysicalDevice m_gpu;
  vkb::Device m_device;
  vkb::DispatchTable m_dispatchTable;
};


Context::Context(const SurfaceConfig & config)
{
  vk::SurfaceKHR surface;
  m_impl = std::make_unique<Impl>("appName", config, surface);
  //memory_manager = CreateMemoryManager(*this);
  m_swapchain = std::make_unique<Swapchain>(*this, surface);
}

Context::~Context()
{
  vkDeviceWaitIdle(m_impl->GetDevice());
}

std::unique_ptr<IFramebuffer> Context::CreateFramebuffer() const
{
  return nullptr; //std::make_unique<Framebuffer>(*this, GetSwapchain().GetBuffersCount());
}

std::unique_ptr<IPipeline> Context::CreatePipeline(const IFramebuffer & framebuffer,
                                                   uint32_t subpassIndex) const
{
  return std::make_unique<Pipeline>(*this, framebuffer, subpassIndex);
}

void Context::WaitForIdle() const
{
  vkDeviceWaitIdle(GetDevice());
}

const vk::Instance Context::GetInstance() const
{
  return m_impl->GetInstance();
}

const vk::Device Context::GetDevice() const
{
  return m_impl->GetDevice();
}

const vk::PhysicalDevice Context::GetGPU() const
{
  return m_impl->GetGPU();
}

std::pair<uint32_t, VkQueue> Context::GetQueue(QueueType type) const
{
  return m_impl->GetQueue(static_cast<vkb::QueueType>(type));
}

} // namespace RHI::vulkan

namespace RHI
{
std::unique_ptr<IContext> CreateContext(const SurfaceConfig & config)
{
  return std::make_unique<vulkan::Context>(config);
}
} // namespace RHI


namespace RHI::vulkan::utils
{

vk::Semaphore CreateVkSemaphore(vk::Device device)
{
  VkSemaphoreCreateInfo info{};
  VkSemaphore result = VK_NULL_HANDLE;
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  // Don't use createSemaphore in dispatchTable because it's broken
  if (vkCreateSemaphore(device, &info, nullptr, &result) != VK_SUCCESS)
    throw std::runtime_error("failed to create semaphore");
  return vk::Semaphore(result);
}

vk::Fence CreateFence(vk::Device device, bool locked)
{
  VkFenceCreateInfo info{};
  VkFence result = VK_NULL_HANDLE;
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (locked)
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  // Don't use createFence in dispatchTable because it's broken
  if (vkCreateFence(device, &info, nullptr, &result) != VK_SUCCESS)
    throw std::runtime_error("failed to create fence");
  return vk::Fence(result);
}

vk::CommandPool CreateCommandPool(vk::Device device, uint32_t queue_family_index)
{
  VkCommandPool commandPool = VK_NULL_HANDLE;

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queue_family_index;
  if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create command pool!");
  }
  return vk::CommandPool{commandPool};
}
} // namespace RHI::vulkan::utils
