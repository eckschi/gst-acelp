#pragma once

#ifndef	__C_STRUCTS_H__
#define	__C_STRUCTS_H__

#ifdef __cplusplus
extern "C" {
#endif 

//#define PRINT_ERRORS

#define	STRUCTS_pp			 10

#define	STRUCTS_L_window	256
#define STRUCTS_L_next		 40
#define	STRUCTS_L_frame		240
#define	STRUCTS_L_subfr		 60
#define	STRUCTS_p			 10
#define	STRUCTS_pp1			 11
#define	STRUCTS_pit_min		 20
#define	STRUCTS_pit_max		143
#define	STRUCTS_L_inter		 15
#define	STRUCTS_parm_size	 23
#define	STRUCTS_dim_rr		 32
#define	STRUCTS_L_total		(STRUCTS_L_frame + STRUCTS_L_next + STRUCTS_p)

#define STRUCTS_gamma1		31130
#define STRUCTS_gamma2		19661
#define STRUCTS_gamma3		24576
#define STRUCTS_gamma4		27853


/* Initial lsp values used after each time */
/* a reset is executed */
extern const Word16 lspold_init[];


typedef struct ST_TETRA_OP tetra_op_t;

struct ST_TETRA_OP
{
// tetra_op.c ---
	/* Constants and Globals */
	Flag Overflow;
	Flag Carry;
	
// sub_dsp.c ---

	/* Last A(z) for case of unstable filter */
	Word16 old_A[STRUCTS_pp + 1];

// sub_sc_d.c ---

	/* Clsp_334 */
	Word16 lsp_old[10];

	/* Static values to be preserved between calls */
	Word16 y_hi;
	Word16 y_lo;
	Word16 x0;

// sdec_tet.c && scod_tet.c ---
	/* Excitation vector */
	Word16 old_exc[STRUCTS_L_frame +
				   STRUCTS_pit_max +
				   STRUCTS_L_inter];
	Word16* exc;					

	/* Spectral expansion factors */
	Word16 F_gamma3[STRUCTS_p];
	Word16 F_gamma4[STRUCTS_p];

	/* Lsp (Line spectral pairs in the cosine domain) */
	Word16 lspold[STRUCTS_p];
	Word16 lspnew[STRUCTS_p];

	/* Filter's memory */
	Word16 mem_syn[STRUCTS_p];

	/* Global definition */
	Word16 last_ener_cod;
	Word16 last_ener_pit;

// sdec_tet.c ---	
	/* Default parameters */
	Word16 old_parm[STRUCTS_parm_size];
	Word16 old_T0;

// scod_tet.c ---
	/* Speech vector */
	Word16 old_speech[STRUCTS_L_total];
	Word16* speech;
	Word16* p_window;
	Word16* new_speech;	/* Global variable */

	/* Weighted speech vector */
	Word16 old_wsp[STRUCTS_L_frame + 
				   STRUCTS_pit_max];
	Word16* wsp;

	/* All - zero vector */
	Word16 ai_zero[STRUCTS_L_subfr + 
				   STRUCTS_pp1];
	Word16* zero;

	/* Spectral expansion factors */
	Word16 F_gamma1[STRUCTS_p];
	Word16 F_gamma2[STRUCTS_p];
		
	/* Lsp (Line Spectral Pairs in the cosine domain) */
	Word16 lspnew_q[STRUCTS_p];
	Word16 lspold_q[STRUCTS_p];	

	/* Filters memories */
	Word16 mem_w0[STRUCTS_p];
	Word16 mem_w[STRUCTS_p];

	/* Matrix rr[dim_rr][dim_rr] */
	Word16 rr[STRUCTS_dim_rr][STRUCTS_dim_rr];
};


void init_tetra_op(tetra_op_t* top);

#ifdef __cplusplus
 }
 #endif 

#endif	// __C_STRUCTS_H__
