#!/bin/csh

./cosoco $argv -model=2 |grep -e "^v"| sed -e 's/^v\(.*\)/\1/'| java -cp ~/csp/XCSP3-Java-Tools/target/xcsp3-solutionChecker-2.6.0.jar org.xcsp.parser.callbacks.SolutionChecker $argv[1]
