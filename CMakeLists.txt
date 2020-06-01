cmake_minimum_required(VERSION 3.5)
project(water)


set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    src/common.h
    src/glad.c
    src/main.cpp
    src/ShaderProgram.h
    src/ShaderProgram.cpp
    )

set(ADDITIONAL_INCLUDE_DIRS
        dependencies/include/GLAD)
set(ADDITIONAL_LIBRARY_DIRS
        dependencies/lib)   
set(ADDITIONAL_RUNTIME_LIBRARY_DIRS
        dependencies/bin)   

set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

if(WIN32)
  set(ADDITIONAL_INCLUDE_DIRS 
        ${ADDITIONAL_INCLUDE_DIRS}
        dependencies/include)
  link_directories(${ADDITIONAL_LIBRARY_DIRS})
else()
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
  find_package(glfw3 REQUIRED)
endif()

include_directories(${ADDITIONAL_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

add_executable(water ${SOURCE_FILES})

target_include_directories(water PRIVATE ${OPENGL_INCLUDE_DIR})
add_custom_command(TARGET water POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}/shaders")

if(WIN32)
  add_custom_command(TARGET water POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dependencies/bin" $<TARGET_FILE_DIR:water>)
  #set(CMAKE_MSVCIDE_RUN_PATH ${ADDITIONAL_RUNTIME_LIBRARY_DIRS})
  target_compile_options(water PRIVATE)
  target_link_libraries(water LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw3dll)
else()
  target_compile_options(water PRIVATE -Wnarrowing)
  target_link_libraries(water LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw rt dl)
endif()
