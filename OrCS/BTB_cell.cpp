#include "simulator.hpp"

BTB_cell_t::BTB_cell_t(){
    this->opcode_address=0;
    this->last_access=0;
    this->predictor=0;
    this->target_address=0;
};