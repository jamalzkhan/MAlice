# MAlice Compiler

# About
- This is a compiler written for a ficticious language called MAlice.
- It was part of a lab exercise for 2nd year Computing students at Imperial College London www.doc.ic.ac.uk
- Our implementation was done in C, and the compiler produces output in Intel x86 assembly.

- In the Compiler_Specification pdf there is the BNF of the compiler and what inputs it takes.
- The Compiler_Evaluation pdf contains how we built the compiler

# Usage
- There are examples of the language in the examples directory and these can be compiled.
- To use the compiler follow these steps: (assuming you are using a *nix environment)
    - Make the project in the src folder
    - Run the command $ ./compile input_file.alice
    - In the same directory as where the input file resides  input_file.o, input_file.s, input_file (executable) are created
    - The input_file.s is the assembly file
    - Run ./_input_file and use the $ echo $? command to get the result of the program which was compiled and run
