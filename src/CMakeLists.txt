# Source files
file(GLOB_RECURSE SOURCES
    *.cpp
)

add_executable(${CMAKE_PROJECT_NAME}
                ${SOURCES}
)

target_include_directories(${CMAKE_PROJECT_NAME}
                            PUBLIC
                              "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/src"
)

target_link_libraries(${CMAKE_PROJECT_NAME}
                        PRIVATE
                          prometheus-cpp::pull
                          spdlog::spdlog_header_only
                          Boost::asio
                          Boost::bimap
)