cmake_minimum_required(VERSION 3.7.1)

project(h4net)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(h4net src/buffer.cpp src/app.cpp src/main.cpp)
target_link_libraries(h4net Threads::Threads)
