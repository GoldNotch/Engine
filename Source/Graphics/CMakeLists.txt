cmake_minimum_required(VERSION 3.13)
project(Graphics)

add_library(${PROJECT_NAME} SHARED)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${GLOBAL_CXX_STANDARD})  
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD ${GLOBAL_C_STANDARD})

# ----------------------- build library -------------------------

target_compile_definitions(${PROJECT_NAME} 
PUBLIC
	GRAPHICS_BUILD
	DATA_PATH="Data"
)

target_sources (${PROJECT_NAME}
PRIVATE
	"include/Graphics.h"
	"include/Scene.h"  

PRIVATE
	"dllmain.cpp"
)

add_subdirectory(RHI)

target_link_libraries(${PROJECT_NAME} 
	Catch2::Catch2WithMain
	RHI
)

target_include_directories(${PROJECT_NAME} 
PRIVATE
   "include/"
	${SOURCE_DIR}/Core
	RHI/
)

# -------------- link system libraries -----------
set(${PROJECT_NAME}_SYSTEM_LIBRARIES 
)
target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_SYSTEM_LIBRARIES})

if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

file(REMOVE_RECURSE  ${DATA_DIR}/Shaders)
# copy shaders into bin
file(COPY Shaders DESTINATION ${DATA_DIR})
#file(RENAME ${DATA_DIR}/Shaders/Shaders ${DATA_DIR}/Shaders)

