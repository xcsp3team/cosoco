cd ../XCSP3-CPP-Parser
./build.sh
cd ../cosoco/
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
#cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" .
cmake --build . --target cosoco -- -j 8
