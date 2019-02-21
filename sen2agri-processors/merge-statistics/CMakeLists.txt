cmake_minimum_required (VERSION 2.6)

project(count-columns)
add_executable(count-columns count-columns.cpp entry.cpp statistics-reader.cpp)

project(merge-statistics)
add_executable(merge-statistics merge-statistics.cpp entry.cpp statistics-reader.cpp)

project(cat-columns)
add_executable(cat-columns cat-columns.cpp entry.cpp statistics-reader.cpp)

project(gapfill-statistics)
add_executable(gapfill-statistics gapfill-statistics.cpp entry.cpp statistics-reader.cpp)

project(merge-counts)
add_executable(merge-counts merge-counts.cpp entry.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

install(TARGETS
            count-columns
            merge-statistics
            cat-columns
            gapfill-statistics
            merge-counts
        DESTINATION usr/bin
)
