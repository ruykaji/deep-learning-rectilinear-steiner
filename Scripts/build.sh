SOURCE_DIR=$(pwd)
RESULT_DIR="$SOURCE_DIR/Result"
CPP_DIR="$SOURCE_DIR/C++"
PYTHON_DIR="$SOURCE_DIR/Python"

if [ -d $RESULT_DIR ]; then
   rm -r $RESULT_DIR
fi

# Building C++ programs
cd $CPP_DIR

if [ ! -d build ]; then
   mkdir build
fi

cd build
cmake -DCMAKE_BUILD_TYPE=Release -DEnableTests=OFF ..
cmake --build . --target all .

cd $SOURCE_DIR

# Building python libraries

# Make build directory
mkdir -p $RESULT_DIR

mkdir $RESULT_DIR/Program
mv $CPP_DIR/Output/Release/bin/SampleGenerator $RESULT_DIR/Program
echo "[Path]\n\nOutput = $RESULT_DIR/Assets\n\n[Generation]\n\nSize = 32\nDepth = 1\nMaxNumberOfPoints = 5\nDesiredCombinations = 10000" > $RESULT_DIR/Program/sample_generator_config.ini
