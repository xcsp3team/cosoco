rm -rf bin/cosoco
cd ../XCSP3-CPP-Parser
cmake --build . --target clean -- -j 8
rm -rf lib/libxcsp3parser.a
cd ../cosoco/
cmake --build . --target clean -- -j 8
rm -rf CMakeCache.txt  CMakeFiles cmake_install.cmake Makefile
