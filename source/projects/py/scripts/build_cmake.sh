rm -rf ./build ./externals

pushd .

mkdir build
cd build
cmake ..
make

popd

