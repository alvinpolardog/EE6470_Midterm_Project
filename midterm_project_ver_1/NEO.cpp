#include <cmath>
#include <cstdlib>

#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "NEO.h"

NEO::NEO( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK(input_buffer);
#endif

	SC_THREAD( neo_calculate );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_value.clk_rst(i_clk, i_rst);
    o_result.clk_rst(i_clk, i_rst);
#endif
}

NEO::~NEO() {}


void NEO::neo_calculate() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_value.reset();
		o_result.reset();
#endif
		wait();
	}

    sc_dt::sc_uint<4> current_buffer_idx = 0;
    sc_dt::sc_uint<4> current_neo_idx = 0;
    sc_dt::sc_uint<4> next = 0;
    sc_dt::sc_uint<4> prev = 0;
    input_buffer[current_buffer_idx++] = i_value.get();
    wait();
    input_buffer[current_buffer_idx++] = i_value.get();
    wait();


	while (true) {
		input_type value;

		for (int i = 0; i < 1999; i++){
#ifndef NATIVE_SYSTEMC
            HLS_UNROLL_LOOP ( CONSERVATIVE, 4, "NEO_CALC" ); 
#endif
#ifndef NATIVE_SYSTEMC
			{
				HLS_DEFINE_PROTOCOL("input");
                HLS_CONSTRAIN_LATENCY(0,2,"neo_input");
                current_neo_idx = ((current_buffer_idx == 0)? BUFFER_SIZE -1 : (current_buffer_idx - 1) );
				input_buffer[current_buffer_idx++] = i_value.get();
                if (current_buffer_idx == BUFFER_SIZE)
                    current_buffer_idx = 0;
				wait();
			}
#else
                current_neo_idx = ((current_buffer_idx == 0)? BUFFER_SIZE -1 : (current_buffer_idx - 1) );
				input_buffer[current_buffer_idx++] = i_value.get();
                if (current_buffer_idx == BUFFER_SIZE)
                    current_buffer_idx = 0;
#endif
            output_type neo_value;

                next = ((current_neo_idx == (BUFFER_SIZE - 1))?  0 : (current_neo_idx + 1));
                prev = ((current_neo_idx == 0)? BUFFER_SIZE -1 : (current_neo_idx - 1) );
            {
                HLS_CONSTRAIN_LATENCY(0,2,"neo_calculation");
                neo_value = input_buffer[current_neo_idx] * input_buffer[current_neo_idx] - input_buffer[next] * input_buffer[prev];
                // cout<<i<<' '<< "prev "<<prev<<' '<<input_buffer[prev]<<"; cur "<< current_neo_idx<< ' '<< input_buffer[current_neo_idx]<<"; next "<< next<< ' '<< input_buffer[next]<<"; neo_value "<<neo_value<<endl;
            }

#ifndef NATIVE_SYSTEMC
			{
				HLS_DEFINE_PROTOCOL("output");
				o_result.put(neo_value);
				wait();
			}
#else
			o_result.write(neo_value);
#endif
		}
	}
}
