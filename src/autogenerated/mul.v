module mul8(overflow, out, a, b);
	localparam SIZE = 8;

	input  signed [(SIZE-1):0]   a;
	input  signed [(SIZE-1):0]   b;
	output signed [(2*SIZE-1):0] out;
	output overflow;

	assign out = a * b;
	// Overflow = the result doesn't fit in the SIZE/2 LSB
	assign overflow = out[(2*SIZE-1)] ? ~&out[(2*SIZE-1):SIZE] : |out[(2*SIZE-1):SIZE];
endmodule

module mul16(overflow, out, a, b);
	localparam SIZE = 16;

	input  signed [(SIZE-1):0]   a;
	input  signed [(SIZE-1):0]   b;
	output signed [(2*SIZE-1):0] out;
	output overflow;

	assign out = a * b;
	// Overflow = the result doesn't fit in the SIZE/2 LSB
	assign overflow = out[(2*SIZE-1)] ? ~&out[(2*SIZE-1):SIZE] : |out[(2*SIZE-1):SIZE];
endmodule