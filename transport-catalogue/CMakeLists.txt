project(transport)


set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD 17)


enable_testing()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(transport
 main.cpp 
 transport_catalogue.cpp
 domain.cpp 
 json_reader.cpp 
 request_handler.cpp 
 json.cpp
 svg.cpp
 map_renderer.cpp
 json_builder.cpp
 geo.cpp
 transport_router.cpp
 )




#  add_subdirectory(test /test)

#  add_test(NAME TEST1 COMMAND test1)






