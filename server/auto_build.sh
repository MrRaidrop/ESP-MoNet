rm -rf build/
mkdir build 
cp key.pem cert.pem firmware.bin firmware.sha256 ./build
cd build
cmake ..
make -j
./https_server