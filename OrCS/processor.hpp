#include "simulator.hpp"

// ============================================================================
// ============================================================================
class processor_t {
    private:    

    public:
		BTB_cell_t BTB[1024][4];
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock();
	    void statistics();
		void print_trace(opcode_package_t new_instruction);
};
