cmake_minimum_required(VERSION 2.8.4)
project(lab04)

set(SOURCE_FILES convolve.cpp)
set(OIIO_LIB "-lOpenImageIO")
set(OPENGL_LIB "-framework Foundation -framework GLUT -framework OpenGL")
set (GCC_COVERAGE_LINK_FLAGS "${OIIO_LIB} ${OPENGL_LIB}")
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

add_executable(filt ${SOURCE_FILES})