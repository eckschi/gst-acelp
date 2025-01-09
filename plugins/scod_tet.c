/************************************************************************
*
*	FILENAME		:	scod_tet.c
*
*	DESCRIPTION		:	Main routines for speech source decoding
*
************************************************************************
*
*	SUB - ROUTINES	:	- Init_Coder_Tetra()
*						- Coder_Tetra()
*
************************************************************************
*
*	INCLUDED FILES	:	source.h
*
************************************************************************/

#include "c_source.h"

/*----------------------------------------------------------------------*
 *         Coder constant parameters.                                   *
 *                                                                      *
 *   L_window    : LPC analysis window size.                            *
 *   L_next      : Samples of next frame needed for autocor.            *
 *   L_frame     : Frame size.                                          *
 *   L_subfr     : Sub - frame size.                                      *
 *   p           : LPC order.                                           *
 *   pp1         : LPC order + 1                                          *
 *   L_total     : Total speech size.                                   *
 *   dim_rr      : Dimension of matrix rr[][].                          *
 *   pit_min     : Minimum pitch lag.                                   *
 *   pit_max     : Maximum pitch lag.                                   *
 *   L_inter     : Length of filter for interpolation                   *
 *----------------------------------------------------------------------*/

#define  L_window (Word16)STRUCTS_L_window
#define  L_next   (Word16)STRUCTS_L_next
#define  L_frame  (Word16)STRUCTS_L_frame
#define  L_subfr  (Word16)STRUCTS_L_subfr
#define  p        (Word16)STRUCTS_p
#define  pp1      (Word16)STRUCTS_pp1
#define  L_total  (Word16)STRUCTS_L_total
#define  dim_rr   (Word16)STRUCTS_dim_rr
#define  pit_min  (Word16)STRUCTS_pit_min
#define  pit_max  (Word16)STRUCTS_pit_max
#define  L_inter  (Word16)STRUCTS_L_inter

/*--------------------------------------------------------*
 *   LPC bandwidth expansion factors.                     *
 *      In Q15 = 0.95, 0.60, 0.75, 0.85                   *
 *--------------------------------------------------------*/

#define gamma1  (Word16)STRUCTS_gamma1
#define gamma2  (Word16)STRUCTS_gamma2
#define gamma3  (Word16)STRUCTS_gamma3
#define gamma4  (Word16)STRUCTS_gamma4


/**************************************************************************
*
*	ROUTINE				:	Init_Coder_Tetra
*
*	DESCRIPTION			:	Initialization of variables for the speech encoder
*
**************************************************************************
*
*	USAGE				:	Init_Coder_Tetra()
*
*	INPUT ARGUMENT(S)	:	None
*
*	OUTPUT ARGUMENT(S)	:	None
*
*	RETURNED VALUE		:	None
*
**************************************************************************/

void Init_Coder_Tetra(tetra_op_t* top)
{
	Word16 i, j;
	
	
	/*-----------------------------------------------------------------------*
	*      Initialize pointers to speech vector.                            *
	*                                                                       *
	*                                                                       *
	*   |--------------------|--------|--------|......|-------|-------|     *
	*     previous speech       sf1      sf2             sf4    L_next      *
	*                                                                       *
	*   <----------------  Total speech vector(L_total)   ----------->*
	*   |           <----  LPC analysis window(L_window)  ----------->*
	*   |           |        <---- present frame(L_frame) --->*
	*  old_speech   |        |       <----- new speech(L_frame) ----->*
	*            p_window    |       |                                      *
	*                     speech     |                                      *
	*                             new_speech                                *
	*-----------------------------------------------------------------------*/
	
	top->new_speech = top->old_speech + L_total - L_frame;	/* New speech     */
	top->speech     = top->new_speech - L_next;			/* Present frame  */
	top->p_window   = top->old_speech + L_total - L_window;	/* For LPC window */
	
	/* Initialize global variables */
	
	top->last_ener_cod = 0;
	top->last_ener_pit = 0;
	
	/* Initialize static pointers */
	
	top->wsp    = top->old_wsp + pit_max;
	top->exc    = top->old_exc + pit_max + L_inter;
	top->zero   = top->ai_zero + pp1;
	
	
	/* Static vectors to zero */
	
	for (i = 0; i < L_total; i++)
		top->old_speech[i] = 0;
	
	for (i = 0; i < pit_max + L_inter; i++)
		top->old_exc[i] = top->old_wsp[i] = 0;
	
	for (i = 0; i < p; i++)
		top->mem_syn[i] = top->mem_w[i] = top->mem_w0[i] = 0;
	
	for (i = 0; i < L_subfr; i++)
		top->zero[i] = 0;
	
	for (i = 0; i < dim_rr; i++)
		for (j = 0; j < dim_rr; j++)
			top->rr[i][j] = 0;
		
	/* Initialisation of lsp values for first */
	/* frame lsp interpolation */
	
	for (i = 0; i < p; i++)
		top->lspold_q[i] = top->lspold[i] = lspold_init[i];
	
	/* Compute LPC spectral expansion factors */
	
	Fac_Pond(gamma1, top->F_gamma1, top);
	Fac_Pond(gamma2, top->F_gamma2, top);
	Fac_Pond(gamma3, top->F_gamma3, top);
	Fac_Pond(gamma4, top->F_gamma4, top);
	
	return;
}


/**************************************************************************
*
*	ROUTINE				:	Coder_Tetra
*
*	DESCRIPTION			:	Main speech coder function
*
**************************************************************************
*
*	USAGE				:	Coder_Tetra(ana, synth)
* (Routine_Name(output1, output2))
*
*	INPUT ARGUMENT(S)	:	None
*
*	OUTPUT ARGUMENT(S)	:	
*
*		OUTPUT1			:	- Description : Analysis parameters
*							- Format : 23 * 16 bit - samples
*
*		OUTPUT2			:	- Description : Local synthesis
*							- Format : 240 * 16 bit - samples
*
*	RETURNED VALUE		:	None
*
*	COMMENTS			:	- 240 speech data should have been copied to vector
*							 new_speech[].  This vector is global and is declared in
*							  this function.
*							- Output2 is for debugging only
*
**************************************************************************/

void Coder_Tetra(Word16 ana[], Word16 synth[], tetra_op_t* top)
{
	/* LPC coefficients */
	
	Word16 r_l[pp1], r_h[pp1];	/* Autocorrelations low and high        */
	Word16 A_t[(pp1)*4];		/* A(z) unquantized for the 4 subframes */
	Word16 Aq_t[(pp1)*4];		/* A(z)   quantized for the 4 subframes */
	Word16 Ap1[pp1];		/* A(z) with spectral expansion         */
	Word16 Ap2[pp1];		/* A(z) with spectral expansion         */
	Word16 Ap3[pp1];		/* A(z) with spectral expansion         */
	Word16 Ap4[pp1];		/* A(z) with spectral expansion         */
	Word16 *A, *Aq;			/* Pointer on A_t and Aq_t              */
	
	/* Other vectors */
	
	Word16 h1[L_subfr];
	Word16 zero_h2[L_subfr + 64], *h2;
	Word16 zero_F[L_subfr + 64],  *F;
	Word16 res[L_subfr];
	Word16 xn[L_subfr];
	Word16 xn2[L_subfr];
	Word16 dn[L_subfr + 4];
	Word16 code[L_subfr + 4];
	Word16 y1[L_subfr];
	Word16 y2[L_subfr];
	
	
	
	/* Scalars */
	
	Word16 i, i_subfr;
	Word16 T0, T0_min, T0_max, T0_frac;
	Word16 gain_pit, gain_code, index;
	Word16 sign_code, shift_code;
	Word16 temp;
	Word32 L_temp;
	
	/* Initialization of F and h2 */
	
	F  = &zero_F[64];
	h2 = &zero_h2[64];
	for (i = 0; i < 64; i++)
		zero_F[i] = zero_h2[i] = 0;
	
		/*------------------------------------------------------------------------*
		*  - Perform LPC analysis:                                               *
		*       * autocorrelation + lag windowing                                *
		*       * Levinson - Durbin algorithm to find a[]                          *
		*       * convert a[] to lsp[]                                           *
		*       * quantize and code the LSPs                                     *
		*       * find the interpolated LSPs and convert to a[] for all          *
		*         subframes(both quantized and unquantized)                     *
	*------------------------------------------------------------------------*/
	
	Autocorr(top->p_window, p, r_h, r_l, top);		/* Autocorrelations */
	
	Lag_Window(p, r_h, r_l, top);			/* Lag windowing    */
	
	Levin_32(r_h, r_l, A_t, top);			/* Levinson - Durbin  */
	
	Az_Lsp(A_t, top->lspnew, top->lspold, top);		/* From A(z) to lsp */
	
	Clsp_334(top->lspnew, top->lspnew_q, ana, top);		/* Lsp quantization */
	
	ana += 3;				/* Increment analysis parameters pointer */
	
	/* Interpolation of LPC for the 4 subframes */
	
	Int_Lpc4(top->lspold,   top->lspnew,   A_t, top);
	Int_Lpc4(top->lspold_q, top->lspnew_q, Aq_t, top);
	
	/* update the LSPs for the next frame */
	
	for (i = 0; i < p; i++)
	{
		top->lspold[i]   = top->lspnew[i];
		top->lspold_q[i] = top->lspnew_q[i];
	}
	
	
	/*----------------------------------------------------------------------*
	* - Find the weighted input speech wsp[] for the whole speech frame    *
	* - Find open - loop pitch delay                                         *
	* - Set the range for searching closed - loop pitch                      *
	*----------------------------------------------------------------------*/
	
	A = A_t;
	for (i = 0; i < L_frame; i += L_subfr)
	{
		Pond_Ai(A, top->F_gamma1, Ap1, top);
		Pond_Ai(A, top->F_gamma2, Ap2, top);
		Residu(Ap1, &top->speech[i], &top->wsp[i], L_subfr, top);
		Syn_Filt(Ap2, &top->wsp[i], &top->wsp[i], L_subfr, top->mem_w, (Word16)1, top);
		A += pp1;
	}
	
	/* Find open loop pitch delay */
	
	T0 = Pitch_Ol_Dec(top->wsp, L_frame, top);
	
	/* range for closed loop pitch search */
	
	T0_min = sub(T0, (Word16)2, top);
	if (T0_min < pit_min)
		T0_min = pit_min;
	T0_max = add(T0_min, (Word16)4, top);
	if (T0_max > pit_max)
	{
		T0_max = pit_max;
		T0_min = sub(T0_max, (Word16)4, top);
	}
	
	
	/*------------------------------------------------------------------------*
	*          Loop for every subframe in the analysis frame                 *
	*------------------------------------------------------------------------*
	*  To find the pitch and innovation parameters. The subframe size is     *
	*  L_subfr and the loop is repeated L_frame/L_subfr times.               *
	*     - find the weighted LPC coefficients                               *
	*     - find the LPC residual signal res[]                               *
	*     - compute the target signal for pitch search                       *
	*     - compute impulse response of weighted synthesis filter(h1[])     *
	*     - find the closed - loop pitch parameters                            *
	*     - encode the pitch delay                                           *
	*     - update the impulse response h1[] by including fixed - gain pitch   *
	*     - find the autocorrelations of h1[](rr[][])                       *
	*     - find target vector for codebook search                           *
	*     - backward filtering of target vector                              *
	*     - codebook search                                                  *
	*     - encode codebook address                                          *
	*     - VQ of pitch and codebook gains                                   *
	*     - find synthesis speech                                            *
	*     - update states of weighting filter                                *
	*------------------------------------------------------------------------*/
	
	Aq = Aq_t;	/* pointer to interpolated quantized LPC parameters */
	
	for (i_subfr = 0;  i_subfr < L_frame; i_subfr += L_subfr)
	{
		/*---------------------------------------------------------------*
		 * Find the weighted LPC coefficients for the weighting filter.  *
		 *---------------------------------------------------------------*/
		
		Pond_Ai(Aq, top->F_gamma3, Ap3, top);
		Pond_Ai(Aq, top->F_gamma4, Ap4, top);
		
		
		/*---------------------------------------------------------------*
		 * Compute impulse response, h1[], of weighted synthesis filter  *
		 *---------------------------------------------------------------*/
		
		top->ai_zero[0] = 4096;				/* 1 in Q12 */
		for (i = 1; i <= p; i++)
			top->ai_zero[i] = 0;
		
		Syn_Filt(Ap4, top->ai_zero, h1, L_subfr, top->zero, (Word16)0, top);
		
		/*---------------------------------------------------------------*
	 	 * Compute LPC residual and copy it to exc[i_subfr]              *
		 *---------------------------------------------------------------*/
		
		Residu(Aq, &top->speech[i_subfr], res, L_subfr, top);
		
		for (i = 0; i < L_subfr; i++)
			top->exc[i_subfr + i] = res[i];
		
		/*---------------------------------------------------------------*
		 * Find the target vector for pitch search:->xn[]              *
		 *---------------------------------------------------------------*/
		
		Syn_Filt(Ap4, res, xn, L_subfr, top->mem_w0, (Word16)0, top);
		
		/*----------------------------------------------------------------------*
		 *                 Closed - loop fractional pitch search                *
		 *----------------------------------------------------------------------*
		 * The pitch range for the first subframe is divided as follows:        *
		 *   19 1/3  to   84 2/3   resolution 1/3                               *
		 *   85      to   143      resolution 1                                 *
		 *                                                                      *
		 * The period in the first subframe is encoded with 8 bits.             *
		 * For the range with fractions:                                        *
		 *   code = (T0 - 19)*3 + frac - 1;   where T0=[19..85] and frac=[-1, 0, 1] *
		 * and for the integer only range                                       *
		 *   code = (T0 - 85) + 197;        where T0=[86..143]                  *
		 *----------------------------------------------------------------------*
		 * For other subframes: if t0 is the period in the first subframe then  *
		 * T0_min = t0 - 5   and  T0_max = T0_min + 9   and  the range is given by      *
		 *      T0_min - 1 + 1/3   to  T0_max + 2/3                             *
		 *                                                                      *
		 * The period in the 2nd, 3rd, 4th subframe is encoded with 5 bits:     *
		 *  code = (T0- (T0_min - 1))*3 + frac - 1;  where T0[T0_min - 1 .. T0_max + 1] *
		 *---------------------------------------------------------------------*/
		
		T0 = Pitch_Fr(&top->exc[i_subfr], xn, h1, L_subfr, T0_min, T0_max,
						i_subfr, &T0_frac, top);
		
		if (i_subfr == 0)
		{
			/* encode pitch delay(with fraction) */
			
			if (T0 <= 85)
			{
				/* index = T0*3 - 58 + T0_frac; */
				index = add(T0, add(T0, T0, top), top);
				index = sub(index, (Word16)58, top);
				index = add(index, T0_frac, top);
			}
			else
				index = add(T0, (Word16)112, top);
			
			
			/* find T0_min and T0_max for other subframes */
			
			T0_min = sub(T0, (Word16)5, top);
			if (T0_min < pit_min)
				T0_min = pit_min;
			T0_max = add(T0_min, (Word16)9, top);
			if (T0_max > pit_max)
			{
				T0_max = pit_max;
				T0_min = sub(T0_max, (Word16)9, top);
			}
		}
		
		else						/* other subframes */
		{
			i = sub(T0, T0_min, top);
			/* index = i*3 + 2 + T0_frac;  */
			index = add(i, add(i, i, top), top);
			index = add(index, (Word16)2, top);
			index = add(index, T0_frac, top);
		}
		
		*ana++ = index;
		
		
		/*-----------------------------------------------------------------*
		 *   - find unity gain pitch excitation(adaptive codebook entry)   *
		 *     with fractional interpolation.                              *
		 *   - find filtered pitch exc. y1[] = exc[] filtered by 1/Ap4(z)  *
		 *   - compute pitch gain and limit between 0 and 1.2              *
		 *   - update target vector for codebook search                    *
		 *-----------------------------------------------------------------*/
		
		
		Pred_Lt(&top->exc[i_subfr], T0, T0_frac, L_subfr, top);
		
		Syn_Filt(Ap4, &top->exc[i_subfr], y1, L_subfr, top->zero, (Word16)0, top);
		
		gain_pit = G_Pitch(xn, y1, L_subfr, top);
		
		/* xn2[i] = xn[i] - y1[i]*gain_pit */
		
		for (i = 0; i < L_subfr; i++)
		{
			L_temp = L_mult(y1[i], gain_pit, top);
			L_temp = L_shl(L_temp, (Word16)3, top);	/* gain_pit in Q12 */
			L_temp = L_sub(Load_sh16(xn[i], top), L_temp, top);
			xn2[i] = extract_h(L_temp);
		}
		
		
		/*----------------------------------------------------------------*
		 * -Compute impulse response F[] and h2[] for innovation codebook *
		 * -Find correlations of h2[];  rr[i][j] = sum h2[n - i]*h2[n - j]*
		 *----------------------------------------------------------------*/
		
		for (i = 0; i <= p; i++)
			top->ai_zero[i] = Ap3[i];
		Syn_Filt(Ap4, top->ai_zero, F, L_subfr, top->zero, (Word16)0, top);
		
		/* Introduce pitch contribution with fixe gain of 0.8 to F[] */
		
		for (i = T0; i < L_subfr; i++)
		{
			temp = mult(F[i - T0], (Word16)26216, top);
			F[i] = add(F[i], temp, top);
		}
		
		/* Compute h2[]; ->F[] filtered by 1/Ap4(z) */
		
		Syn_Filt(Ap4, F, h2, L_subfr, top->zero, (Word16)0, top);
		
		Cal_Rr2(h2, (Word16*)top->rr, top);
		
		/*-----------------------------------------------------------------*
		 * - Backward filtering of target vector(find dn[] from xn2[])     *
		 * - Innovative codebook search(find index and gain)               *
		 *-----------------------------------------------------------------*/
		
		Back_Fil(xn2, h2, dn, L_subfr, top);	/* backward filtered target vector dn */
		
		*ana++ =D4i60_16(dn, F, h2, top->rr, code, y2, &sign_code, &shift_code, top);
		*ana++ = sign_code;
		*ana++ = shift_code;
		gain_code = G_Code(xn2, y2, L_subfr, top);
		
		/*-----------------------------------------------------------------*
		 * - Quantization of gains.                                        *
		 *-----------------------------------------------------------------*/
		
		*ana++ = Ener_Qua(Aq, &top->exc[i_subfr], code, L_subfr, &gain_pit, &gain_code, top);
		
		/*-------------------------------------------------------*
		 * - Find the total excitation                           *
		 * - Update filter memory mem_w0 for finding the target  *
		 *   vector in the next subframe.                        *
		 *   The filter state mem_w0[] is found by filtering by  *
		 *   1/Ap4(z) the error between res[i] and exc[i]        *
		 *-------------------------------------------------------*/
		
		for (i = 0; i < L_subfr;  i++)
		{
			/* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
			/* exc[i]  in Q0   gain_pit in Q12               */
			/* code[i] in Q12  gain_cod in Q0                */
			L_temp = L_mult0(top->exc[i + i_subfr], gain_pit);
			L_temp = L_mac0(L_temp, code[i], gain_code, top);
			top->exc[i + i_subfr] = (Word16)L_shr_r(L_temp, (Word16)12, top);
		}
		
		for (i = 0; i < L_subfr; i++)
			res[i] = sub(res[i], top->exc[i_subfr + i], top);
		
		Syn_Filt(Ap4, res, code, L_subfr, top->mem_w0, (Word16)1, top);
		
		/* Note: we use vector code[] as output only as temporary vector */
		
		/*-------------------------------------------------------*
		 * - find synthesis speech corresponding to exc[]        *
		 *   This filter is to help debug only.                  *
		 *-------------------------------------------------------*/
		
		Syn_Filt(Aq, &top->exc[i_subfr], &synth[i_subfr], L_subfr, 
				 top->mem_syn, (Word16)1, top);
		
		Aq += pp1;
	}
  
	/*--------------------------------------------------*
	 * Update signal for next frame.                    *
	 *->shift to the left by L_frame:                   *
	 *     speech[], wsp[] and  exc[]                   *
	 *--------------------------------------------------*/

	for (i = 0; i < L_total - L_frame; i++)
	  top->old_speech[i] = top->old_speech[i + L_frame];

	for (i = 0; i < pit_max; i++)
	  top->old_wsp[i] = top->old_wsp[i + L_frame];

	for (i = 0; i < pit_max + L_inter; i++)
	  top->old_exc[i] = top->old_exc[i + L_frame];

	return;
}

