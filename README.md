# mymake

This project implements the core functionality of the Make utility. This program reads in a file specifying dependencies and rules as in a standard Makefile. Then, a dependency graph is built from this file. As the dependency graph is traversed, searching for the target file specified,  it checks for file existence and the last-modified timestamp associated with files, and executes the command associated with a target when a target needs to be rebuilt. All memory that is allocated is freed up when the program finishes executing.

# Directions

- In bash, cd into the project directory
- enter 'make' to create the executable, mymake2.
- run mymake2 with 'mymake2 -f aMakeFile aTarget'
- '-f aMakeFile' specifies which file should be used to build the dependency graph. If no file is specified, 'myMakefile' will be used.
- 'aTarget' is the target appearing in the makefile. If none is specified, the first target defined will be set as the target.
- The commands will  run which correlate with the target that needs to be rebuilt.
