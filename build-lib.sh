cmake -DNO_XCSP3=ON -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
#cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" .
cmake --build . --target libcosoco -- -j 8
