cmake_minimum_required(VERSION 3.13)
project(RHI)

add_library(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${GLOBAL_CXX_STANDARD})  
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD ${GLOBAL_C_STANDARD})

# ----------------------- build library -------------------------

target_sources (${PROJECT_NAME}
PRIVATE
	"Context.hpp"
)

#add_subdirectory(AssetLoader)
add_subdirectory(Vulkan)
#add_subdirectory(DirectXImpl)

target_link_libraries(${PROJECT_NAME} 
	Catch2::Catch2WithMain
)

target_include_directories(${PROJECT_NAME} 
PRIVATE
   "./"
	${SOURCE_DIR}/Core
)

# -------------- link system libraries -----------
set(${PROJECT_NAME}_SYSTEM_LIBRARIES 
)
target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_SYSTEM_LIBRARIES})

if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

