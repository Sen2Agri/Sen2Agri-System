project(Composite)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(OTB REQUIRED)

INCLUDE(${OTB_USE_FILE})

otb_create_application(
  NAME           ComputeNDVI
  SOURCES        ComputeNDVI.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

otb_create_application(
  NAME           DirectionalCorrection
  SOURCES        DirectionalCorrection.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

otb_create_application(
  NAME           UpdateSynthesis
  SOURCES        updatesynthesisfunctor.cpp UpdateSynthesis.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})


if(BUILD_TESTING)
  add_subdirectory(test)
endif()

add_executable(ComputeNDVI main.cpp)
target_link_libraries(ComputeNDVI ${OTB_LIBRARIES})

add_executable(DirectionalCorrection main.cpp)
target_link_libraries(DirectionalCorrection ${OTB_LIBRARIES})

add_executable(UpdateSynthesis main.cpp)
target_link_libraries(UpdateSynthesis ${OTB_LIBRARIES})
