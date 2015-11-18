#include "cf_utils.h"

tagged_branch_target_buffer_entry::tagged_branch_target_buffer_entry(bool& _tag,
address_type _src, address_type _targ)
{
	tag = _tag;
	source = _src;	
	target = _targ;
	instances = 0;
	taken_count = 0;
	for(int i=0;i:WARP_SIZE+1;i++){
		occupancy[i]=0;
	}
}

bool tagged_branch_target_buffer_entry::operator==(tagged_branch_target_entry 
const& btb_entry) const;
{
	return (source=btb_entry.getSource() && target=btb_entry.getTarget());
};


double tagged_branch_target_buffer_entry::getOccupancy()
{
	int occupied = 0;
	int total_count = 0;
	for(int i=1;i<WARP_SIZE+1;i++){
		occupied = i*occupancy[i];
		total_count += occupancy[i];
	}
	return (((float) occupied)/((float) total_count*WARP_SIZE));
}

double tagged_branch_target_buffer_entry::getTakenFraction()
{
	return (((float) taken_fraction)/((float) instances));
}

void tagged_branch_target_buffer_entry::updateBranch(bool& direction)
{
	instances++;
	if(direction) taken_count++;
}

void tagged_branch_target_buffer_entry::updateOccupancy(int& warp_occ)
{
	occupancy[warp_occ]++;
}

tagged_branch_target_buffer::tagged_branch_target_buffer
{
	btb = *(new tagged_branch_target_buffer_entry**);
}

tagged_branch_target_buffer_entry* tagged_branch_target_buffer::find_btb_entry(
address_type _src, address_type _targ)
{
	it = std::find_if(btb.begin(), btb.end(), match_btb_entry(_src,_targ));
	return *it;
}

bool match_btb_entry::operator()(tagged_branch_target_buffer_entry* const&
btb_entry) const
{
	return (btb_entry->getSource()==source && btb_entry->getTarget()==target);
}
