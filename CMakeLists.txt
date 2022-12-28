cmake_minimum_required(VERSION 3.5)

project(chronograph LANGUAGES CXX)
file(GLOB SOURCE_FILES "src/*.cpp")

add_executable(chronograph ${SOURCE_FILES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(BOOSTROOT "D:/Projects/boost/boost_1_80_0")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.80.0 ) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    # add_executable(progname file1.cxx file2.cxx) 
    target_link_libraries(chronograph ${Boost_LIBRARIES})
endif()

# Set the linker flags to use static libraries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
