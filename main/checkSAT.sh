#!/bin/csh

./cosoco -model  $argv |grep -e "^v"| sed -e 's/^v\(.*\)/\1/'| java -cp ~/csp/XCSP3-Java-Tools/target/xcsp3-solutionChecker-2.1.0.jar org.xcsp.parser.callbacks.SolutionChecker $argv[1]
