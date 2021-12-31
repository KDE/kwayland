# The server part of this repo has been moved to Plasma kwayland-server since 5.73
if(EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.74.0)
    add_subdirectory(testserver)
endif()

include(ECMMarkAsTest)

set(scannerSRCS generator.cpp)

add_definitions(-DMAPPING_FILE="${CMAKE_CURRENT_SOURCE_DIR}/mapping.txt")
add_executable(kwaylandScanner ${scannerSRCS})
target_link_libraries(kwaylandScanner Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Concurrent)
ecm_mark_as_test(kwaylandScanner)
