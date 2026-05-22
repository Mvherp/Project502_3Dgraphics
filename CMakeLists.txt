# CMakeList.txt : CMake project for Project502MVHerpHCaeyman, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("InfoF502Project")


find_package(OpenGL REQUIRED)

#for glad library
add_library( glad STATIC 3rdParty/glad/src/glad.c "texture.h")
set(GLAD_INCLUDE "3rdParty/glad/include")
target_include_directories(glad PUBLIC ${GLAD_INCLUDE})

#for GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)

#Search for glfw in case it is installed
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
    #if glfw is not found, we will use the sources from the submodules
    add_subdirectory(3rdParty/glfw)
    include_directories(3rdParty/glfw/include/)
endif()


include_directories(3rdParty/glad/include/
                    3rdParty/glm/
                    3rdParty/stb/)


# Add source to this project's executable.
set(Main "Main.cpp" "camera.h" "shader.h" "object.h")


#These commands are there to specify the path to the folder containing the object and textures files as macro
#With these you can just use PATH_TO_OBJECTS and PATH_TO_TEXTURE in your c++ code and the compiler will replace it by the correct expression
add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/textures")
add_compile_definitions(PATH_TO_SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/shaders")

add_executable(${PROJECT_NAME}_exe ${Main} "texture.h")
target_link_libraries(${PROJECT_NAME}_exe PUBLIC OpenGL::GL glfw glad)



# TODO: Add tests and install targets if needed.
