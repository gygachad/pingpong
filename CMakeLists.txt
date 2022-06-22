cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(pingpong VERSION ${PROJECT_VESRION})

configure_file(version.h.in version.h)

file(COPY ${CMAKE_BINARY_DIR}/version.h
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
		
add_executable(pingpong_server 
					server/main.cpp 
					server/srv_session.cpp
                    server/player.cpp
					server/server.cpp 
					connection.cpp 
					str_tool.cpp 
					version.cpp)
					
add_executable(pingpong_client 
					client/main.cpp 
					client/clnt_session.cpp 
					connection.cpp 
					str_tool.cpp 
					version.cpp)

set_target_properties(pingpong_server pingpong_client PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(pingpong_server
    PRIVATE "${CMAKE_BINARY_DIR}" ${ASIO_INCLUDE_DIR}/include
)

target_include_directories(pingpong_client
    PRIVATE "${CMAKE_BINARY_DIR}" ${ASIO_INCLUDE_DIR}/include
)

if (MSVC)
    target_compile_options(pingpong_server PRIVATE /W4)
	target_compile_options(pingpong_client PRIVATE /W4)
    target_compile_definitions(pingpong_server PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(pingpong_client PRIVATE _WIN32_WINNT=0x0601)
	add_compile_definitions(SET_WIN_CONSOLE_MODE)
else ()
    target_compile_options(pingpong_server PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
	target_compile_options(pingpong_client PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_link_libraries(pingpong_server pthread)
    target_link_libraries(pingpong_client pthread)
endif()

install(TARGETS pingpong_server pingpong_client RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

if (WIN32)
    set(CPACK_GENERATOR ZIP)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)