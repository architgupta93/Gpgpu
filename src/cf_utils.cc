#include "cf_utils.h"

// Declaring functions for a BTB entry

tagged_branch_target_buffer_entry::tagged_branch_target_buffer_entry(bool& _tag,
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


double tagged_branch_target_buffer_entry::get_occupancy() const
{
	int occupied = 0;
	int total_count = 0;
	for(int i=1;i<WARP_SIZE+1;i++){
		occupied = i*occupancy[i];
		total_count += occupancy[i];
	}
	return (((float) occupied)/((float) total_count*WARP_SIZE));
}

double tagged_branch_target_buffer_entry::get_taken_fraction() const
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
address_type& _src, address_type& _targ)
{
	std::vector<tagged_branch_target_buffer_entry*>:: iterator it;
	it = std::find_if(btb.begin(), btb.end(), match_btb_entry(_src,_targ));
	if(it != btb.end()) {return *it;}
		// If the branch has already been seen, update it in the BTB
	tagged_branch_target_buffer_entry* btb_entry = new tagged_branch_target_buffer_entry(_src,_targ);
	btb.push_back(btb_entry);
		// Otherwise, create a new entry for this branch in the BTB
		// and return the new entry to be initialized elsewhere
	return btb.back();	
}

