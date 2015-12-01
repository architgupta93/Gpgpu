#include "cf_utils.h"
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>

void add_label_ids(FileName& ptx_output_stream_fname, char* fname)
{
	std::string line;
	// Creating a new name for this file that we are going
	// to generate
	for (int i=0; i<FNAME_SIZE; i++)
	{
		ptx_output_stream_fname.name[i] = fname[i];
		if (fname[i] == '\0')
		{
			if (i<FNAME_SIZE-4)
			{
				ptx_output_stream_fname.name[i] = '_';
				ptx_output_stream_fname.name[i+1] = 'l';
				ptx_output_stream_fname.name[i+2] = 'a';
				ptx_output_stream_fname.name[i+3] = 'b';
				ptx_output_stream_fname.name[i+4] = '\0';
			}
			else
			{
				ptx_output_stream_fname.name[FNAME_SIZE-5] = '_';
				ptx_output_stream_fname.name[FNAME_SIZE-4] = 'l';
				ptx_output_stream_fname.name[FNAME_SIZE-3] = 'a';
				ptx_output_stream_fname.name[FNAME_SIZE-2] = 'b';
				ptx_output_stream_fname.name[FNAME_SIZE-1] = '\0';
			}	
			break;
		}
	}
	std::ifstream ptx_input_stream;
	std::ofstream ptx_output_stream;
	int label_id = 0;
	ptx_input_stream.open(fname, std::ios::in);
	ptx_output_stream.open(ptx_output_stream_fname.name, std::ios::out);
	while (std::getline(ptx_input_stream, line))
	{
		if(line == "EXTRN:")
		{
			ptx_output_stream << ("EXTRN" + boost::lexical_cast<std::string>(label_id++) + ":\n");;	
		} 
		else if (line == "INTRN:")
		{

			ptx_output_stream << ("INTRN" + boost::lexical_cast<std::string>(label_id++) + ":\n");;	
		}
		else
		{
			ptx_output_stream << line+"\n";
		}
	}	
	ptx_input_stream.close();
	ptx_output_stream.close();
	return;
}

// Declaring functions for a BTB entry

tagged_branch_target_buffer_entry::tagged_branch_target_buffer_entry(bool _tag,
address_type _src, address_type _targ)
{
	tag = _tag;
	source = _src;	
	target = _targ;
	instances = 0;
	taken_count = 0;
	for(int i=0;i<WARP_SIZE+1;i++){
		occupancy[i]=0;
	}
}

tagged_branch_target_buffer_entry::tagged_branch_target_buffer_entry(
address_type _src, address_type _targ)
{
	source = _src;	
	target = _targ;
	instances = 0;
	taken_count = 0;
	for(int i=0;i<WARP_SIZE+1;i++){
		occupancy[i]=0;
	}
}

bool tagged_branch_target_buffer_entry::operator==(tagged_branch_target_buffer_entry 
const& btb_entry) const
{
	return (source==btb_entry.get_source() && target==btb_entry.get_target());
};


float tagged_branch_target_buffer_entry::get_occupancy() const
{
	int occupied = 0;
	int total_count = occupancy[0];
	for(int i=1;i<WARP_SIZE+1;i++){
		occupied += i*occupancy[i];
		total_count += occupancy[i];
	}
	return (((float) occupied)/((float) total_count*WARP_SIZE));
}

float tagged_branch_target_buffer_entry::get_taken_fraction() const
{
	return (((float) taken_count)/((float) instances));
}

void tagged_branch_target_buffer_entry::update_branch(bool& direction)
{
	instances++;
	if(direction) taken_count++;
}

void tagged_branch_target_buffer_entry::update_occupancy(int& warp_occ)
{
	occupancy[warp_occ]++;
}

void tagged_branch_target_buffer_entry::merge(const tagged_branch_target_buffer_entry* _entry)
{
	instances += _entry->get_instances();
	taken_count += _entry->get_taken_count();
	for (int i=0;i<WARP_SIZE+1; i++)
	{
		occupancy[i]+=_entry->occupancy[i];
	}
}

void tagged_branch_target_buffer_entry::print()
{
	const char* int_desc = "intrinsic";
	const char* ext_desc = "extrinsic";
	if (tag==BRANCH_INTRN)
		printf("%12x %12s %12x %10u %12f %15f\n", source, int_desc, target, instances, get_taken_fraction(), get_occupancy());
	else				
		printf("%12x %12s %12x %10u %12f %15f\n", source, ext_desc, target, instances, get_taken_fraction(), get_occupancy());
//	printf("%12x ", source);
//	if(tag==BRANCH_INTRN) printf("intrinsic ");
//	else printf("extrinsic ");
//	printf("%12x %20u %6f %6f\n", target, instances, get_taken_fraction(), get_occupancy());				
//	for (int i=0; i<WARP_SIZE+1;i++)
//	{
//		printf("W%d: %d, ", i, occupancy[i]);
//	}
//	printf("\n");
}

// Function declarations for a BTB

bool match_btb_entry::operator()(tagged_branch_target_buffer_entry* const&
btb_entry) const
{
	return (btb_entry->get_source()==source && btb_entry->get_target()==target);
}

tagged_branch_target_buffer::tagged_branch_target_buffer()
{
	btb = *(new std::vector<tagged_branch_target_buffer_entry*> ());
}

tagged_branch_target_buffer_entry* tagged_branch_target_buffer::find_btb_entry(
const bool& _tag, const address_type& _src, const address_type& _targ)	//SOHUM: added tag info here
{
	std::vector<tagged_branch_target_buffer_entry*>:: iterator it;
	it = std::find_if(btb.begin(), btb.end(), match_btb_entry(_src,_targ));
	if(it != btb.end()) {return *it;}
		// If the branch has already been seen, update it in the BTB
	tagged_branch_target_buffer_entry* btb_entry = new tagged_branch_target_buffer_entry(_tag,_src,_targ); //SOHUM: added tag info here
	btb.push_back(btb_entry);
		// Otherwise, create a new entry for this branch in the BTB
		// and return the new entry to be initialized elsewhere
	return btb.back();	
}

void tagged_branch_target_buffer::print()
{
	printf("----------------------------------------------------------------------------------------\n");
	printf("%12s %12s %12s %10s %12s %15s\n", "PC", "TYPE", "TARGET", "INSTANCES", "TAKEN", "OCCUPANCY");
	printf("----------------------------------------------------------------------------------------\n");
	std::vector<tagged_branch_target_buffer_entry*>:: iterator it;
	for (it=btb.begin(); it != btb.end(); it++)
	{
		(*it)->print();
	}
	return;
}

void tagged_branch_target_buffer::merge_btb(const tagged_branch_target_buffer* child_btb)
{
	tagged_branch_target_buffer_entry* match;
	std::vector<tagged_branch_target_buffer_entry*>:: const_iterator it;
	for (it=child_btb->btb.begin();it!=child_btb->btb.end();++it)
	{
		match = find_btb_entry((*it)->getTag(),(*it)->get_source(),(*it)->get_target());
		match->merge(*it);
	}
}

void tagged_branch_target_buffer::flush()
{
	btb.clear();	
}
