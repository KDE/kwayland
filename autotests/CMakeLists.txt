include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

add_subdirectory(client)

# The server part of this repo has been moved to Plasma kwayland-server since 5.73
if(EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.74.0)
    add_subdirectory(server)
endif()
