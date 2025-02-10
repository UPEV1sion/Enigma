cmake_minimum_required(VERSION 3.15)

project(enigma VERSION 0.1.0
        DESCRIPTION "Enigma machine simulator"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Although I've provided MSCV support, I recommend GCC or Clang, because of the better compiler flags.
# Clang offers exceptional performance while GCC is the standard.
if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Wunused-result -march=native")
endif ()

set(FILE_PATH_JSON "${CMAKE_SOURCE_DIR}/src/data/enigma.json")
add_definitions(-DFILE_PATH_JSON="${FILE_PATH_JSON}")

set(FILE_PATH_IC "${CMAKE_SOURCE_DIR}/src/data/IC.txt")
add_definitions(-DFILE_PATH_IC="${FILE_PATH_IC}")

set(FILE_PATH_CYCLO "${CMAKE_SOURCE_DIR}/src/data/cycles.txt")
add_definitions(-DFILE_PATH_CYCLO="${FILE_PATH_CYCLO}")

set(FILE_PATH_ENIGMA "${CMAKE_SOURCE_DIR}/src/gui/glade/enigma.glade")
add_definitions(-DFILE_PATH_ENIGMA="${FILE_PATH_ENIGMA}")

set(FILE_PATH_START "${CMAKE_SOURCE_DIR}/src/gui/glade/start.glade")
add_definitions(-DFILE_PATH_START="${FILE_PATH_START}")

set(FILE_PATH_BOMB "${CMAKE_SOURCE_DIR}/src/gui/glade/bomb.glade")
add_definitions(-DFILE_PATH_BOMB="${FILE_PATH_BOMB}")

set(FILE_PATH_CONFIGURATIONS "${CMAKE_SOURCE_DIR}/src/data/configs.txt")
add_definitions(-DFILE_PATH_CONFIGURATIONS="${FILE_PATH_CONFIGURATIONS}")

set(FILE_PATH_CRIB_CIPHER_CYCLE "${CMAKE_SOURCE_DIR}/src/data/crib_cipher_cycle.dot")
add_definitions(-DFILE_PATH_CRIB_CIPHER_CYCLE="${FILE_PATH_CRIB_CIPHER_CYCLE}")

if(GCC_COMPILER)
    find_package(OpenMP REQUIRED)
    if(OpenMP_C_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    endif()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif ()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if (ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")

    add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory coverage
            COMMAND lcov --capture --directory ../../../AppData/Local/Temp --output-file coverage/coverage.info
            COMMAND lcov --list coverage/coverage.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif ()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

find_package(PostgreSQL REQUIRED)
include_directories(/usr/include/postgresql)
##################### Backend #####################
add_subdirectory(src)
#add_executable(enigma src/main.c)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(enigma SHARED src/main.c)
set_target_properties(enigma_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(enigma enigma_lib ${GTK3_LIBRARIES} pthread PostgreSQL::PostgreSQL)


