cmake_minimum_required(VERSION 3.15)

project(enigma VERSION 0.1.0
        DESCRIPTION "Enigma machine simulator"
        LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(FILE_PATH_JSON "${CMAKE_SOURCE_DIR}/src/data/enigma.json")
add_definitions(-DFILE_PATH_JSON="${FILE_PATH_JSON}")

set(FILE_PATH_IC "${CMAKE_SOURCE_DIR}/src/data/IC.txt")
add_definitions(-DFILE_PATH_IC="${FILE_PATH_IC}")

set(FILE_PATH_CYCLO "${CMAKE_SOURCE_DIR}/src/data/cyles.txt")
add_definitions(-DFILE_PATH_CYCLO="${FILE_PATH_CYCLO}")

set(FILE_PATH_ENIGMA "${CMAKE_SOURCE_DIR}/src/gui/glade/enigma.glade")
add_definitions(-DFILE_PATH_ENIGMA="${FILE_PATH_ENIGMA}")

set(FILE_PATH_START "${CMAKE_SOURCE_DIR}/src/gui/glade/start.glade")
add_definitions(-DFILE_PATH_START="${FILE_PATH_START}")

set(FILE_PATH_BOMB "${CMAKE_SOURCE_DIR}/src/gui/glade/bomb.glade")
add_definitions(-DFILE_PATH_BOMB="${FILE_PATH_BOMB}")

set(FILE_PATH_CONFIGURATIONS "${CMAKE_SOURCE_DIR}/src/data/configs.txt")
add_definitions(-DFILE_PATH_CONFIGURATIONS="${FILE_PATH_CONFIGURATIONS}")

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

cmake_minimum_required(VERSION 3.10)
project(MyProject)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (DEFINED ENV{MINGW_INCLUDE_DIR})
        set(MINGW_INCLUDE_DIR $ENV{MINGW_INCLUDE_DIR})
    else()
        set(MINGW_INCLUDE_DIR "C:/msys64/mingw64/include")
    endif()

    if (DEFINED ENV{MINGW_LIB_DIR})
        set(MINGW_LIB_DIR $ENV{MINGW_LIB_DIR})
    else()
        set(MINGW_LIB_DIR "C:/msys64/mingw64/lib")
    endif()

    include_directories(${MINGW_INCLUDE_DIR})
    link_directories(${MINGW_LIB_DIR})

    message(STATUS "MinGW Include Directory: ${MINGW_INCLUDE_DIR}")
    message(STATUS "MinGW Library Directory: ${MINGW_LIB_DIR}")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (DEFINED ENV{LINUX_INCLUDE_DIR})
        set(LINUX_INCLUDE_DIR $ENV{LINUX_INCLUDE_DIR})
    else()
        set(LINUX_INCLUDE_DIR "/usr/include")
    endif()

    if (DEFINED ENV{LINUX_LIB_DIR})
        set(LINUX_LIB_DIR $ENV{LINUX_LIB_DIR})
    else()
        set(LINUX_LIB_DIR "/usr/lib")
    endif()

    include_directories(${LINUX_INCLUDE_DIR})
    link_directories(${LINUX_LIB_DIR})

    message(STATUS "Linux Include Directory: ${LINUX_INCLUDE_DIR}")
    message(STATUS "Linux Library Directory: ${LINUX_LIB_DIR}")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (DEFINED ENV{DARWIN_INCLUDE_DIR})
        set(DARWIN_INCLUDE_DIR $ENV{DARWIN_INCLUDE_DIR})
    else()
        set(DARWIN_INCLUDE_DIR "/usr/local/include")
    endif()

    if (DEFINED ENV{DARWIN_LIB_DIR})
        set(DARWIN_LIB_DIR $ENV{DARWIN_LIB_DIR})
    else()
        set(DARWIN_LIB_DIR "/usr/local/lib")
    endif()

    include_directories(${DARWIN_INCLUDE_DIR})
    link_directories(${DARWIN_LIB_DIR})

    message(STATUS "Darwin Include Directory: ${DARWIN_INCLUDE_DIR}")
    message(STATUS "Darwin Library Directory: ${DARWIN_LIB_DIR}")

endif()


##################### Backend #####################
add_subdirectory(src)
add_executable(enigma src/main.c)

target_link_libraries(enigma enigma_lib ${GTK3_LIBRARIES} cjson)

