module mul8(out, a, b);
   input  [ 7:0] a;
   input  [ 7:0] b;
   output [15:0] out;
   assign out = a * b;
endmodule

module mul16(out, a, b);
   input  [15:0] a;
   input  [15:0] b;
   output [31:0] out;
   assign out = a * b;
endmodule