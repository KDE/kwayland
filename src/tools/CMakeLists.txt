include(ECMMarkAsTest)

set(scannerSRCS generator.cpp)

add_definitions(-DMAPPING_FILE="${CMAKE_CURRENT_SOURCE_DIR}/mapping.txt")
add_executable(kwaylandScanner ${scannerSRCS})
target_link_libraries(kwaylandScanner Qt6::Core Qt6::Concurrent)
ecm_mark_as_test(kwaylandScanner)
