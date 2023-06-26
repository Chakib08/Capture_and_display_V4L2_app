cmake_minimum_required(VERSION 2.8)
project(Capture_and_display_V4L2_app)
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)


set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-arch=sm_30)
include_directories(${CUDA_INCLUDE_DIST})


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(${PROJECT_NAME} main.cpp v4l2Device.cpp)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${CUDA_LIBRARIES})
