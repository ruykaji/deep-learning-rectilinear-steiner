#!/bin/bash

# Set up directories
SOURCE_DIR=$(pwd)
RESULT_DIR="$SOURCE_DIR/Result"
CPP_DIR="$SOURCE_DIR/C++"
PYTHON_DIR="$SOURCE_DIR/Python"

# Remove the result directory if it exists
[ -d "$RESULT_DIR" ] && rm -r "$RESULT_DIR"

# Create build directory for C++ programs if it doesn't exist
BUILD_DIR="$CPP_DIR/build"
mkdir -p "$BUILD_DIR"

# Build C++ programs
cd "$BUILD_DIR" || exit
cmake -DCMAKE_BUILD_TYPE=Release -DEnableTests=OFF ..
cmake --build . --target all

# Return to source directory
cd "$SOURCE_DIR" || exit

# Create result directory structure
mkdir -p "$RESULT_DIR/Program"

# Move generated binary to result directory
mv "$CPP_DIR/Output/Release/bin/SampleGenerator" "$RESULT_DIR/Program"

# Generate sample config file
cat <<EOL > "$RESULT_DIR/Program/sample_generator_config.ini"
[Path]

Output = $RESULT_DIR/Assets

[Generation]

Size = 32
Depth = 1
MaxNumberOfPoints = 5
DesiredCombinations = 10000
EOL