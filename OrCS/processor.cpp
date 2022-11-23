#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {
	this->miss=0;
	this->BTB_misses=0;
	this->BTB_hits=0;
	this->prediction_hits=0;
	this->prediction_misses=0;
};

// =====================================================================
void processor_t::allocate() {

};

int processor_t::handle_cbp(uint32_t size,uint64_t PC,uint64_t next_address){
	int bankFound=-1;
	int result=0;
	result=CBP.get_prediction(size,PC,&bankFound,next_address);
	
	return result;
}

int processor_t::two_bits(opcode_package_t instruction, u_int64_t next_address,int i){
	uint64_t PC = instruction.opcode_address;
	int index = PC & 1023; //Calculate tag
	
	if(BTB[index][i].prediction==0 || BTB[index][i].prediction==1){ //Predicted Not Taken
			if(next_address == PC+instruction.opcode_size){  //If not taken Sucess
				BTB[index][i].target_address=PC+instruction.opcode_size;
				if(BTB[index][i].prediction>0){
					BTB[index][i].prediction--;
				}
				return PHIT;
			}else{											//If taken Fail
				BTB[index][i].target_address=next_address;
				if(BTB[index][i].prediction<3){
					BTB[index][i].prediction++;
				}
				return PMISS;
			}
		}else{ //Predicted Taken
			if(next_address == PC+instruction.opcode_size){  //If not taken Fail
				BTB[index][i].target_address=PC+instruction.opcode_size;
				if(BTB[index][i].prediction>0){
					BTB[index][i].prediction--;
				}
				return PMISS;
			}else{											//If taken Sucess
				BTB[index][i].target_address=next_address;
				if(BTB[index][i].prediction<3){
					BTB[index][i].prediction++;
				}
				return PHIT;
			}
	}
}


int processor_t::handle_BTB(opcode_package_t instruction,int branch_type, u_int64_t next_address){
	uint64_t PC = instruction.opcode_address;
	int index = PC & 1023; //Calculate tag
	
	if(branch_type!=4){
		for(int i = 0; i < 4; i++){
			if(BTB[index][i].opcode_address==PC){
				BTB[index][i].last_access = orcs_engine.global_cycle;
				return BHIT;
			}
		}
	}else{
		for(int i = 0; i < 4; i++){
			if(BTB[index][i].opcode_address==PC){ //check for hit or miss on BTB
				BTB[index][i].last_access = orcs_engine.global_cycle;
				// return (two_bits(instruction,next_address,i));
				return handle_cbp(instruction.opcode_size,PC,next_address);
			}
		}
	}
	//Didn't find it on BTB

	int LRU=0;
	for (int i = 0; i < 4; i++){
		if(BTB[index][i].last_access==0){ //Checks if there is an empty space
			LRU=i;
			break;
		}
		if(BTB[index][LRU].last_access>BTB[index][i].last_access){
			LRU=i; //Saves that space as the LRU to use if an empty space is not found
		}
	}
	BTB[index][LRU].last_access= orcs_engine.global_cycle;
	BTB[index][LRU].opcode_address=PC;
	BTB[index][LRU].prediction=0;
	if(branch_type==4)
		BTB[index][LRU].target_address=PC+instruction.opcode_size;
	else
		BTB[index][LRU].target_address=next_address;
	return BMISS;
	
	 

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
		orcs_engine.simulator_alive = false;
		return;
	}
	
	if(actual_instruction.opcode_operation==7){
		switch (handle_BTB(actual_instruction,actual_instruction.branch_type,new_instruction.opcode_address)){
		case BMISS:
			miss=16;
			BTB_misses++;
			break;
		case BHIT:
			BTB_hits++;
			break;
		case PMISS:
			miss=16;
			prediction_misses++;
			break;
		case PHIT:
			prediction_hits++;
			break;
		default:
			break;
		}
	}
};

// =====================================================================
void processor_t::statistics() {
	double hits = prediction_hits;
	double total= (prediction_hits+prediction_misses);
	double division=(hits/total)*100;
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");
	ORCS_PRINTF("BTB MISSES = %ld\n",BTB_misses);
	ORCS_PRINTF("BTB HITS = %ld\n",BTB_hits+prediction_hits);
	ORCS_PRINTF("Prediction HITS = %ld\n",prediction_hits);
	ORCS_PRINTF("Prediction MISSES = %ld\n",prediction_misses);
	ORCS_PRINTF("Prediction HITS percentage = %f%c \n",division,'%');
	ORCS_PRINTF("TOTAL CYCLES %ld\n\n",orcs_engine.global_cycle);
};
