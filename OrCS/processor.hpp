#include "simulator.hpp"

// ============================================================================
// ============================================================================
#define BMISS 0
#define BHIT 1
#define PMISS 2
#define PHIT 3
class processor_t {
    private:    

    public:
		BTB_cell_t BTB[1024][4];
		CBP_t CBP;
		int miss;
		long BTB_misses;
		long BTB_hits;
		long prediction_hits;
		long prediction_misses;		
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock();
	    void statistics();
		void print_trace(opcode_package_t new_instruction);
		int handle_BTB(opcode_package_t instruction,int branch_type, u_int64_t next_address);
		int two_bits(opcode_package_t instruction, u_int64_t next_address,int i);
		int handle_cbp(uint32_t size,uint64_t PC, uint64_t next_address);

};
