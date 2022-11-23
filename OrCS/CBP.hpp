#include "simulator.hpp"

using namespace std;
typedef bitset<81> history_t;
#define NTAKEN 0
#define TAKEN 1

class folded_history{
    public:
    unsigned comp;
    int CLENGTH;
    int OLENGTH;
    int OUTPOINT;

    folded_history () {}

    void init(int original_length, int compressed_length) {
        comp = 0;
        OLENGTH = original_length;
        CLENGTH = compressed_length;
        OUTPOINT = OLENGTH % CLENGTH;
    }

    void update(history_t h){
        comp = (comp << 1) | h[0];
        comp ^= h[OLENGTH] << OUTPOINT;
        comp ^= (comp >> CLENGTH);
        comp &= (1<<CLENGTH)-1;
    }
};

class CBP_t {
    public:
        class Bimodal_cell{
            public:
            Bimodal_cell(){
                ctr=3;
                m=0;
            };
            int8_t ctr;
            int m;
        };
        class Bank_t{
            public:
            class Bank_cell{
                public:
                int8_t ctr;
                uint16_t tag;
                int u;
                Bank_cell(){
                    ctr=3;
                    tag=0;
                    u=0;
                };
            };
            Bank_cell cell[1024];
            Bank_t(){}
        };
        Bimodal_cell Bimodal[4096];
        Bank_t banks[4];
        folded_history CSR_i[4];
        folded_history CSR_t[2][4];
        history_t ghist;
        CBP_t(){
            int bits=80;
            ghist=0;
            for (int i = 0; i < 4; i++){
                CSR_i[i].init(bits,10);
                CSR_t[0][i].init(CSR_i[i].OLENGTH,8);
                CSR_t[0][i].init(CSR_i[i].OLENGTH,7);
                bits/=2;
            }
        }
        int get_prediction(uint32_t size,uint64_t PC,int *bankFound,uint64_t next_address);
        int check_Bimodal(uint64_t PC);
        int check_Bank(uint64_t PC,int bank);
        int check_prediction(int prediction,uint32_t size,uint64_t PC,uint64_t next_address );
        void update_cbp(int hit,int prediction,int bank,uint64_t PC);
        void update_Bimodal(uint64_t PC,int taken);

};
