#!/bin/bash
# Usage: ./build.sh [--static]
#   --static  produce a portable binary (see STATIC_BUILD in CMakeLists.txt)
set -e

STATIC_BUILD=OFF
case "$1" in
    --static) STATIC_BUILD=ON ;;
    "")       ;;
    *)        echo "unknown option: $1" >&2; echo "usage: $0 [--static]" >&2; exit 1 ;;
esac

cd ../XCSP3-CPP-Parser
# Its own build.sh configures a Debug build: no optimisation at all. The
# parser reads every instance cosoco is given, so it is on the critical path
# of every run -- a 5 MB instance takes 0.68 s to parse that way, and 0.26 s
# built like this. Configure the tree here rather than calling that script.
# No -G: cmake refuses to change the generator of a tree already configured,
# and the default is the one we want anyway.
cmake -DCMAKE_BUILD_TYPE=Release .
cmake --build . --target all -- -j 8
cd ../pfactory
./bootstrap
make
cd ../cosoco/
cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_BUILD=${STATIC_BUILD} -G "Unix Makefiles" .
#cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" .
cmake --build . --target cosoco -- -j 8
