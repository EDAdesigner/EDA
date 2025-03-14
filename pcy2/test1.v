module carry(a,b,c,cout);
input a,b,c;
output cout;
wire x; 
assign x = a & b; 
assign cout = x | c; 
endmodule
