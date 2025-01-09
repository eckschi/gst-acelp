#include "c_source.h"
#include <memory.h>


/* Initial lsp values used after each time */
/* a reset is executed */
const Word16 lspold_init[] =
{
	30000, 26000,  21000,  15000,   8000, 
	    0, -8000, -15000, -21000, -26000
};


void init_tetra_op(tetra_op_t* top)
{
	// tetra_op.c ---
	{
		top->Overflow = 0;
		top->Carry = 0;
	}

	// sub_dsp.c ---
	{
		Word16 old_A[sizeof(top->old_A)] = 
		{ 
			4096, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
		};
		memcpy(top->old_A, old_A, sizeof(old_A));
	}

	// sub_sc_d.c ---
	{
		memcpy(top->lsp_old, lspold_init, sizeof(lspold_init));
	}

	// sdec_tet.c && scod_tet.c ---
	{		
		memcpy(top->lspold, lspold_init, sizeof(lspold_init));
	}	
}

