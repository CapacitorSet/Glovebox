module incr8_if(out, src, cond);
	localparam SIZE = 8;

	input signed [(SIZE-1):0] src;
	input cond;
	output signed [(SIZE-1):0] out;

	assign out = src + cond;
endmodule

module incr16_if(out, src, cond);
	localparam SIZE = 16;

	input signed [(SIZE-1):0] src;
	input cond;
	output signed [(SIZE-1):0] out;

	assign out = src + cond;
endmodule