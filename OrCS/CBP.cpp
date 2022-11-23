#include "simulator.hpp"

int CBP_t::check_Bimodal(uint64_t PC){
    int index = PC & 4095;
    return (Bimodal[index].ctr);
}

void CBP_t::update_Bimodal(uint64_t PC,int taken){
    int index = PC & 4095;
    if(taken){
        if(Bimodal[index].ctr<7)
            Bimodal[index].ctr++;
    }else{
        if(Bimodal[index].ctr>0)
            Bimodal[index].ctr--;
    }
}

void CBP_t::update_Banks(uint64_t PC,int amount){
    
}

int CBP_t::check_Bank(uint64_t PC,int bank){
    int index= (PC & 1023) ^ ((PC>>10) & 1023) ^ CSR_i[bank].comp;
    uint16_t tag = (PC & 255) ^ (CSR_t[bank]->comp) ^ (CSR_t[bank]->comp<<1);
    if(tag==banks[bank].cell[index].tag)
        return banks[bank].cell[index].ctr;
    else
        return -1;
}
        
int CBP_t::check_prediction(int prediction,uint32_t size,uint64_t PC,uint64_t next_address ){
    if(PC+size==next_address){
        if(!prediction)
            return PMISS;
        else
            return PHIT;
    }else{
        if(prediction)
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
            *bankFound=i;
            break;
        }
    }
    if(*bankFound==-1){
        if(check_Bimodal(PC)<4)
            return check_prediction(size,NTAKEN,PC,next_address);
        else
            return check_prediction(size,TAKEN,PC,next_address);
    }else{
        if(b[*bankFound]<4)
            return check_prediction(size,NTAKEN,PC,next_address);
        else
            return check_prediction(size,TAKEN,PC,next_address);
    }
}

void CBP_t::update_cbp(int hit,int taken,int bank,uint64_t PC){
    if(bank==-1){
        update_Bimodal(PC,taken);
        if(!taken){
            
        }
    }

}

