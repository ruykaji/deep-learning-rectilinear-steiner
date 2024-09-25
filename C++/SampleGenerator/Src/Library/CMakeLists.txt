# Ini library
add_library(Ini Ini.cpp)

# Matrix library
add_library(Matrix Matrix.cpp)

# Utils library
add_library(Utils Utils.cpp)

# Generator library
add_library(Generator Generator.cpp)
target_link_libraries(Generator PUBLIC Utils)

# Graph library
add_library(Graph Graph.cpp)