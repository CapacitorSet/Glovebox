verilog-to-tfhe
===============

This directory contains a toolchain for transforming Verilog code into TFHE instructions. In particular:

 * `tfhe.lib` is a Yosys library containing the logic gates implemented by TFHE;
 * `verilog-grammar.peg` is a [PEG.js](https://pegjs.org/) grammar for a subset of Verilog;
 * `parser.js` is the above grammar, compiled into a parser;
 * `index.ts` is a script that parses the output of Yosys and generates a C++ implementation.

This tool is very much **experimental**.

## How it works

Rough summary:

 * Yosys does logic synthesis, i.e. it translates algorithms written in Verilog into a list of wires and logic gates, also expressed in Verilog.
 * `parser.js` translates the Verilog implementation into a JS object suitable for processing.
 * `index.ts` reorders the list of gates so that it is logically coherent (wires must be written to before they're used as inputs) and mildly optimized (wires are reused when possible), and translates it into C++ code for Glovebox.

## Requirements

 * [Yosys](https://github.com/YosysHQ/yosys)
 * [Node.js](https://nodejs.org/en/) 6.0 or higher
 * Yarn, Typescript (`npm install --global yarn typescript`)

## Installation

 * Install the Node.js dependencies: `yarn install --production`
 * Compile: `tsc`

## Automated usage

```sh
./transpile.sh input.v output.cpp
```

## Manual usage

 * Write a Verilog file, eg. `adder.v` containing the module `add8`;
 * Run the following Yosys commands:

  ```
read_verilog adder.v
hierarchy -check -top add8
proc; opt; memory; opt; fsm; opt; techmap; opt
abc -liberty tfhe.lib
write_verilog output.v
  ```

 * Strip comments:

  ```sh
grep -Fv '(*' output.v > stripped.v
  ```

 * Transform to C++:

  ```sh
node index.js stripped.v > output.cpp
  ```