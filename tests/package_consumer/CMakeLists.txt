cmake_minimum_required(VERSION 3.16)

project(skill_rating_package_consumer LANGUAGES CXX)

find_package(skill_rating_cpp REQUIRED)

add_executable(package_consumer main.cpp)
target_link_libraries(package_consumer PRIVATE skill_rating::skill_rating)
