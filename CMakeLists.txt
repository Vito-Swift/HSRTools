<<<<<<< HEAD
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(HSRTool LANGUAGES CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -mavx2 -O3")

add_executable("${PROJECT_NAME}_Encode_Stage1" prototype/HID.cpp)
=======
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(HSRTool LANGUAGES CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -mavx2 -O3")

add_executable("${PROJECT_NAME}_Encode_Stage1" prototype/HID.cpp)
add_executable(isal prototype/testisal.cpp)
>>>>>>> 7da68982efd244f94006de3ad3a600e1fcb536eb
