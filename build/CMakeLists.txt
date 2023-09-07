cmake_minimum_required(VERSION 3.26)
project(cppsdns)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CPP_COMPILER g++)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

set(LIBRARY_NAME cppsdns)

set(PUBLIC_PROJECT_PATH include/cppsdns)

set(PUBLIC_SOURCES 
    ${PUBLIC_PROJECT_PATH}/dnspack.h src/dnspack.cpp
    ${PUBLIC_PROJECT_PATH}/packetfs.h src/packetfs.cpp
    src/main.cpp
)

set(LINUX_PUBLIC_SOURCES
    ${PUBLIC_PROJECT_PATH}/bindserv.h src/bindserv_linux.cpp
)

set(WINDOWS_PUBLIC_SOURCES
    ${PUBLIC_PROJECT_PATH}/bindserv.h src/bindserv_windows.cpp
)

if (WIN32)
    add_executable(cppsdns ${PUBLIC_SOURCES} ${WINDOWS_PUBLIC_SOURCES})
    target_link_libraries(cppsdns wsock32 ws2_32)
endif (WIN32)

if (UNIX)
    add_executable(cppsdns ${PUBLIC_SOURCES} ${LINUX_PUBLIC_SOURCES})
endif (UNIX)

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

enable_testing()


