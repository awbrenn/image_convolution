cmake_minimum_required(VERSION 2.8.4)
project(lab04)

set(SOURCE_FILES convolve.cpp)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(OIIO_INC "")
    set(OIIO_LIB "-lOpenImageIO")
    set(OPENGL_LIB "-framework Foundation -framework GLUT -framework OpenGL")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(OIIO_INC "-I/group/dpa/include")
    set(OIIO_LIB "-L/group/dpa/lib -lOpenImageIO")
    set(OPENGL_LIB "-lglut lGL -lGLU")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

set (GCC_COVERAGE_LINK_FLAGS "${OIIO_INC} ${OIIO_LIB} ${OPENGL_LIB}")
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

add_executable(filt ${SOURCE_FILES})