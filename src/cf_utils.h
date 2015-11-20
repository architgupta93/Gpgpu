#ifndef CF_UTILS_INCLUDED
#define CF_UTILS_INCLUDED

#include <vector>
#include <algorithm>
#include <iostream>

typedef unsigned address_type;

#define TAKEN true
#define NOT_TAKEN false
#define WARP_SIZE 32

class tagged_branch_target_buffer_entry
{
public:

	// Constructor for the class
	tagged_branch_target_buffer_entry(bool& _tag, address_type _src,
	address_type _targ);
	tagged_branch_target_buffer_entry(address_type _src, address_type _targ);

	// Destructor for the class
	~tagged_branch_target_buffer_entry();

	// Defining operators (equality or others, as and when required)
	bool operator==(tagged_branch_target_buffer_entry const& btb_entry) const;

	// Reading the current state of the BTB entry
	bool getTag() {return tag;}
	address_type get_source() const{return source;}
	address_type get_target() const{return target;}
	int get_taken_count() const{return taken_count;}
	int get_instances() const{return instances;}
	double get_taken_fraction() const;
	double get_occupancy() const;

	// Modifying the current state of the BTB entry
	void update_branch(bool& direction);
	void update_occupancy(int& warp_occ);
private:
	bool tag;			// Indicates whether the branch is
					// intrinsic or extrinsic
	address_type source;
	address_type target;
	unsigned int instances;
	unsigned int taken_count;
	int occupancy[WARP_SIZE+1];	// Each index in the occupancy array
					// stores the number of times an
					// occupancy of "index" was seen
};

struct match_btb_entry{
	address_type source;
	address_type target;
	match_btb_entry(address_type _src, address_type _targ) : source(_src), target(_targ) {}
	bool operator()(tagged_branch_target_buffer_entry* const& btb_entry) const;
};

class tagged_branch_target_buffer
{
private:
	std::vector<tagged_branch_target_buffer_entry*> btb;	

public:
	tagged_branch_target_buffer();
	tagged_branch_target_buffer_entry* find_btb_entry(address_type& _src,
	address_type& _targ);
};
#endif // end #ifndef CF_UTILS_INCLUDED
