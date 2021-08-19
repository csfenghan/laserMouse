cmake_minimum_required(VERSION 3.0)
project(laserMouse)

set(CMAKE_BUILD_TYPE "Debug")

find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories("include")

add_executable(laserMouse src/main.cpp 
                            src/calibrate.cpp
                            src/detect.cpp
                            src/mouse_control.cpp
                            src/location.cpp)
target_link_libraries(laserMouse ${OpenCV_LIBS} ${X11_LIBRARIES} pthread)
