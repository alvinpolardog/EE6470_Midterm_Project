#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

#include <esc.h>                // for the latency logging functions


#include "Testbench.h"

#include <queue>
static std::queue<sc_time> time_queue;

// Testbench Constructor
Testbench::Testbench(sc_module_name n, string output_file) : sc_module(n), output_file_name(output_file){
  SC_THREAD(pass_input_values);
  sensitive << i_clk.pos();
  dont_initialize();
  SC_THREAD(fetch_result);
  sensitive << i_clk.pos();
  dont_initialize();
}

// Testbench Destructor
Testbench::~Testbench() {
	cout << "Total run time = " << total_run_time << endl;
}


// Open the input file, and start input stream
void Testbench::open_input_file(string input_file_name){
  const char * infile_name = input_file_name.c_str();
  stim_file.open(infile_name);
  if(stim_file.fail()){
    cerr << "Couldn't open "<< infile_name << "for reading." <<endl;
    exit(0);
  }
}

// Read value from the input file through the input stream 'stim_file'
input_type Testbench::read_value_from_input(bool & eof){
  input_type value;
  stim_file >> std::ws;
  eof = ( stim_file.eof() );
  if(!stim_file.eof()){
      stim_file >> value;
      eof = ( stim_file.eof() );
  }
  return value;
}

// Close input stream
void Testbench::close_input_file(){
  stim_file.close();
}

// Read over input file, 
// bundling values according to CHANNEL_WIDTH,
// and send it to the NEO module
void Testbench::pass_input_values() {
	n_txn = 0;
	max_txn_time = SC_ZERO_TIME;
	min_txn_time = SC_ZERO_TIME;
	total_txn_time = SC_ZERO_TIME;

#ifndef NATIVE_SYSTEMC
	o_value.reset();
#endif
	o_rst.write(false);
	wait(5);
	o_rst.write(true);
	wait(1);
	total_start_time = sc_time_stamp();

  bool eof = false;

  input_channel_type first_buffer = 0;
  input_type value = read_value_from_input(eof);
  first_buffer.range(INPUT_WIDTH-1,0) = value;
  value = read_value_from_input(eof);
  first_buffer.range(2*INPUT_WIDTH-1,INPUT_WIDTH) = value;
#ifndef NATIVE_SYSTEMC
      o_value.put(first_buffer);
#else
      o_value.write(first_buffer);
#endif
  int channel_filled = 0;
  input_channel_type buffer;
  while(eof == false){
    value = read_value_from_input(eof);
    channel_filled++;
    buffer.range(channel_filled*INPUT_WIDTH-1, (channel_filled-1)*INPUT_WIDTH) = value;
    time_queue.push( sc_time_stamp() );

    if ((!eof) && channel_filled == CHANNEL_WIDTH){
#ifndef NATIVE_SYSTEMC
      o_value.put(buffer);
#else
      o_value.write(buffer);
#endif
    channel_filled = 0;
    }
  }

  if (channel_filled > 0){
    if (channel_filled != CHANNEL_WIDTH)
      buffer.range(CHANNEL_WIDTH*INPUT_WIDTH-1,channel_filled*INPUT_WIDTH) = 0;
#ifndef NATIVE_SYSTEMC
    o_value.put(buffer);
#else
    o_value.write(buffer);
#endif
  }
  
  wait(1000);
  close_input_file();
  cerr << name() << " Error! Input timed out!" << endl;

#ifndef NATIVE_SYSTEMC
  esc_stop();
#else
  sc_stop();
#endif
}


// Open output file stream
void Testbench::open_output_file( string output_file_name ){
  // put output file in appropriate directory in bdw_work
  std::string filename = getenv( "BDW_SIM_CONFIG_DIR" );
  filename += "/";
  filename += output_file_name;
  cout<<filename<<endl;
  resp_file.open( filename.c_str() );
  if( resp_file.fail()){
      cerr << "Couldn't open " << filename << " for writing." << endl;
      exit( 0 );
  }
}

// Write value to output stream
void Testbench::write_value_to_output(output_type value ){
    resp_file << value << endl;
}

// Close output stream
void Testbench::close_output_file(){
  resp_file.close();
}


// Receive bundled results
// decompose the results and write to output stream
void Testbench::fetch_result() {
#ifndef NATIVE_SYSTEMC
	i_result.reset();
#endif
  open_output_file(output_file_name);

	wait(5);
	wait(1);
  unsigned long total_latency = 0;
  output_type neo_value = 0;

  int i = 0;
  while(i < SIGNAL_LENGTH-2){
#ifndef NATIVE_SYSTEMC
			output_channel_type output_array = i_result.get();
#else
			output_channel_type output_array = i_result.read();
#endif
      int array_idx = 1;
      while(i<SIGNAL_LENGTH-2){
        neo_value = output_array.range(array_idx*OUTPUT_WIDTH-1,(array_idx-1)*OUTPUT_WIDTH);
        cout<<"NEO RESULT #"<< i << " : "<<neo_value<<endl;
        write_value_to_output(neo_value);
        sc_time sent_time( time_queue.front() );
        time_queue.pop();
        unsigned long latency = clock_cycle( sc_time_stamp() - sent_time );
        total_latency += latency;
        cout << "Latency for sample " << i << " is " <<  latency << endl;
        i++;
        if (array_idx++ == CHANNEL_WIDTH)
          break;
      }
  }
  unsigned long average_latency = (total_latency / (SIGNAL_LENGTH-2)) + 1;
  cout << "Testbench sink thread read " << SIGNAL_LENGTH << " values. Average latency " << average_latency <<  "." << endl;

	total_run_time = sc_time_stamp() - total_start_time;


  close_output_file();
#ifndef NATIVE_SYSTEMC
  esc_stop();
#else
  sc_stop();
#endif
}


// Solve for clock cycle from sc_time
int Testbench::clock_cycle( sc_time time )
{
    sc_clock * clk_p = DCAST < sc_clock * >( i_clk.get_interface() );
    sc_time clock_period = clk_p->period(); // get period from the sc_clock object.
    return ( int )( time / clock_period );

}


