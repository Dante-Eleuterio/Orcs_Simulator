/* Author: Pierre Michaud
   November 2004
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <cstdlib>
#include <bitset>
#include <inttypes.h>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class


#define ASSERT(cond) if (!(cond)) {printf("assert line %d\n",__LINE__); exit(EXIT_FAILURE);}

// number of global tables (those indexed with global history)
#define NHIST 4
// base 2 logarithm of number of entries in bimodal table
#define LOGB 12
// base 2 logarithm of number of entry in each global table
#define LOGG 10 
// number of bits for each up-down saturating counters
#define CBITS 3
// number of bits for each "meta" counter
#define MBITS 1
// number of tag bits in each global table entry
#define TBITS 8
// maximum global history length used
#define MAXHIST 81


using namespace std;


typedef uint32_t address_t;
typedef bitset<MAXHIST> history_t;


// this is the cyclic shift register for folding 
// a long global history into a smaller number of bits

class folded_history 
{
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
    ASSERT(OLENGTH < MAXHIST);
  }

  void update(history_t h)
    {
      ASSERT((comp>>CLENGTH)==0);
      comp = (comp << 1) | h[0];
      comp ^= h[OLENGTH] << OUTPOINT;
      comp ^= (comp >> CLENGTH);
      comp &= (1<<CLENGTH)-1;
    }
};


// all the predictor is there

class PREDICTOR
{
  public:

  // bimodal table entry
    class bentry {
    public:
      int8_t ctr;
      int8_t meta;
      bentry() {
	ctr = (1 << (CBITS-1));
	for (int i=0; i<NHIST; i++) {
	  meta = (1 << (MBITS-1));
	}
      }
    };

    // global table entry
    class gentry {
    public:
      int8_t ctr;
      uint16_t tag;
      bool ubit;
      gentry() {
	ctr = (1 << (CBITS-1));
	tag = 0;
	ubit = false;
      }
    };

    // predictor storage data
    history_t ghist;
    folded_history ch_i[NHIST];
    folded_history ch_t[2][NHIST];
    bentry * btable;
    gentry * gtable[NHIST];


    PREDICTOR()
      {
	ghist = 0;
	ch_i[0].init(80,LOGG); // 80-bit global history
	ch_i[1].init(40,LOGG); // 40-bit global history
	ch_i[2].init(20,LOGG); // 20-bit global history
	ch_i[3].init(10,LOGG); // 10-bit global history
	for (int i=0; i<NHIST; i++) {
	  ch_t[0][i].init(ch_i[i].OLENGTH,TBITS);
	  ch_t[1][i].init(ch_i[i].OLENGTH,TBITS-1);
	}
	btable = new bentry [1<<LOGB];
	for (int i=0; i<NHIST; i++) {
	  gtable[i] = new gentry [1<<LOGG];
	}
      }


    // index function for the bimodal table
    int bindex(address_t pc) 
      {
	return(pc & ((1<<LOGB)-1));
      }

    // index function for the global tables
    int gindex(address_t pc, int bank)
      {
	int index = pc ^ (pc>>LOGG) ^ ch_i[bank].comp;
	return(index & ((1<<LOGG)-1));
      }

    // index function for the tags
    uint16_t gtag(address_t pc, int bank)
      {
	int tag = pc ^ ch_t[0][bank].comp ^ (ch_t[1][bank].comp << 1);
	return(tag & ((1<<TBITS)-1));
      }

    // most significant bit of up-down saturating counter
    bool ctrpred(int8_t ctr, int nbits)
      {
	return((ctr >> (nbits-1)) != 0);
      }

    // up-down saturating counter
    void ctrupdate(int8_t & ctr,bool taken,int nbits)
      {
	if (taken) {
	  if (ctr < ((1<<nbits)-1)) {
	    ctr++;
	  }
	} else {
	  if (ctr > 0) {
	    ctr--;
	  }
	}
      }

    // prediction given by longest matching global history
    bool read_prediction(address_t pc, int & bank)
      {
	int index;
	bank = NHIST;
	for (int i=0; i < NHIST; i++) {
	  index = gindex(pc,i);
	  if (gtable[i][index].tag == gtag(pc,i)) {
	    bank = i;
	    break;
	  }
	}
	if (bank < NHIST) {
	  return ctrpred(gtable[bank][index].ctr,CBITS);
	} else {
	  return ctrpred(btable[bindex(pc)].ctr,CBITS);
	}
      }

    // PREDICTION
    bool get_prediction(const branch_record_c* br, const op_state_c* os)
      {
	int bank;
	bool prediction = true;
	if (br->is_conditional) {
	  address_t pc = br->instruction_addr;
	  prediction = read_prediction(pc, bank);
	}
	return prediction;
      }


    // PREDICTOR UPDATE
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
      {
	bool pred_taken,btaken;
	int bank,bi,gi[NHIST];
	if (br->is_conditional) {
	  address_t pc = br->instruction_addr;
	  bi = bindex(pc);
	  // in a real processor, it is not necessary to re-read the predictor at update
          // it suffices to propagate the prediction along with the branch instruction
	  pred_taken = read_prediction(pc, bank);
	  btaken = ctrpred(btable[bi].ctr,CBITS);

	  // steal new entries only if prediction was wrong
	  if ((pred_taken != taken) && (bank > 0)) {
	    bool choose_random = true;
	    for (int i=0; i<bank; i++) {
	      gi[i] = gindex(pc,i);
	      choose_random = choose_random && (gtable[i][gi[i]].ubit);
	    }
	    bool init_taken = (ctrpred(btable[bi].meta,MBITS))? taken : btaken;
	    if (choose_random) {
	      int i = (unsigned) random() % bank;
	      gtable[i][gi[i]].tag = gtag(pc,i);
	      gtable[i][gi[i]].ctr = (1<<(CBITS-1)) - ((init_taken)? 0:1);
	      gtable[i][gi[i]].ubit = false;
	    } else {
	      for (int i=0; i<bank; i++) {
		if (! gtable[i][gi[i]].ubit) {
		  gtable[i][gi[i]].tag = gtag(pc,i);
		  gtable[i][gi[i]].ctr = (1<<(CBITS-1)) - ((init_taken)? 0:1);
		  gtable[i][gi[i]].ubit = false;
		}
	      }
	    }
	  }

	  // update the counter that provided the prediction, and only it
	  if (bank < NHIST) {
	    gi[bank] = gindex(pc,bank);
	    ASSERT(pred_taken == ctrpred(gtable[bank][gi[bank]].ctr,CBITS));
	    ctrupdate(gtable[bank][gi[bank]].ctr,taken,CBITS);
	  } else {
	    ctrupdate(btable[bi].ctr,taken,CBITS);
	  }

	  // update the meta counter
	  if (pred_taken != btaken) {
	    ASSERT(bank < NHIST);
	    ctrupdate(btable[bi].meta,(pred_taken==taken),MBITS);
	    gtable[bank][gi[bank]].ubit = (pred_taken == taken);
	  }

	  // update global history and cyclic shift registers
	  ghist = (ghist << 1) | (history_t) taken;
	  for (int i=0; i<NHIST; i++) {
	    ch_i[i].update(ghist);
	    ch_t[0][i].update(ghist);
	    ch_t[1][i].update(ghist);
	  }
	}
      }
};

#endif // PREDICTOR_H_SEEN

