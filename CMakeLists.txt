cmake_minimum_required(VERSION 2.8)

project(astra_test)

set(OPENNI2_DIR "/home/btree/下载/OpenNI-Linux-x64-2.3")
set(NITE2_DIR "/home/btree/NiTE-Linux-x64-2.2")
set(NI_LIB_DIRNAME "Redist")

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_GLIBCXX_USE_C99=1)

include_directories(
	${OPENNI2_DIR}/Include
    	${NITE2_DIR}/Include
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
	${OPENCV_INCLUDE}
    	${CMAKE_CURRENT_SOURCE_DIR}
	)

link_directories(
	${OPENCV_LIB}
	${OPENNI2_DIR}/${NI_LIB_DIRNAME}
	${NITE2_DIR}/${NI_LIB_DIRNAME}
	)

add_executable(${PROJECT_NAME}
	main.cpp
	Viewer.cpp
)

target_link_libraries(${PROJECT_NAME}
	OpenNI2
	NiTE2
	${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES}
	${OpenCV_LIBS}
)
