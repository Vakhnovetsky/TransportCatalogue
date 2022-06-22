cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

set(HEADERS_FILES
	"headers/domain.h"
	"headers/geo.h"
	"headers/graph.h"
	"headers/json.h"
	"headers/json_builder.h"
	"headers/json_reader.h"
	"headers/map_renderer.h"
	"headers/ranges.h"
	"headers/request_handler.h"
	"headers/router.h"
	"headers/serialization.h"
	"headers/svg.h"
	"headers/transport_catalogue.h"
	"headers/transport_router.h")

set(SOURCE_FILES
	"main.cpp" 
	"source/domain.cpp"
	"source/geo.cpp"
	"source/json.cpp"
	"source/json_builder.cpp"
	"source/json_reader.cpp"
	"source/map_renderer.cpp"
	"source/request_handler.cpp"
	"source/serialization.cpp"
	"source/svg.cpp"
	"source/transport_catalogue.cpp"
	"source/transport_router.cpp")

set(PROTO_FILES
	graph.proto
	map_renderer.proto
	svg.proto
	transport_catalogue.proto
	transport_router.proto)

# Команда вызова protoc
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# добавляем цель
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${HEADERS_FILES} ${SOURCE_FILES} ${PROTO_FILES})

target_include_directories(transport_catalogue PRIVATE "headers")

# find_package определила переменную Protobuf_INCLUDE_DIRS, которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

# Также find_package определила Protobuf_LIBRARY.
# Protobuf зависит от библиотеки Threads. Добавим и её при компоновке.
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)