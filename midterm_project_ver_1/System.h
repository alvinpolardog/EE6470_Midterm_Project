#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>
using namespace sc_core;

#include "define.h"
#include "Testbench.h"
#ifndef NATIVE_SYSTEMC
#include "NEO_wrap.h"
#else
#include "NEO.h"
#endif

class System: public sc_module
{
public:
	SC_HAS_PROCESS( System );
	System( sc_module_name n, std::string input_file, std::string output_file );
	~System();
private:
  Testbench tb;
#ifndef NATIVE_SYSTEMC
	NEO_wrapper neo;
#else
	NEO neo;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< input_type > rgb;
	cynw_p2p< output_type > result;
#else
	sc_fifo< input_type > rgb;
	sc_fifo< output_type > result;
#endif

	std::string _output_file;
};
#endif
