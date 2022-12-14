#include "simulator.hpp"
#define INDEX_SIZE 10

int CBP_t::calculate_index(uint64_t PC,int bank){
    int index= (PC & 1023) ^ ((PC>>10) & 1023) ^ CSR_i[bank].comp;
    return index;
}


int CBP_t::calculate_tag(uint64_t PC,int bank){
    int tag = (PC) ^ (CSR_t[0][bank].comp) ^ (CSR_t[1][bank].comp<<1);
    return tag ;
}

void CBP_t::update_Bimodal(uint64_t PC){
    int index = PC & 4095;
    if(taken){
        if(Bimodal[index].ctr<7)
            Bimodal[index].ctr++;
    }else{
        if(Bimodal[index].ctr>0)
            Bimodal[index].ctr--;
    }
}

int CBP_t::check_Bimodal(uint64_t PC){
    int index = PC & 4095;
    if(Bimodal[index].ctr<4){
        return NTAKEN;
    }else{
        return TAKEN;
    }
}

void CBP_t::update_Banks(uint64_t PC,int amount){
    int randomize=1;
    int index=0;
    int randomBank=0;
    int tag=0;
    for (int i = amount-1; i >=0; i--){
        index= calculate_index(PC,i);
        if(banks[i].cell[index].u==0){
            randomize=0;
            tag= calculate_tag(PC,i);
            banks[i].cell[index].tag=tag;
            banks[i].cell[index].u=0;
            if(Bimodal[PC & 4095].m)
                banks[i].cell[index].ctr=3+taken;
            else{
                if(Bimodal[PC & 4095].ctr<4)
                    banks[i].cell[index].ctr=3;
                else
                    banks[i].cell[index].ctr=4;
            }
        }
    }
    if(randomize){
        randomBank = rand() % amount;
        index= calculate_index(PC,randomBank);
        tag= calculate_tag(PC,randomBank);
        banks[randomBank].cell[index].tag=tag;
        banks[randomBank].cell[index].u=0;
        if(Bimodal[PC & 4095].m)
            banks[randomBank].cell[index].ctr=3+taken;
        else{
            if(Bimodal[PC & 4095].ctr<4)
                banks[randomBank].cell[index].ctr=3;
            else
                banks[randomBank].cell[index].ctr=4;
        }
    }
}

int CBP_t::check_Bank(uint64_t PC,int bank){
    int index= calculate_index(PC,bank);
    int tag = calculate_tag(PC,bank);
    if(tag==banks[bank].cell[index].tag)
        return banks[bank].cell[index].ctr;
    else
        return -1;
}

void CBP_t::update_cbp(int bank,uint64_t PC){
    uint16_t index = calculate_index(PC,bank);
    
    if(bank==-1){
        update_Bimodal(PC);
    }else{
        if(taken){
            if(banks[bank].cell[index].ctr<7)
                banks[bank].cell[index].ctr++;
        }else{
            if(banks[bank].cell[index].ctr>0)
                banks[bank].cell[index].ctr--;
        }
    }
    if(!hit && bank!=0){
        if(bank==-1){
            update_Banks(PC,4);
        }
        else{
            update_Banks(PC,bank);
        }
    }
    if(update_bits){ 
        Bimodal[PC & 4095].m=hit; //Update the m bit
        banks[bank].cell[index].u=hit; //Update the u bit
        update_bits=0;
    }

    ghist = (ghist<<1) | (history_t) taken;
    for (int i = 0; i < 4; i++){
        CSR_i[i].update(ghist);
        CSR_t[0][i].update(ghist);
        CSR_t[1][i].update(ghist);
    }
    
}

int CBP_t::check_prediction(uint32_t size,int prediction,uint64_t PC,uint64_t next_address){
    if(prediction!=check_Bimodal(PC)){
        update_bits=1;
    }
    if(PC+size==next_address){
        taken=0;
        if(prediction)
            return PMISS;
        else
            return PHIT;
    }else{
        taken=1;
        if(!prediction)
            return PMISS;
        else
            return PHIT;
    }
}

int CBP_t::get_prediction(uint32_t size,uint64_t PC,int *bankFound,uint64_t next_address){
    int b[4];
    for (int i = 0; i < 4; i++){
        b[i]=check_Bank(PC,i);
        if(b[i]!=-1){
            (*bankFound)=i;
            break;
        }
    }
    if((*bankFound)==-1){
            return check_prediction(size,check_Bimodal(PC),PC,next_address);
    }else{
        if(b[*bankFound]<4)
            return check_prediction(size,NTAKEN,PC,next_address);
        else
            return check_prediction(size,TAKEN,PC,next_address);
    }
}



