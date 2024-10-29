cd ../XCSP3-CPP-Parser
./build.sh
cd ../cosoco/
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="`pwd`/main" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="`pwd`/lib" -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="`pwd`/lib" -G "Unix Makefiles" .
#cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" .
cmake --build . --target cosoco -- -j 8
