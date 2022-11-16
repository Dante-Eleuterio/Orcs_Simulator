#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {

};

// =====================================================================
void print_trace(opcode_package_t new_instruction){
	printf("opcode_operation: %d\n",new_instruction.opcode_operation);
	printf("opcode_address: %ld\n",new_instruction.opcode_address);
	printf("opcode_size: %d\n",new_instruction.opcode_size);
	for (int i = 0; i < 16; i++){
		if(new_instruction.read_regs[i]!=0){
			printf("read_reg %d: %d\n",i,new_instruction.read_regs[i]);
		}
	}
	for (int i = 0; i < 16; i++)
	{
		if(new_instruction.write_regs[i]!=0){
			printf("write_reg %d: %d\n",i,new_instruction.write_regs[0]);
		}
	}
	
	printf("base_reg: %d\n",new_instruction.base_reg);
	printf("index_reg: %d\n",new_instruction.index_reg);
	printf("is_read: %d\n",new_instruction.is_read);
	if(new_instruction.is_read){
		printf("read_addres: %ld\n",new_instruction.read_address);
		printf("read_size: %d\n",new_instruction.read_size);
	}
	printf("is_read2: %d\n",new_instruction.is_read2);
	if(new_instruction.is_read2){
		printf("read2_address: %ld\n",new_instruction.read2_address);
		printf("read2_size: %d\n",new_instruction.read2_size);
	}
	printf("is_write: %d\n",new_instruction.is_write);
	if(new_instruction.is_write){
		printf("write_address: %ld\n",new_instruction.write_address);
		printf("write_size: %d\n",new_instruction.write_size);
	}
	printf("branch_type: %d\n",new_instruction.branch_type);
	printf("is_indirect: %d\n",new_instruction.is_indirect);
	printf("is_predicated: %d\n",new_instruction.is_predicated);
	printf("is_prefetch: %d\n",new_instruction.is_prefetch);
	printf("--------------------------\n");

}

// =====================================================================
void processor_t::clock() {

	/// Get the next instruction from the trace
	opcode_package_t new_instruction;
	bool teste= orcs_engine.trace_reader->trace_fetch(&new_instruction);
	print_trace(new_instruction);
	teste= orcs_engine.trace_reader->trace_fetch(&new_instruction);
	print_trace(new_instruction);
	teste= orcs_engine.trace_reader->trace_fetch(&new_instruction);
	print_trace(new_instruction);

	if(new_instruction.opcode_operation==7){
		teste= orcs_engine.trace_reader->trace_fetch(&new_instruction);
		print_trace(new_instruction);
		if(new_instruction.opcode_address!=4231689){
			printf("PULOU\n");
		}
		else{
			printf("NÃO PULOU\n");
		}
	}

	if (teste) {
		/// If EOF
		orcs_engine.simulator_alive = false;
	}

};

// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

};
