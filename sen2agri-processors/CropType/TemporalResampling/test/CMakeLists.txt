find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(ITK REQUIRED)

add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(TestTemporalResampling TestTemporalResampling.cpp)

target_include_directories(TestTemporalResampling PUBLIC ..)

target_link_libraries(TestTemporalResampling
    "${Boost_LIBRARIES}"
    "${ITK_LIBRARIES}")
add_test(TestTemporalResampling TestTemporalResampling)

add_executable(TestTemporalMerging TestTemporalMerging.cpp)

target_include_directories(TestTemporalMerging PUBLIC ..)

target_link_libraries(TestTemporalMerging
    "${Boost_LIBRARIES}"
    "${ITK_LIBRARIES}")
add_test(TestTemporalMerging TestTemporalMerging)

