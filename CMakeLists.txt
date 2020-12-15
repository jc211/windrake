cmake_minimum_required(VERSION 3.17)
project(drake LANGUAGES CXX)
set(DRAKE_VERSION_MAJOR 0)
set(DRAKE_VERSION_MINOR 24)
set(DRAKE_VERSION ${DRAKE_VERSION_MAJOR}.${DRAKE_VERSION_MINOR})

find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)




include(./drake/common/CMakeLists.txt)
include(./drake/math/CMakeLists.txt)

add_library(drake SHARED ${COMMON_SRC} ${MATH_SRC})
target_include_directories(drake 
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:"include/drake">
)

target_link_libraries(drake PRIVATE Eigen3::Eigen fmt::fmt spdlog::spdlog gflags_shared)
target_compile_options(drake PRIVATE  $<$<COMPILE_LANGUAGE:CXX>: "/Zc:__cplusplus" "/bigobj" > $<$<COMPILE_LANGUAGE:CUDA>: -v>)
target_compile_definitions(drake PRIVATE _USE_MATH_DEFINES _ENABLE_EXTENDED_ALIGNED_STORAGE) # This loads M_PI on MSVC from cmath.h, the other define is to deal with aligned storage used in never_destroyed.h
set_property(TARGET drake PROPERTY CXX_STANDARD 17)
add_library(drake::drake ALIAS drake)



install(TARGETS drake EXPORT DrakeTargets LIBRARY)
install(EXPORT DrakeTargets FILE DrakeTargets.cmake DESTINATION share)
install(DIRECTORY drake/ DESTINATION include/drake/drake
        FILES_MATCHING PATTERN "*.h")



include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${DRAKE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${DRAKE_VERSION_MINOR}")
include(CPack)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DrakeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/DrakeConfig.cmake"
  INSTALL_DESTINATION "share"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/DrakeConfigVersion.cmake"
  VERSION ${DRAKE_VERSION}
  COMPATIBILITY AnyNewerVersion
)
# install the configuration file
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/DrakeConfig.cmake
  DESTINATION share
  )