#ifndef _NEO_H_
#define _NEO_H_

#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

#include "define.h"
#define BUFFER_SIZE 10

class NEO: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in <bool>  i_rst;


#ifndef NATIVE_SYSTEMC
	cynw_p2p<input_type>::in i_value;
	cynw_p2p<output_type>::out o_result;
#else
	sc_fifo_in<input_type> i_value;
	sc_fifo_out<output_type> o_result;
#endif

	SC_HAS_PROCESS( NEO );
	NEO( sc_module_name n );
	~NEO();


private:
	void neo_calculate();
	input_type input_buffer[BUFFER_SIZE];
};

#endif
