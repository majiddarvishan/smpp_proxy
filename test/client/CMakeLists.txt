add_executable(test_client smpp_test_client.cpp)

# target_include_directories(test_client
#                             PUBLIC
#                               "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/src"
# )

target_link_libraries(test_client
                        PRIVATE
                          spdlog::spdlog_header_only
                          Boost::asio
                          Boost::bimap
)