#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {
	this->miss=0;
	this->miss_counter=0;
};

// =====================================================================
void processor_t::allocate() {

};

int processor_t::handle_BTB(opcode_package_t instruction,int branch_type, u_int64_t next_address){
	uint64_t PC = instruction.opcode_address;
	int index = PC & 1023; //Calculate tag
	static long counter[4];
	static int flag=0;
	if(!flag){
		memset(counter,0,sizeof(counter));
		flag=1;
	}
	if(branch_type!=4){
		for(int i = 0; i < 4; i++){
			if(BTB[index][i].opcode_address==PC){
				BTB[index][i].last_access = orcs_engine.global_cycle;
				return HIT;
			}
		}
	}else{
		for(int i = 0; i < 4; i++){
			if(BTB[index][i].opcode_address==PC){ //check for hit or miss on BTB
				BTB[index][i].last_access = orcs_engine.global_cycle;
				if(BTB[index][i].two_bits==0 || BTB[index][i].two_bits==1){ //Predicted Not Taken
					if(next_address == PC+instruction.opcode_size){  //If not taken Sucess
						BTB[index][i].target_address=PC+instruction.opcode_size;
						if(BTB[index][i].two_bits>0){
							BTB[index][i].two_bits--;
						}
						return HIT;
					}else{											//If taken Fail
						BTB[index][i].target_address=next_address;
						if(BTB[index][i].two_bits<3){
							BTB[index][i].two_bits++;
						}
						return MISS;
					}
				}else{ //Predicted Taken
					if(next_address == PC+instruction.opcode_size){  //If not taken Fail
						BTB[index][i].target_address=PC+instruction.opcode_size;
						if(BTB[index][i].two_bits>0){
							BTB[index][i].two_bits--;
						}
						return MISS;
					}else{											//If taken Sucess
						BTB[index][i].target_address=next_address;
						if(BTB[index][i].two_bits<3){
							BTB[index][i].two_bits++;
						}
						return HIT;
					}
				}
			}
		}
	}
	//Didn't find it on BTB
	//AQUI O PROBLEMA PROVAVELMENTE
	int LRU=0;
	for (int i = 0; i < 4; i++){
		if(BTB[index][i].last_access==0){
			LRU=i;
			break;
		}
		if(BTB[index][LRU].last_access>BTB[index][i].last_access){
			LRU=i;
		}
	}
	counter[LRU]++;
	printf("LRU 0:%ld 1:%ld 2:%ld 3:%ld\n",counter[0],counter[1],counter[2],counter[3]);
	BTB[index][LRU].last_access= orcs_engine.global_cycle;
	BTB[index][LRU].opcode_address=PC;
	BTB[index][LRU].two_bits=0;
	if(branch_type==4)
		BTB[index][LRU].target_address=PC+instruction.opcode_size;
	else
		BTB[index][LRU].target_address=next_address;
	return MISS;
	
	 

}
// =====================================================================
void processor_t::print_trace(opcode_package_t new_instruction){
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
	if(miss!=0){
		miss--;
		return;
	}
	opcode_package_t actual_instruction;
	static opcode_package_t new_instruction;
	actual_instruction=new_instruction;
	if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
		/// If EOF
		orcs_engine.simulator_alive = false;
		return;
	}

	if(actual_instruction.opcode_operation==7){
		if(!handle_BTB(actual_instruction,actual_instruction.branch_type,new_instruction.opcode_address)){
			miss=16;
			miss_counter++;
		}
	}

	
};

// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");
	ORCS_PRINTF("TOTAL MISSES = %ld\n",miss_counter);
	ORCS_PRINTF("TOTAL CYCLES %ld\n",orcs_engine.global_cycle);

};
