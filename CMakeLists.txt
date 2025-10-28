cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ComputerGraphics C CXX)

# Set this before including framework such that it knows to use the OpenGL4.5 version of GLAD
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
	add_subdirectory("../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_executable(Master_TechDemo ${SRC_FILES})

target_compile_definitions(Master_TechDemo PRIVATE RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_compile_features(Master_TechDemo PRIVATE cxx_std_20)
target_link_libraries(Master_TechDemo PRIVATE CGFramework)
enable_sanitizers(Master_TechDemo)
set_project_warnings(Master_TechDemo)

# Copy all files in the resources folder to the build directory after every successful build.
add_custom_command(TARGET Master_TechDemo POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_LIST_DIR}/resources/" "$<TARGET_FILE_DIR:Master_TechDemo>/resources/")

# Copy all *.glsl shader files from shaders/ and its subfolders to the build directory, preserving the directory structure.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

set(Master_TechDemo_shader_copies "")
# List all *.glsl files in the shaders folder and its subfolders
file(GLOB_RECURSE Master_TechDemo_shader_sources RELATIVE "${CMAKE_CURRENT_LIST_DIR}/shaders" "${CMAKE_CURRENT_LIST_DIR}/shaders/*.glsl")
foreach (rel_shader_path IN LISTS Master_TechDemo_shader_sources)
	set(src "${CMAKE_CURRENT_LIST_DIR}/shaders/${rel_shader_path}")
	set(dst "${CMAKE_CURRENT_BINARY_DIR}/shaders/${rel_shader_path}")
	get_filename_component(dst_dir "${dst}" DIRECTORY)
	file(MAKE_DIRECTORY "${dst_dir}")
	add_custom_command(
		OUTPUT "${dst}"
		COMMAND ${CMAKE_COMMAND} -E copy "${src}" "${dst}"
		DEPENDS "${src}"
		COMMENT "Copying shader: ${rel_shader_path}"
	)
	list(APPEND Master_TechDemo_shader_copies "${dst}")
endforeach()
add_custom_target(Master_TechDemo_copy_shaders DEPENDS ${Master_TechDemo_shader_copies})
add_dependencies(Master_TechDemo Master_TechDemo_copy_shaders)

