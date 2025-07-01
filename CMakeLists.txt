cmake_minimum_required(VERSION 3.25.1...4.1.0)

project(listening_log_server LANGUAGES CXX)

include_directories(include)

add_executable(listening_log_server src/main.cpp)

target_link_libraries(${PROJECT_NAME} pthread boost_system)
