#ifndef CF_UTILS_INCLUDED
#define CF_UTILS_INCLUDED

typedef unsigned address_type

#define TAKEN true
#define NOT_TAKEN false
#define WARP_SIZE 32

class tagged_branch_target_buffer_entry
{
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
public:

	// Constructor for the class
	tagged_branch_target_buffer_entry(bool& _tag, address_type _src,
	address_type _targ);

	// Destructor for the class
	~tagged_branch_target_buffer_entry();

	// Defining operators (equality or others, as and when required)
	bool operator==(tagged_branch_target_entry const& btb_entry) const;

	// Reading the current state of the BTB entry
	bool getTag() {return tag;}
	address_type getSource() {return source;}
	address_type getTarget() {return target;}
	int getTakenCount() {return taken_count;}
	int getInstances() {return instances;}
	double getTakenFraction();
	double getOccupancy();

	// Modifying the current state of the BTB entry
	void updateBranch(bool& direction);
	void updateOccupancy(int& warp_occ);
}

struct match_btb_entry: std::unary_function<tagged_branch_target_buffer_entry*, bool>{
	address_type source;
	address_type target;
	match_btb_entry(address_type _src, address_type _targ) : source(_src), target(_targ) {}
	bool operator()(tagged_branch_target_buffer_entry* const& btb_entry) const;
}

class tagged_branch_target_buffer
{
private:
	tagged_branch_target_buffer_entry** btb;	

public:
	tagged_branch_target_buffer();
	tagged_branch_target_buffer_entry* find_btb_entry(address_type& _src,
	address_type& _targ);
}
#endif // end #ifndef CF_UTILS_INCLUDED
