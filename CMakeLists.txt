PROJECT(tmra)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Project: TMRA")
MESSAGE(STATUS "Project Directory: ${PROJECT_SOURCE_DIR}")

add_subdirectory(test)
add_subdirectory(rtsp)
add_subdirectory(miniboost)

