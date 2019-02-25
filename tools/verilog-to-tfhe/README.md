verilog-to-tfhe
===============

This directory contains several files for transforming Verilog code into TFHE instructions. In particular:

 * `tfhe.lib` is a Yosys library containing the logic gates implemented by TFHE;
 * `verilog-grammar.peg` is a [PEG.js](https://pegjs.org/) grammar for a subset of Verilog;
 * `parser.js` is the above grammar, compiled into a parser;
 * `index.js` is a script that parses the output of Yosys and generates a C++ implementation.

This tool is very much **experimental**.

## Requirements

 * [Yosys](https://github.com/YosysHQ/yosys)
 * [Node.js](https://nodejs.org/en/) 6.0 or higher
 * Yarn, Typescript (`npm install --global yarn typescript`)

## Usage

 * Install the Node.js dependencies: `yarn install --production`
 * Compile: `tsc`
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

  ```
grep -Fv '(*' output.v > stripped.v
  ```

 * Transform to C++:

  ```
node index.js stripped.v > output.cpp
  ```