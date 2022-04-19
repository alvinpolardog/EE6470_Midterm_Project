#include <cmath>
#include <cstdlib>

#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "NEO.h"

// NEO Module Constructor:
// create thread and set up interfaces
NEO::NEO( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	// HLS_MAP_TO_REG_BANK(input_buffer);
	HLS_FLATTEN_ARRAY(input_buffer);
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

// Main thread of NEO Module
// Read from interface, solve for NEO, and return results
void NEO::neo_calculate() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_value.reset();
		o_result.reset();
#endif
		wait();
	}

	// Set up NEO buffer
	input_array = i_value.get();
	wait();

	current_buffer_idx = 0;
    input_buffer[current_buffer_idx++] = input_array.range(INPUT_WIDTH-1,0);
    input_buffer[current_buffer_idx++] = input_array.range(2*INPUT_WIDTH-1,INPUT_WIDTH);

	// Main Loop
	while (true) {
#ifndef NATIVE_SYSTEMC
        	HLS_PIPELINE_LOOP(SOFT_STALL, 1, "Loop" );
#endif	
			
			{
#ifndef NATIVE_SYSTEMC
				// Read Values from input interface
				HLS_DEFINE_PROTOCOL("input");
				input_array = i_value.get();
				wait();
#else		
				input_array = i_value.read();
#endif			
				// Loop over and decompose the input, transfer to internal buffer
				current_neo_idx = ((current_buffer_idx == 0)? BUFFER_SIZE -1 : (current_buffer_idx - 1) );
				for(int i = 1; i <= CHANNEL_WIDTH; i++){
#ifndef NATIVE_SYSTEMC
					 HLS_UNROLL_LOOP ( CONSERVATIVE, CHANNEL_WIDTH, "NEO_READ" ); 
					// HLS_CONSTRAIN_LATENCY(0,2,"neo_calculation");
#endif
					int start = i*INPUT_WIDTH-1;
					int end = (i-1)*INPUT_WIDTH;
					input_buffer[current_buffer_idx] = input_array.range(start,end);
					// wait();
                	if (++current_buffer_idx == BUFFER_SIZE)
                    	current_buffer_idx = 0;
				}

			}

			// Loop over internal buffer, and solve for NEO for each new values, storing it inside the result(output_array)
			for(int i = 1; i <= CHANNEL_WIDTH; i++){		
#ifndef NATIVE_SYSTEMC
				HLS_UNROLL_LOOP ( CONSERVATIVE, CHANNEL_WIDTH, "NEO_CALC" ); 
				//HLS_CONSTRAIN_LATENCY(0,3,"neo_calculation");

            	//HLS_PIPELINE_LOOP(SOFT_STALL, 1, "Loop" );
#endif
                
				next = ((current_neo_idx == (BUFFER_SIZE - 1))?  0 : (current_neo_idx + 1));
				prev = ((current_neo_idx == 0)? BUFFER_SIZE -1 : (current_neo_idx - 1) );
				neo_value = input_buffer[current_neo_idx] * input_buffer[current_neo_idx] - input_buffer[next] * input_buffer[prev];
			
				output_array.range(i*OUTPUT_WIDTH-1,(i-1)*OUTPUT_WIDTH) = neo_value;
				// cout<<i<<' '<< "prev "<<prev<<' '<<input_buffer[prev]<<"; cur "<< current_neo_idx<< ' '<< input_buffer[current_neo_idx]<<"; next "<< next<< ' '<< input_buffer[next]<<"; neo_value "<<neo_value<<endl;
				current_neo_idx++;
				if (current_neo_idx == BUFFER_SIZE)
                    current_neo_idx = 0;
			}

			// Return results
#ifndef NATIVE_SYSTEMC
			{
				HLS_DEFINE_PROTOCOL("output");
				o_result.put(output_array);
				wait();
			}
#else
			o_result.write(neo_value);
#endif
	}
}
