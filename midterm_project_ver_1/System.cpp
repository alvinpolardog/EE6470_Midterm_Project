#include "System.h"
System::System( sc_module_name n, string input_file, string output_file ): sc_module( n ), 
	tb("tb", output_file), neo("neo"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	neo.i_clk(clk);
	neo.i_rst(rst);
	tb.o_value(rgb);
	tb.i_result(result);
	neo.i_value(rgb);
	neo.o_result(result);

 	tb.open_input_file(input_file);
}

System::~System() {}
