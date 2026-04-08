Graph Layout Engine (C Edition)
Description

This tool is designed for the automated 2D positioning of graph vertices. It implements two distinct approaches: the mathematical Tutte barycentric method and the Fruchterman-Reingold force-directed physical simulation. The engine ensures the planarity of the final layout by actively detecting and eliminating edge crossings.
Compilation

To build the program, use the GCC compiler with the math library linked:

gcc -Wall -Wextra -std=c11 main.c layout_tutte.c fruchterman.c io.c -o program -lm

Usage and Syntax

./program -i <input.txt> -o <output> -a <tutte|fruchterman> [-f <txt|bin>]

CLI Parameters:

    -i: (Required) Path to the input file.

    -o: (Required) Path to the output file.

    -a: (Required) Choice of algorithm: tutte or fruchterman.

    -f: (Optional) Export format: txt (default) or bin.

    -h: Displays detailed instructions and input formatting rules.

Input File Formatting Rules

The validation module (io.c) requires strict adherence to the following data format:

    Line Structure: Edge_Name;Node_U;Node_V;Weight (supported separators: ; or ,).

    Edge Name:

        Maximum 10 characters.

        Allowed: lowercase a-z and digits 0-9.

        Prohibited: spaces, uppercase letters, and diacritics (e.g., Polish ą, ę, German umlauts, etc.).

        Missing names or special characters will trigger a format error.

    Vertex Identifiers (U, V):

        Positive integers in the range 1 - 1000.

        Self-loops (U=V) are strictly prohibited.

    Edge Weight:

        Decimal number up to 100.0.

        The program automatically corrects the decimal separator (replaces , with .).

    File Structure:

        Empty lines are skipped.

        Lines starting with # are not ignored and are treated as format errors.

Diagnostics and Exit Codes

The program returns the following exit codes upon failure to help with automated testing and debugging:
Code	Error Name	Cause
1	Argument Error	Missing -i or -o flags, or invalid output format specified.
2	Read Error	Memory allocation failure or unable to open the file.
3	Path Error	Input file does not exist or the path is incorrect.
4	Algorithm Error	Specified algorithm name is not tutte or fruchterman.
5	Empty File	Input file contains no valid edges or only whitespace.
6	Size Error	Graph has fewer than 3 nodes or fewer than 2 edges.
8	Planarity Error	Resulting layout contains crossings (Tutte) or FR failed after 3000 attempts.
9	Connectivity Error	Disconnected graph components detected (failed DFS connectivity test).
10 / 12	Structure Error	Invalid data format (e.g., names too long, self-loops, invalid characters).


Modular Architecture

    graph.h: Core data structures for Node, Edge, and Graph.

    io.c / io.h: Data parsing, syntax validation, and DFS connectivity analysis.

    layout_tutte.c: Deterministic barycentric layout implementation.

    fruchterman.c: Force-directed physical simulation and real-time CCW edge crossing detection.

    main.c: CLI orchestration, validation flow, and "Retry Logic" loop.