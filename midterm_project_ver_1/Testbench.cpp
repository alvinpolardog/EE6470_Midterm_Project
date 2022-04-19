#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

#include <esc.h>                // for the latency logging functions


#include "Testbench.h"

#include <queue>
static std::queue<sc_time> time_queue;


Testbench::Testbench(sc_module_name n, string output_file) : sc_module(n), output_file_name(output_file){
  SC_THREAD(pass_input_values);
  sensitive << i_clk.pos();
  dont_initialize();
  SC_THREAD(fetch_result);
  sensitive << i_clk.pos();
  dont_initialize();
}

Testbench::~Testbench() {
	//cout<< "Max txn time = " << max_txn_time << endl;
	//cout<< "Min txn time = " << min_txn_time << endl;
	//cout<< "Avg txn time = " << total_txn_time/n_txn << endl;
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

void Testbench::close_input_file(){
  stim_file.close();
}


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
  while(eof == false){
    input_type value = read_value_from_input(eof);
    if (!eof){
#ifndef NATIVE_SYSTEMC
      o_value.put(value);
      time_queue.push( sc_time_stamp() );
#else
      o_value.write(value);
      time_queue.push( sc_time_stamp() );
#endif
    }
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

void Testbench::write_value_to_output(output_type value ){
    resp_file << value << endl;
}

void Testbench::close_output_file(){
  resp_file.close();
}



void Testbench::fetch_result() {
#ifndef NATIVE_SYSTEMC
	i_result.reset();
#endif
  open_output_file(output_file_name);

	wait(5);
	wait(1);
  unsigned long total_latency = 0;

  for (int i = 0; i < SIGNAL_LENGTH-2; i++){
#ifndef NATIVE_SYSTEMC
			output_type neo_value = i_result.get();
#else
			output_type neo_value = i_result.read();
#endif
      // cout<<"NEO RESULT #"<< i << " : "<<neo_value<<endl;
      write_value_to_output(neo_value);
      sc_time sent_time( time_queue.front() );
      time_queue.pop();
      unsigned long latency = clock_cycle( sc_time_stamp() - sent_time );
      total_latency += latency;
      cout << "Latency for sample " << i << " is " <<  latency << endl;
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

int Testbench::clock_cycle( sc_time time )
{
    sc_clock * clk_p = DCAST < sc_clock * >( i_clk.get_interface() );
    sc_time clock_period = clk_p->period(); // get period from the sc_clock object.
    return ( int )( time / clock_period );

}


