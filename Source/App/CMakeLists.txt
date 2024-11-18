cmake_minimum_required(VERSION 3.13)
project(App)

add_library(${PROJECT_NAME} SHARED)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${GLOBAL_CXX_STANDARD})  
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD ${GLOBAL_C_STANDARD})  

target_sources(${PROJECT_NAME}
INTERFACE
	"App.h"
PRIVATE
  "dllmain.cpp"
  "Game.hpp"
)

target_include_directories(${PROJECT_NAME} 
PRIVATE
	${SOURCE_DIR}/Graphics/include
	${SOURCE_DIR}/Core
)
target_link_libraries(${PROJECT_NAME} Graphics Core)

target_link_libraries(${PROJECT_NAME} 
	Catch2::Catch2WithMain
)

# -------------- definitions ---------------------
set(${PROJECT_NAME}_COMPILE_DEFINITIONS
	-DAPP_BUILD
)

add_definitions(${${PROJECT_NAME}_COMPILE_DEFINITIONS})

if (MSVC)
	#add_compile_definitions(_HAS_ITERATOR_DEBUGGING=0) # disable iterator debugging in MSVC
endif()