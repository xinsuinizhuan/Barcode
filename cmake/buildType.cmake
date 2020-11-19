if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DCV_DEBUG)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
endif ()
