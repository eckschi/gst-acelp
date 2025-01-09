#ifndef	__ACELP_STRUCTS_H__
#define	__ACELP_STRUCTS_H__

//#define PRINT_ERRORS
#include <stdint.h>
#include <stdbool.h>

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

// #define MAX_32 (int32_t)0x7fffffff
// #define MIN_32 (int32_t)0x80000000

// #define MAX_16 (int16_t)0x7fff
// #define MIN_16 (int16_t)0x8000


typedef struct _CoderData CoderData;

struct _CoderData
{
  // encoder params
  int16_t mParams[24]; ///< parameters (used to be called ana as well) PRM_NO(23) plus BFI(1, only decoder)
  int Overflow;
  int16_t old_A[STRUCTS_pp + 1];		///< Last A(z) for case of unstable filter 
  int16_t lsp_old[10];					///< Clsp_334 
  int16_t y_hi;
  int16_t y_lo;
  int16_t x0;
  int16_t old_exc[STRUCTS_L_frame + STRUCTS_pit_max + STRUCTS_L_inter]; 		///< Excitation vector 
  int16_t* exc;					
  int16_t F_gamma1[STRUCTS_p];			///< Spectral expansion factor
  int16_t F_gamma2[STRUCTS_p];			///< Spectral expansion factor
  int16_t F_gamma3[STRUCTS_p]; 		///< Spectral expansion factor
  int16_t F_gamma4[STRUCTS_p];			///< Spectral expansion factor	
  int16_t lspold[STRUCTS_p];			///< Lsp (Line spectral pairs in the cosine domain) 
  int16_t lspnew[STRUCTS_p];
  int16_t mem_syn[STRUCTS_p];			///< Filter's memory 
  int16_t last_ener_cod;
  int16_t last_ener_pit;
  int16_t old_parm[STRUCTS_parm_size];
  int16_t old_T0;
  int16_t old_speech[STRUCTS_L_total];	///< speech vector
  int16_t* speech;
  int16_t* new_speech;                    /* Global variable */
  int16_t* p_window;
  int16_t old_wsp[STRUCTS_L_frame + STRUCTS_pit_max];	///< Weighted speech vector 
  int16_t* wsp;
  int16_t ai_zero[STRUCTS_L_subfr + STRUCTS_pp1];	///< All - zero vector 
  int16_t* zero;
  int16_t lspnew_q[STRUCTS_p];						///< Lsp (Line Spectral Pairs in the cosine domain) 
  int16_t lspold_q[STRUCTS_p];	
  int16_t mem_w0[STRUCTS_p];						///< Filters memories 
  int16_t mem_w[STRUCTS_p];
  int16_t rr[STRUCTS_dim_rr][STRUCTS_dim_rr];		///< Matrix rr[dim_rr][dim_rr] 
};

#endif	// __ACELP_STRUCTS_H__
