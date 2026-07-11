cmake_minimum_required(VERSION 3.12)
project(dangerpca VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Source files
set(SOURCES
    src/main.c
    src/database/db.c
    src/services/services.c
    src/ui/ui.c
    thirdparty/pdfgen.c
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SQLITE3 REQUIRED sqlite3)
    
    include_directories(${SDL2_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
    
    add_executable(dangerpca ${SOURCES})
    target_link_libraries(dangerpca PRIVATE ${SDL2_LIBRARIES} ${SQLITE3_LIBRARIES} m dl pthread)
else()
    # Windows/Fallback find_package
    find_package(SDL2 REQUIRED)
    find_package(SQLite3 REQUIRED)
    
    include_directories(${SDL2_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS})
    
    add_executable(dangerpca ${SOURCES})
    target_link_libraries(dangerpca PRIVATE SDL2::SDL2 SQLite::SQLite3)
endif()

# Copy assets folder to output build directory so running locally works perfectly
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build output directory"
)
add_dependencies(dangerpca copy_assets)

# Installation rules
install(TARGETS dangerpca DESTINATION bin)
install(DIRECTORY assets DESTINATION share/dangerpca)

# CPack configuration for DEB packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "dangerpca")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sistem ERP Sekolah Offline Desktop Berkinerja Tinggi")
set(CPACK_PACKAGE_CONTACT "hifni@dangerpca.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hifni")
set(CPACK_DEBIAN_PACKAGE_SECTION "education")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libsqlite3-0")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
