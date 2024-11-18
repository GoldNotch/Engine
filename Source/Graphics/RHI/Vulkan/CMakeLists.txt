target_sources (${PROJECT_NAME}
PRIVATE
	"VulkanContext.cpp"
	"Swapchain.cpp"
	"Framebuffer.cpp" 
	"Pipeline.cpp" 
	"Utils/PipelineBuilder.cpp" 
	"Utils/FramebufferBuilder.cpp"
	"CommandBuffer.cpp" 
	"BufferGPU.cpp"
	"DescriptorsBuffer.cpp"
PRIVATE
	"VulkanContext.hpp"
	"Swapchain.hpp"
	"Framebuffer.hpp"
	"Pipeline.hpp"
	"Utils/Builders.hpp"
	"CommandBuffer.hpp"
	"BufferGPU.hpp"
	"DescriptorsBuffer.hpp"
)


# -------------- definitions ---------------------
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)
find_package(vulkan-memory-allocator REQUIRED FATAL_ERROR)

target_link_libraries(${PROJECT_NAME}
	Vulkan::Vulkan 
	vulkan-memory-allocator::vulkan-memory-allocator
	glm::glm
)

#---------------- Link third-party libraries ---------------

FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        main #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
target_link_libraries(${PROJECT_NAME} vk-bootstrap::vk-bootstrap)

