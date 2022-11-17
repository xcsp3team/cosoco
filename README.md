  ## COSOCO: A COmpact SOlver for COnstraint problems
 
### Authors
 Gilles Audemard (audemard@cril.fr)

This work would not have taken place without [Christophe Lecoutre](https://github.com/lecoutre). 
I  would like to thank him very warmly for his support. Indeed, this is the result of many discussions 
and most of the contraint propagators come from [ACE](https://github.com/xcsp3team/ace)

### Introduction
 Written in C++, cosoco is a small (less than 12,000 lines of code) constraint solver that deals with 
 almost all constraints that forms the [XCSP3 core](http://xcsp.org):
 
  + The main data structures (related to CSP instances) contain around 600 loc.
  + The solver part contains around 1500 loc.
  + The optimizer part around 250 loc.
  + The constraint directory is the biggest one with around 5,500 loc.
 
 As introduced above, cosoco uses the [XCSP3 core](XCSP3 format) and parses  problems using the parser 
 available [https://github.com/xcsp3team/XCSP3-CPP-Parser](here)
 
 A class diagram is provided with the source code (see diagram.puml)
 
 
 
### Supported constraints
 This solver supports (you can have examples [http://xcsp.org/specifications](here))
 
  + generic constraints: instantiation, extension and intension
  + langage constraints: MDD, regular, circuit
  + comparison constraints: alldiff (var, lex and matrix), allequal, ordered, lex, lex matrix
  + counting/summing constraints: count, sum, nValues, cardinality
  + connection constraints: element, minimum, maximum, channel
  + scheduling constraints: noOverlap, cumulative
 
### Supported problems
 Cosoco supports both satisfaction and optimization problems and take XCSP3 instances as input.
 
### Installation
  Requirements:
  
  + C++14 
  + CMake (>= 3.3)
  + LibXML2 (for parsing problems)
  + [https://github.com/xcsp3team/XCSP3-CPP-Parser](XCSP3 C++ parser). Put in a directory at the same level than cosoco
  
  That's all.
  
  Compilation: ./build.sh

### Notations used in the code
+ Domain:
   + value : v
   + index : idv


+ Variables: 
   + x,y,z,t,x1
   + index: idx
   

+ Constraints : 
   + c,c1,c2,c3 : constraints
   + idc: index  
   + scope

### Directories
  + constraints: all constraints implemented. This directory contains subdirectories, named follow the name of constraints in XCSP specifications.
  + core: the classes Domain, Problem, Variables  
  + main: contains the XCSP3 Parser callback and the main file
  + mtl: data structures, most of them come from minisat (thanks again to minisat team)
  + optimizer: the optimizer solver
  + solver: the solver, with observer, heuristics, restarts...
  + utils: options, system, verbose... Some code comes from minisat...



### How to implement a constraint
+ Create the constraint, there are special inheritences if the constraint is binary, global...
+ Override these functions:
     + `virtual bool isCorrectlyDefined()`: check if the constraint is ok. It is a safeguard.
     + `virtual void delayedConstruction(int id)`: perhaps you need to initialize some data structures.. It is called when all the problem is parsed (end tag in XCSP3 format)
     + `virtual State status()`: Perhaps, you know that the constraint is always CONSISTENT/INCONSISTENT. In that case,
      overide this method en return the good status. (See Unary.cc for example)
     + `virtual void reinitialize`: related to the previous function. In case, of full backtrack, one needs to reinitialize the constraint.
     + `virtual bool isSatisfiedBy()`: given a tuple (associated to the scope) return true if the constraint is satisfied by this tuple.
     + `virtual bool filter(Variable *x)`: the most important one. The domain of x changed during this propagation step. One need to filter the constraint.
      
### How to implement heuristics, restarts
+ You just need to inherit the related main classes, that is, `HeuristicVal` (for values), 
`HeuristicVar` (for variables) or `Restart` (for restart). All these classes are located in the directory solver.

### Observers
+ There are several observers. You can attach a component to one of them, in order to be warned
if one of the event occurs.
