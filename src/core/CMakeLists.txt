cmake_minimum_required (VERSION 3.0)

project (bonobo
	VERSION 0.2
	LANGUAGES CXX
)

set (
	SOURCES

	"Bonobo.cpp"
	"InputHandler.cpp"
	"Log.cpp"
	"LogView.cpp"
	"Misc.cpp"
	"opengl.cpp"
	"Types.cpp"
	"various.cpp"
	"Window.cpp"
)

add_library (${PROJECT_NAME} ${SOURCES})

target_include_directories (${PROJECT_NAME} PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
	target_compile_options (${PROJECT_NAME} PRIVATE "-std=c++14")
endif ()
target_compile_options (${PROJECT_NAME} PRIVATE "-Wall")

target_link_libraries (${PROJECT_NAME} ${IMGUI_LIBRARY} external_libs)
if (USE_SYSTEM_GLFW)
	target_link_libraries (${PROJECT_NAME} glfw)
else ()
	target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/glfw/include")
	target_link_libraries (${PROJECT_NAME} ${GLFW_LIBRARY})
endif ()
if (APPLE)
	find_library (COCOA_LIBRARY Cocoa)
	find_library (IOKIT_LIBRARY IOKit)
	find_library (CORE_VIDEO_LIBRARY CoreVideo)
	find_library (CORE_FOUNDATION_LIBRARY CoreFoundation)

	target_link_libraries (
		${PROJECT_NAME}

		${COCOA_LIBRARY}
		${IOKIT_LIBRARY}
		${CORE_VIDEO_LIBRARY}
		${CORE_FOUNDATION_LIBRARY}
	)
endif ()
