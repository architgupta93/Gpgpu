#ifndef CF_UTILS_INCLUDED
#define CF_UTILS_INCLUDED

#include <vector>
#include <algorithm>
#include <iostream>

typedef unsigned address_type;

#define TAKEN true
#define NOT_TAKEN false
#define WARP_SIZE 32
#define FNAME_SIZE 1024

//SOHUM: The definitions used for branch types
#define BRANCH_INTRN 0
#define BRANCH_EXTRN 1

enum thread_active_status{
	ACTIVE,
	INACTIVE_EXTRINSIC,
	INACTIVE_INTRINSIC
};

// Structs/classes to define the inactive counters for different threads

class thread_status_table{
	friend class warp_inst_t;
	friend class simt_stack;
public:
	thread_status_table();	
	set_active_status_pointer(std::vector<thread_active_status*> _root);
	void set_active_status(thread_active_status status, unsigned laneId);
	void clock();
	void clear();

private:
	std::vector<thread_active_status>* m_thread_active_status(WARP_SIZE); 
    	std::vector<unsigned> m_thread_active_counter(WARP_SIZE);
    	std::vector<unsigned> m_thread_extrinsic_counter(WARP_SIZE);
   	std::vector<unsigned> m_thread_intrinsic_counter(WARP_SIZE);
};

// Renaming labels to provide unique IDs

struct FileName
{
	char name[FNAME_SIZE];
};

void add_label_ids(FileName& ptx_output_stream_fname, char* fname);

class tagged_branch_target_buffer_entry
{
public:

	// Constructor for the class
	tagged_branch_target_buffer_entry(bool _tag, address_type _src,
	address_type _targ);	//SOHUM: changed signature of tag from bool& to bool
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
	float get_taken_fraction() const;
	float get_occupancy() const;

	// Modifying the current state of the BTB entry
	void update_branch(bool& direction);
	void update_occupancy(int& warp_occ);
	void merge(tagged_branch_target_buffer_entry const* _entry);
					// Add the instances, taken count etc
					// of another BTB entry to this entry
	void print();
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
	match_btb_entry(const address_type& _src, const address_type& _targ) : source(_src), target(_targ) {}
	bool operator()(tagged_branch_target_buffer_entry* const& btb_entry) const;
};

class tagged_branch_target_buffer
{
private:
	std::vector<tagged_branch_target_buffer_entry*> btb;	

public:
	tagged_branch_target_buffer();
	tagged_branch_target_buffer_entry* find_btb_entry( const bool& _tag, const address_type& _src,
	const address_type& _targ);
	void merge_btb(const tagged_branch_target_buffer* child_btb);
	void print();
	void flush();
};
#endif // end #ifndef CF_UTILS_INCLUDED
