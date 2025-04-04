cmake_minimum_required(VERSION 3.6)

project(smpp-proxy
        LANGUAGES CXX
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
#   set(CMAKE_BUILD_TYPE Debug)
endif()

set(Maintainer "Majid Darvishan (majiddarvishan@outlook.com)")

# add_compile_options(-Werror -Wall -Wno-error=variadic-macros)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -Wextra -Wshadow -Wno-deprecated -Wall -Wpointer-arith -Wstrict-aliasing -fPIC")

#if want create coverage file
#cmake -DMAKE_COVERAGE=1 ..

#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME prometheus-cpp
  VERSION 1.2.4
  GIT_REPOSITORY "https://github.com/jupp0r/prometheus-cpp.git"
  GIT_TAG v1.2.4
  OPTIONS "BUILD_SHARED_LIBS ON"
          "ENABLE_TESTING OFF"
          "OVERRIDE_CXX_STANDARD_FLAGS OFF"
          "GENERATE_PKGCONFIG OFF"
)

CPMAddPackage(
  NAME fmt
  VERSION 10.2.1
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG 10.2.1
  OPTIONS "FMT_INSTALL OFF"
          "FMT_TEST OFF"
)

CPMAddPackage(
  NAME spdlog
  VERSION 1.12.0
  GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
  GIT_TAG v1.12.0
  OPTIONS "SPDLOG_BUILD_SHARED ON"
          "SPDLOG_BUILD_EXAMPLE OFF"
          "SPDLOG_BUILD_EXAMPLE_HO OFF"
          "SPDLOG_BUILD_TESTS OFF"
          "SPDLOG_BUILD_TESTS_HO OFF"
          "SPDLOG_BUILD_BENCH OFF"
          "SPDLOG_SANITIZE_ADDRESS OFF"
          "SPDLOG_BUILD_WARNINGS OFF"
          "SPDLOG_INSTALL OFF"
          "SPDLOG_FMT_EXTERNAL_HO ON"
)

CPMAddPackage(
  NAME Boost
  VERSION 1.86.0
  GITHUB_REPOSITORY "boostorg/boost"
  GIT_TAG "boost-1.86.0"
)

# ######### define variables ##########################
execute_process(
  COMMAND git rev-parse --verify HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# add_definitions(-DREVISION="${REVISION}")
add_compile_definitions(GIT_REVISION="${GIT_REVISION}")

execute_process(
  COMMAND git config --get remote.origin.url
  COMMAND awk "{print $NF}"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(GIT_PATH="${GIT_PATH}")

execute_process(
  COMMAND git --version
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(GIT_VERSION="${GIT_VERSION}")

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(GIT_BRANCH="${GIT_BRANCH}")

execute_process(
  COMMAND git diff
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GITDIFF
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GITDIFF STREQUAL "")
  set(LOCAL_CHANGES 0)
else(GITDIFF STREQUAL "")
  set(LOCAL_CHANGES 1)
endif(GITDIFF STREQUAL "")

add_compile_definitions(LOCAL_CHANGES=${LOCAL_CHANGES})

add_compile_definitions(MAJOR_VERSION="${PROJECT_VERSION_MAJOR}")
add_compile_definitions(MINOR_VERSION="${PROJECT_VERSION_MINOR}")
add_compile_definitions(PATCH_VERSION="${PROJECT_VERSION_PATCH}")

execute_process(
  COMMAND lsb_release -ds
  OUTPUT_VARIABLE SERVER_OS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(SERVER_OS="${SERVER_OS}")

execute_process(
  COMMAND ip route get 8.8.8.8
  # COMMAND awk "NR==1 {print $NF}"
  COMMAND cut -d: -f2
  COMMAND awk "{print $7}"
  OUTPUT_VARIABLE SERVER_IP
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(SERVER_IP="${SERVER_IP}")

execute_process(
  COMMAND date
  OUTPUT_VARIABLE SERVER_TIME
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(SERVER_TIME="${SERVER_TIME}")
# ##################################################################################################

add_executable(${CMAKE_PROJECT_NAME}
                main.cpp
                smpp_proxy.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}
                            PUBLIC
                              "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>"
)

target_link_libraries(${CMAKE_PROJECT_NAME}
                        PRIVATE
                          prometheus-cpp::pull
                          spdlog::spdlog_header_only
                          Boost::asio
                          Boost::bimap
)