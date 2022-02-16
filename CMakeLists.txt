cmake_minimum_required(VERSION 3.20)
project(DistributedComputationalGraph)

set(CMAKE_CXX_STANDARD 20)

# Kafka
add_subdirectory(3rdparty/modern-cpp-kafka/include)

# Redis
set(REDIS_PLUS_PLUS_BUILD_SHARED OFF)
set(REDIS_PLUS_PLUS_BUILD_STATIC ON)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF)
set(REDIS_PLUS_PLUS_CXX_STANDARD 20)
add_subdirectory(3rdparty/redis-plus-plus)

# RPC
add_subdirectory(3rdparty/rpclib)

# Plog
add_subdirectory(3rdparty/plog)

# Cxx opts
set(CXXOPTS_BUILD_EXAMPLES off)
set(CXXOPTS_BUILD_TESTS off)
add_subdirectory(3rdparty/cxxopts)

# Eigen
find_package(Eigen3 3.4 REQUIRED)

# Json
find_package(nlohmann_json REQUIRED)

set(LIBS modern-cpp-kafka-api plog cxxopts rpc hiredis redis++_static)

add_library(tensor_codecs STATIC src/TensorCodecs.cpp)
target_link_libraries(tensor_codecs rpc Eigen3::Eigen)

add_library(operations STATIC src/Operations.cpp src/OperationsManager.cpp)
target_link_libraries(operations rpc Eigen3::Eigen tensor_codecs)

add_executable(manager src/manager.cpp src/Manager.cpp src/common.cpp)
target_link_libraries(manager ${LIBS})

add_executable(worker src/worker.cpp src/Worker.cpp src/common.cpp)
target_link_libraries(worker ${LIBS} operations)

add_executable(client src/client.cpp src/Client.cpp src/common.cpp)
target_link_libraries(client ${LIBS} tensor_codecs operations nlohmann_json::nlohmann_json)