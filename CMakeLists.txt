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
#set(CMAKE_CXX_FLAGS -pthread)

project(bulkmt VERSION 0.0.${VERSION_PATCH})

set (BOOST_MIN_VERSION "1.54.0")

#Find Boost UTF component, get BOOST_LIBRARIES and BOOST_INCLUDE_DIR values
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(BOOST_DEBUG                 ON)
set(Boost_USE_MULTITHREADED    ON)


#Set library target
add_library(homework_10 ./library/homework_10.cpp)
add_library(input_processor ./library/input_processor.cpp)
add_library(input_reader ./library/input_reader.cpp)
add_library(publisher_mt ./library/publisher_mt.cpp)

add_library(command_processor_mt INTERFACE)
target_sources(command_processor_mt INTERFACE ${PROJECT_SOURCE_DIR}/library/command_processor_mt.h)
target_include_directories(command_processor_mt INTERFACE ${PROJECT_SOURCE_DIR}/library)

add_library(broadcasters INTERFACE)
target_sources(broadcasters INTERFACE ${PROJECT_SOURCE_DIR}/library/broadcasters.h)
target_include_directories(broadcasters INTERFACE ${PROJECT_SOURCE_DIR}/library)

add_library(listeners INTERFACE)
target_sources(listeners INTERFACE ${PROJECT_SOURCE_DIR}/library/listeners.h)
target_include_directories(listeners INTERFACE ${PROJECT_SOURCE_DIR}/library)

add_library(logger_mt INTERFACE)
target_sources(logger_mt INTERFACE ${PROJECT_SOURCE_DIR}/library/logger_mt.h)
target_include_directories(logger_mt INTERFACE ${PROJECT_SOURCE_DIR}/library)

add_library(smart_buffer_mt INTERFACE)
target_sources(smart_buffer_mt INTERFACE ${PROJECT_SOURCE_DIR}/library/smart_buffer_mt.h)
target_include_directories(smart_buffer_mt INTERFACE ${PROJECT_SOURCE_DIR}/library)


#Set executable target
add_executable(bulkmt bulkmt.cpp)

#Set test target_link_libraries
add_executable(otus_hw_10_test otus_hw_10_test.cpp)

#Enable tests
enable_testing()
add_test(
NAME test_hw_10
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
#target_link_libraries(${PROJECT_NAME} pthread)

target_link_libraries(bulkmt homework_10)

target_link_libraries(homework_10 command_processor_mt)

target_link_libraries(command_processor_mt INTERFACE
                      input_processor
                      input_reader
                      logger_mt
                      publisher_mt)

target_link_libraries(input_reader
                      broadcasters
                      smart_buffer_mt)

target_link_libraries(input_processor
                      listeners
                      smart_buffer_mt)

target_link_libraries(logger_mt INTERFACE
                      broadcasters
                      listeners
                      smart_buffer_mt
                      pthread)

target_link_libraries(publisher_mt
                      broadcasters listeners
                      smart_buffer_mt
                      pthread)

target_link_libraries(smart_buffer_mt INTERFACE
                      broadcasters listeners)

target_link_libraries(otus_hw_10_test
                      ${Boost_LIBRARIES}
					  homework_10
                      command_processor_mt)

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT alexandr_kudinov@mail.ru)

include(CPack)
