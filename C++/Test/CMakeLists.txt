find_package(GTest CONFIG REQUIRED)
   if(NOT GTest_FOUND)
   message(FATAL_ERROR "Google Test not found!")
endif()

enable_testing()

include(GoogleTest)

add_executable(IniTest ini.test.cpp)
target_link_libraries(IniTest Ini GTest::gtest_main pthread)
gtest_discover_tests(IniTest)

add_executable(MatrixTest matrix.test.cpp)
target_link_libraries(MatrixTest Matrix GTest::gtest_main pthread)
gtest_discover_tests(MatrixTest)

add_executable(GeneoratorTest generator.test.cpp)
target_link_libraries(GeneoratorTest Generator GTest::gtest_main pthread)
gtest_discover_tests(GeneoratorTest)

add_executable(NumpyTest numpy.test.cpp)
target_link_libraries(NumpyTest GTest::gtest_main pthread)
gtest_discover_tests(NumpyTest)