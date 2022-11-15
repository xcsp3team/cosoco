#!/bin/csh

./cosoco -model  $argv |grep -e "^v"| sed -e 's/^v\(.*\)/\1/'| java -cp ~/csp/XCSP3-Java-Tools/build/libs/xcsp3-tools-1.2.0.jar org.xcsp.parser.callbacks.SolutionChecker $argv[1]
