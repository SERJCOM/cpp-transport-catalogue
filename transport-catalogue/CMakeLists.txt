project(transport)


set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD 17)


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


add_executable(debug
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



 target_compile_options(transport
 PRIVATE
 -g

 )

 target_compile_options(debug
 PRIVATE
 -g
 -fsanitize=address
 )