#!/bin/bash

if [ $# -ne 2 ]
then
	echo "Syntax: $0 input.v output.cpp"
	exit
fi

echo "Executing Yosys..."
yosys -p "read_verilog $1; proc; opt; memory; opt; fsm; opt; techmap; opt; abc -liberty tfhe.lib; opt; write_verilog output.v"
echo "Stripping comments..."
grep -Fv '(*' output.v > stripped.v
echo "Generating C++ code..."
node index.js stripped.v > $2
rm -v output.v stripped.v
echo "Done."
