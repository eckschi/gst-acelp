/************************************************************************
*
*	FILENAME		:	sdec_tet.c
*
*	DESCRIPTION		:	Main routines for speech source decoding
*
************************************************************************
*
*	SUB-ROUTINES	:	- Init_Decod_Tetra()
*					- Decod_Tetra()
*
************************************************************************
*
*	INCLUDED FILES	:	source.h
*
************************************************************************/

#include "c_source.h"

/*--------------------------------------------------------*
 *       Decoder constants parameters.                    *
 *                                                        *
 *   L_frame     : Frame size.                            *
 *   L_subfr     : Sub-frame size.                        *
 *   p           : LPC order.                             *
 *   pp1         : LPC order+1                            *
 *   pit_min     : Minimum pitch lag.                     *
 *   pit_max     : Maximum pitch lag.                     *
 *   L_inter     : Length of filter for interpolation     *
 *   parm_size   : Lenght of vector parm[]                *
 *--------------------------------------------------------*/

#define  L_frame  (Word16)STRUCTS_L_frame
#define  L_subfr  (Word16)STRUCTS_L_subfr
#define  p        (Word16)STRUCTS_p
#define  pp1      (Word16)STRUCTS_pp1
#define  pit_min  (Word16)STRUCTS_pit_min
#define  pit_max  (Word16)STRUCTS_pit_max
#define  L_inter  (Word16)STRUCTS_L_inter
#define  parm_size (Word16)STRUCTS_parm_size


/*--------------------------------------------------------*
 *   LPC bandwidth expansion factors for noise filter.    *
 *      In Q15 = 0.75, 0.85                               *
 *--------------------------------------------------------*/

#define gamma3  (Word16)STRUCTS_gamma3
#define gamma4  (Word16)STRUCTS_gamma4


/**************************************************************************
*
*	ROUTINE				:	Init_Decod_Tetra
*
*	DESCRIPTION			:	Initialization of variables for the speech decoder
*
**************************************************************************
*
*	USAGE				:	Init_Decod_Tetra()
*
*	INPUT ARGUMENT(S)		:	None
*
*	OUTPUT ARGUMENT(S)		:	None
*
*	RETURNED VALUE		:	None
*
**************************************************************************/

void Init_Decod_Tetra(tetra_op_t* top)
{
  Word16 i;

  top->old_T0 = 60;
  for(i=0; i<23; i++)
     top->old_parm[i] = 0;

  /* Initialize static pointer */

  top->exc = top->old_exc + pit_max + L_inter;

  /* Initialize global variables */

  top->last_ener_cod = 0;
  top->last_ener_pit = 0;
  
  /* Static vectors to zero */

  for(i=0; i<pit_max + L_inter; i++)
    top->old_exc[i] = 0;

  for(i=0; i<p; i++)
    top->mem_syn[i] = 0;


  /* Initialisation of lsp values for first */
  /* frame lsp interpolation */

  for(i=0; i<p; i++)
    top->lspold[i] = lspold_init[i];


  /* Compute LPC spectral expansion factors */

  Fac_Pond(gamma3, top->F_gamma3, top);
  Fac_Pond(gamma4, top->F_gamma4, top);

 return;
}


/**************************************************************************
*
*	ROUTINE				:	Decod_Tetra
*
*	DESCRIPTION			:	Main speech decoder function
*
**************************************************************************
*
*	USAGE				:	Decod_Tetra(parm,synth)
*							(Routine_Name(input1,output1))
*
*	INPUT ARGUMENT(S)		:	
*
*		INPUT1			:	- Description : Synthesis parameters
*							- Format : 24 * 16 bit-samples
*
*	OUTPUT ARGUMENT(S)		:	
*
*		OUTPUT1			:	- Description : Synthesis
*							- Format : 240 * 16 bit-samples
*
*	RETURNED VALUE		:	None
*
**************************************************************************/

void Decod_Tetra(Word16 parm[], Word16 synth[], tetra_op_t* top)
{
  /* LPC coefficients */

  Word16 A_t[(pp1)*4];		/* A(z) unquantized for the 4 subframes */
  Word16 Ap3[pp1];		/* A(z) with spectral expansion         */
  Word16 Ap4[pp1];		/* A(z) with spectral expansion         */
  Word16 *A;			/* Pointer on A_t                       */

  /* Other vectors */

  Word16 zero_F[L_subfr+64],  *F;
  Word16 code[L_subfr+4];

  /* Scalars */

  Word16 i, i_subfr;
  Word16 T0, T0_min, T0_max, T0_frac;
  Word16 gain_pit, gain_code, index;
  Word16 sign_code, shift_code;
  Word16 bfi, temp;
  Word32 L_temp;

  /* Initialization of F */

  F  = &zero_F[64];
  for(i=0; i<64; i++)
   zero_F[i] = 0;

  /* Test bfi */

  bfi = *parm++;

  if(bfi == 0)
  {
    D_Lsp334(&parm[0], top->lspnew, top->lspold, top);	/* lsp decoding   */

    for(i=0; i< parm_size; i++)		/* keep parm[] as old_parm */
      top->old_parm[i] = parm[i];
  }
  else
  {
    for(i=1; i<p; i++)
      top->lspnew[i] = top->lspold[i];

    for(i=0; i< parm_size; i++)		/* use old parm[] */
      parm[i] = top->old_parm[i];
  }

  parm += 3;			/* Advance synthesis parameters pointer */

  /* Interpolation of LPC for the 4 subframes */

  Int_Lpc4(top->lspold,   top->lspnew,   A_t, top);

  /* update the LSPs for the next frame */

  for(i=0; i<p; i++)
    top->lspold[i]   = top->lspnew[i];

/*------------------------------------------------------------------------*
 *          Loop for every subframe in the analysis frame                 *
 *------------------------------------------------------------------------*
 * The subframe size is L_subfr and the loop is repeated L_frame/L_subfr  *
 *  times                                                                 *
 *     - decode the pitch delay                                           *
 *     - decode algebraic code                                            *
 *     - decode pitch and codebook gains                                  *
 *     - find the excitation and compute synthesis speech                 *
 *------------------------------------------------------------------------*/

  A = A_t;				/* pointer to interpolated LPC parameters */

  for (i_subfr = 0; i_subfr < L_frame; i_subfr += L_subfr)
  {

    index = *parm++;				/* pitch index */

    if (i_subfr == 0)				/* if first subframe */
    {
      if (bfi == 0)
      {						/* if bfi == 0 decode pitch */
         if (index < 197)
         {
           /* T0 = (index+2)/3 + 19; T0_frac = index - T0*3 + 58; */

           i = add(index, (Word16)2, top);
           i = mult(i, (Word16)10923, top);	/* 10923 = 1/3 in Q15 */
           T0 = add(i, (Word16)19, top);

           i = add(T0, add(T0, T0, top), top);	/* T0*3 */
           i = sub((Word16)58, (Word16)i, top);
           T0_frac = add(index, (Word16)i, top);
         }
         else
         {
           T0 = sub(index, (Word16)112, top);
           T0_frac = 0;
         }
      }
      else   /* bfi == 1 */
      {
        T0 = top->old_T0;
        T0_frac = 0;
      }


      /* find T0_min and T0_max for other subframes */

      T0_min = sub(T0, (Word16)5, top);
      if (T0_min < pit_min) T0_min = pit_min;
      T0_max = add(T0_min, (Word16)9, top);
      if (T0_max > pit_max)
      {
        T0_max = pit_max;
        T0_min = sub(T0_max, (Word16)9, top);
      }
    }

    else  /* other subframes */

    {
      if (bfi == 0)				/* if bfi == 0 decode pitch */
      {
         /* T0 = (index+2)/3 - 1 + T0_min; */

         i = add(index, (Word16)2, top);
         i = mult(i, (Word16)10923, top);	/* 10923 = 1/3 in Q15 */
         i = sub(i, (Word16)1, top);
         T0 = add(T0_min, i, top);

         /* T0_frac = index - 2 - i*3; */

         i = add(i, add(i,i, top), top);		/* i*3 */
         T0_frac = sub( index , add(i, (Word16)2, top) , top);
      }
    }

   /*-------------------------------------------------*
    * - Find the adaptive codebook vector.            *
    *-------------------------------------------------*/

    Pred_Lt(&top->exc[i_subfr], T0, T0_frac, L_subfr, top);

   /*-----------------------------------------------------*
    * - Compute noise filter F[].                         *
    * - Decode codebook sign and index.                   *
    * - Find the algebraic codeword.                      *
    *-----------------------------------------------------*/

    Pond_Ai(A, top->F_gamma3, Ap3, top);
    Pond_Ai(A, top->F_gamma4, Ap4, top);

    for (i = 0;   i <= p;      i++) F[i] = Ap3[i];
    for (i = pp1; i < L_subfr; i++) F[i] = 0;

    Syn_Filt(Ap4, F, F, L_subfr, &F[pp1], (Word16)0, top);

    /* Introduce pitch contribution with fixed gain of 0.8 to F[] */

    for (i = T0; i < L_subfr; i++)
    {
      temp = mult(F[i-T0], (Word16)26216, top);
      F[i] = add(F[i], temp, top);
    }

    index = *parm++;
    sign_code  = *parm++;
    shift_code = *parm++;

    D_D4i60(index, sign_code, shift_code, F, code, top);


   /*-------------------------------------------------*
    * - Decode pitch and codebook gains.              *
    *-------------------------------------------------*/

    index = *parm++;        /* index of energy VQ */

    Dec_Ener(index,bfi,A,&top->exc[i_subfr],code, L_subfr, &gain_pit, &gain_code, top);

   /*-------------------------------------------------------*
    * - Find the total excitation.                          *
    * - Find synthesis speech corresponding to exc[].       *
    *-------------------------------------------------------*/

    for (i = 0; i < L_subfr;  i++)
    {
      /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
      /* exc[i]  in Q0   gain_pit in Q12               */
      /* code[i] in Q12  gain_cod in Q0                */

      L_temp = L_mult0(top->exc[i+i_subfr], gain_pit);
      L_temp = L_mac0(L_temp, code[i], gain_code, top);
      top->exc[i+i_subfr] = (Word16)L_shr_r(L_temp, (Word16)12, top);
    }

    Syn_Filt(A, &top->exc[i_subfr], &synth[i_subfr], L_subfr, top->mem_syn, (Word16)1, top);

    A  += pp1;    /* interpolated LPC parameters for next subframe */
  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_frame  exc[]           *
  *--------------------------------------------------*/

  for(i=0; i<pit_max+L_inter; i++)
    top->old_exc[i] = top->old_exc[i+L_frame];

  top->old_T0 = T0;

  return;
}

