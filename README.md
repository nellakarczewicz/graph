Graph Layout Engine (C Edition)

A command-line utility for the automated 2D positioning of graph vertices using barycentric methods and force-directed physical models.


Compilation

The project can be compiled in two ways:
1. Using Makefile (Recommended)
//Bash
make clean
make

2. Using GCC Compiler Directly

If the Makefile is unavailable, use the following command (the math library -lm is required):
//Bash

gcc -Wall -Wextra -std=c11 main.c layout_tutte.c fruchterman.c io.c -o program -lm




Usage and Syntax
The program is controlled via command-line arguments. Use the -h flag to display help.
Execution Syntax:
//Bash

./program -i <input.txt> -o <output> -a <algorithm> [-f <format>]

Parameter Description:

    -i [path]: Path to the input file containing the edge list.

    -o [path]: Path for the output file.

    -a [tutte | fruchterman]: Choice of layout algorithm.

    -f [txt | bin]: Export data format (defaults to txt).

    -h: Displays the user manual and input file formatting rules.




Input File Formatting Rules

The input module (io.c) performs strict data validation. The file must adhere to the following technical criteria:

    Line Structure:

    Edge_Name;Node_U;Node_V;Weight

    Field Requirements:
        Edge Name:
            Maximum length: 10 characters.
            Allowed characters: Lowercase Latin letters (a-z) and digits (0-9) only.
            Prohibited: Spaces, uppercase letters, special characters, and diacritics.
        Vertex Identifiers (Node_U, Node_V):
            Type: Positive integers in the range 1 - 1000.
            Prohibited: Self-loops (Node_U must be different from Node_V).
        Edge Weight:
            Format: Decimal number (e.g., 1.50).
            The program performs automatic decimal separator correction (comma to dot conversion).
        Separators and Special Characters:
            Accepted column separators: Semicolon (;) or comma (,).
            Lines starting with # and empty lines are ignored.

Diagnostics and Error Codes

In case of structural or mathematical errors, the program terminates and returns a specific exit code:
Code	Meaning	                    Cause
6	    Size Error	                Insufficient number of vertices (< 3) or edges (< 2).
7	    Non-planar	                Euler's formula violation (E>3V−6) – graph is too dense.
8	    Optimization Failure	    Fruchterman-Reingold algorithm failed after 3000 attempts.
9	    Non-connected	            Disconnected graph components detected via DFS.
10	    Format Error	            Input data inconsistent with specification (e.g., letters in IDs).


Modular Architecture

    graph.h: Definitions for Node, Edge, and Graph structures.
    io.c / io.h: Data parser, syntax validation, and DFS algorithm.
    layout_tutte.c: Implementation of the deterministic barycentric method.
    fruchterman.c: Implementation of the force-directed model with CCW intersection testing.
    main.c: Coordination of data flow, CLI handling, and "Retry Logic" for stochastic processes.