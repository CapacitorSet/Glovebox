module add8(carry, out, a, b);
   input  [7:0] a;
   input  [7:0] b;
   output [7:0] out;
   output       carry;
   assign {carry, out} = a + b;
endmodule

module add16(carry, out, a, b);
   input  [15:0] a;
   input  [15:0] b;
   output [15:0] out;
   output       carry;
   assign {carry, out} = a + b;
endmodule