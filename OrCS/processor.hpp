#include "simulator.hpp"

// ============================================================================
// ============================================================================
#define HIT 1;
#define MISS 0;
class processor_t {
    private:    

    public:
		BTB_cell_t BTB[1024][4];
		int miss;
		long miss_counter;		
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock();
	    void statistics();
		void print_trace(opcode_package_t new_instruction);
		int handle_BTB(opcode_package_t instruction,int branch_type, u_int64_t next_address);
};
