add_executable(${CMAKE_PROJECT_NAME}
                main.cpp
                smpp_proxy.cpp
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