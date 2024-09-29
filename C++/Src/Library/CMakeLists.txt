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

# Transform library
add_library(Transform Transform.cpp)
target_link_libraries(Transform PUBLIC Graph Matrix)

# Algorithms library
add_library(Algorithms Algorithms.cpp)
target_link_libraries(Algorithms PUBLIC Graph)