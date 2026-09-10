cmake_minimum_required(VERSION 3.5)
set( TARGET_NAME Cloud )
project( ${TARGET_NAME} CXX C)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE sources_files sources/*)

include_directories(externals)

add_executable(${TARGET_NAME} ${sources_files})
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

if( EMSCRIPTEN )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )

	set( CMAKE_EXECUTABLE_SUFFIX ".html" )
	#add_definitions( -s USE_SDL=2 -s LEGACY_GL_EMULATION=1 -std=c++1y )
	add_definitions( -s USE_SDL=2 )
	# RELEASE
	set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-O2 -s USE_SDL=2 -s TOTAL_MEMORY=67108864 -s USE_WEBGL2=1" )
	# DEV
	# set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-s USE_SDL=2" )
	set_property( TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14 )
	target_link_libraries( ${TARGET_NAME} ${DEPENDENCIES_LIBRARIES} )
else()
	find_package(SDL2 CONFIG REQUIRED)
	target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2)
        find_package(glm CONFIG REQUIRED)
        target_link_libraries(${TARGET_NAME} PRIVATE glm::glm-header-only)
endif()


if (CMAKE_HOST_WIN32 AND NOT EMSCRIPTEN)
    add_definitions(-DNOMINMAX)
endif (CMAKE_HOST_WIN32 AND NOT EMSCRIPTEN)

