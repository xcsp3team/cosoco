rm -rf main/cosoco
cd ../XCSP3-CPP-Parser
cmake --build . --target clean -- -j 8
rm -rf lib/libxcsp3parser.a
cd ../pfactory
make clean
cd ../cosoco/
cmake --build . --target clean -- -j 8
rm -rf CMakeCache.txt  CMakeFiles cmake_install.cmake Makefile
rm -rf cmake-build-debug

