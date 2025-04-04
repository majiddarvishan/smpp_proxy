add_executable(test_server smpp_test_server.cpp)

# target_include_directories(test_server
#                             PUBLIC
#                               "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/src"
# )

target_link_libraries(test_server
                        PRIVATE
                          spdlog::spdlog_header_only
                          Boost::asio
                          Boost::bimap
)