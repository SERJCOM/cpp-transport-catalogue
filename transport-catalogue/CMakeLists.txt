cmake_minimum_required(VERSION 3.18.4)

project(TransportCatalogue)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD 17)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto svg.proto)

set(files main.cpp transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp 
json_reader.h json_reader.cpp request_handler.h request_handler.cpp map_renderer.h map_renderer.cpp geo.h geo.cpp transport_router.h transport_router.cpp serialization.h serialization.cpp
transport_catalogue.proto
)
set(my_libs json.cpp svg.cpp json_builder.cpp)


add_executable(transport_catalogue ${files} ${my_libs} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)






