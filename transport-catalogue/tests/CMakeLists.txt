project(tests)
cmake_minimum_required(VERSION 3.25)

find_package(Catch2 3 REQUIRED)
enable_testing ()

add_executable(test1 test1.cpp test2.cpp)
target_link_libraries(test1 PRIVATE Catch2::Catch2WithMain)

add_test(NAME MyTest COMMAND test1)