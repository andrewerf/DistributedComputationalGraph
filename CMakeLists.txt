cmake_minimum_required(VERSION 3.20)
project(DistributedComputationalGraph)

set(CMAKE_CXX_STANDARD 20)

# Kafka
add_subdirectory(3rdparty/modern-cpp-kafka/include)

# Redis
set(REDIS_PLUS_PLUS_BUILD_STATIC OFF)
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


set(LIBS modern-cpp-kafka-api plog cxxopts rpc redis++)

add_executable(manager src/Manager.cpp)
target_link_libraries(manager ${LIBS})
