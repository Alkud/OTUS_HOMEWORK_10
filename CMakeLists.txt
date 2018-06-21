#CmakeList.txt for OTUS Homework #10

#Check CMake version
cmake_minimum_required(VERSION 3.2)

#Set project name, set PROJECT_VERSION_MINOR value equal to TRAVIS_BUILD NUMBER
if (DEFINED ENV{TRAVIS_BUILD_NUMBER})
  set (VERSION_PATCH $ENV{TRAVIS_BUILD_NUMBER})
else ()
  set (VERSION_PATCH 1)
endif()

set(CMAKE_CXX_STANDARD 14)

project(bulkmt VERSION 0.0.${VERSION_PATCH})

#add library subdirectory
add_subdirectory(command_processor)

set (BOOST_MIN_VERSION "1.54.0")

#Find Boost UTF component, get BOOST_LIBRARIES and BOOST_INCLUDE_DIR values
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(BOOST_DEBUG                ON)
set(Boost_USE_MULTITHREADED    ON)


set(homework_10
    ${PROJECT_SOURCE_DIR}/homework_10.h
    ${PROJECT_SOURCE_DIR}/homework_10.cpp)

#Set executable target
add_executable(bulkmt bulkmt.cpp ${homework_10})

#Set test target_link_libraries
add_executable(otus_hw_10_test otus_hw_10_test.cpp ${homework_10})

#Set performance test target_link_libraries
add_executable(bulkmt_perf_test bulkmt_perf_test.cpp)

#Enable tests
enable_testing()
add_test(NAME test_hw_10
         COMMAND otus_hw_10_test)

#Set properties and compile options for executable target
set_target_properties(bulkmt
                      PROPERTIES CXX_STANDARD 14
                      CXX_STANDARD_REQUIRED ON
                      COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

#Set properties for test target
set_target_properties(otus_hw_10_test PROPERTIES
                      COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
                      INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

#Link
target_link_libraries(bulkmt pthread command_processor)

target_link_libraries(bulkmt_perf_test pthread command_processor)

target_link_libraries(otus_hw_10_test pthread command_processor)

target_link_libraries(otus_hw_10_test
                      ${Boost_LIBRARIES})

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT alexandr_kudinov@mail.ru)

include(CPack)
