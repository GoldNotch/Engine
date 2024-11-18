cmake_minimum_required(VERSION 3.13)

project(Launcher)

add_executable(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${GLOBAL_CXX_STANDARD})  
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD ${GLOBAL_C_STANDARD})  

target_sources(${PROJECT_NAME}
PUBLIC
  "main.cpp"
PRIVATE
  "Process.cpp"
  "Process.hpp"
  #"glew/glew.c"

)

target_include_directories(${PROJECT_NAME} 
PRIVATE 
	${SOURCE_DIR}/App
	${SOURCE_DIR}/Core
	${SOURCE_DIR}/Graphics/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE App)

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_include_directories(${PROJECT_NAME} PRIVATE ${glfw3_INCLUDE_DIRS})

#target_include_directories(${PROJECT_NAME} PRIVATE glew)

if (MSVC)
	#add_compile_definitions(_HAS_ITERATOR_DEBUGGING=0) # disable iterator debugging in MSVC
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()