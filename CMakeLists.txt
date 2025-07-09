cmake_minimum_required (VERSION 3.8)

project ("OpenGL_msdf")

add_subdirectory(deps/glad)
add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
#add_subdirectory(deps/freetype)
add_subdirectory(msdf-atlas-gen)

#include_directories(deps/glfw/include)
include_directories(deps/glad/include)

file(GLOB_RECURSE EXAMPLE_SOURCES LIST_DIRECTORIES true src/*.cpp)

add_executable (opengl_msdfatlas ${EXAMPLE_SOURCES})

#  copy provided CCO font file to executable directory
add_custom_command(TARGET opengl_msdfatlas POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/JupiteroidRegular.ttf
        $<TARGET_FILE_DIR:opengl_msdfatlas>/JupiteroidRegular.ttf
    COMMENT "Copying jupiteroidRegular.ttf to executable directory"
)

target_link_libraries(opengl_msdfatlas PRIVATE glfw)
target_link_libraries(opengl_msdfatlas PRIVATE glad)
target_link_libraries(opengl_msdfatlas PRIVATE glm)
target_link_libraries(opengl_msdfatlas PRIVATE freetype)
target_link_libraries(opengl_msdfatlas PRIVATE msdf-atlas-gen)

target_compile_features(opengl_msdfatlas PRIVATE cxx_std_20)