cmake_minimum_required(VERSION 2.8)

project(qcpp)

#include_directories(/usr/include/nvidia-current-updates)
#link_directories(/usr/lib/nvidia-current-updates)

set(CMAKE_CXX_FLAGS "-std=gnu++11")

add_executable(qcpp 
	main.cpp
	QMachine.cc
	QGate.cc
	QMeasurement.cc
	StandardGates.cc
)

target_link_libraries(qcpp OpenCL)
