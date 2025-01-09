#include "stdint.h"

typedef int16_t Word16;
typedef int32_t	Word32;
typedef int32_t	Flag;
#include "c_structs.h"


/************************************************************************
*
*	DESCRIPTION		:	FUNCTION PROTOTYPES for the TETRA
*						speech source coder and decoder
*
************************************************************************/

#ifndef __C_SOURCE_H__
#define __C_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif 

/* Basic functions */

Word32 add_sh(Word32 L_var, Word16 var1, Word16 shift, tetra_op_t* top);
Word32 add_sh16(Word32 L_var, Word16 var1, tetra_op_t* top);
Word16 bin2int(Word16 no_of_bits, Word16 *bitstream, tetra_op_t* top);
Word16 bin2int_8(unsigned int no_of_bits, unsigned char *bitstream, tetra_op_t* top);
void   int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream, tetra_op_t* top);
void   int2bin_8(Word16 value, unsigned int no_of_bits, unsigned char *bitstream, tetra_op_t* top); // bigfoot
Word32 Load_sh(Word16 var1, Word16 shift, tetra_op_t* top);
Word32 Load_sh16(Word16 var1, tetra_op_t* top);
Word32 norm_v(Word32 L_var3, Word16 var1, Word16 *var2, tetra_op_t* top);
Word16 store_hi(Word32 L_var1, Word16 var2, tetra_op_t* top);
Word32 sub_sh(Word32 L_var, Word16 var1, Word16 shift, tetra_op_t* top);
Word32 sub_sh16(Word32 L_var, Word16 var1, tetra_op_t* top);

/* Extended precision functions */

Word32 div_32(Word32 L_num, Word16 hi, Word16 lo, tetra_op_t* top);
Word32 L_comp(Word16 hi, Word16 lo, tetra_op_t* top);
void   L_extract(Word32 L_32, Word16 *hi, Word16 *lo, tetra_op_t* top);
Word32 mpy_mix(Word16 hi1, Word16 lo1, Word16 lo2, tetra_op_t* top);
Word32 mpy_32(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2, tetra_op_t* top);


/* Mathematic functions  */

Word32 inv_sqrt(Word32 L_x, tetra_op_t* top);
void   Log2(Word32 L_x, Word16 *exponant, Word16 *fraction, tetra_op_t* top);
Word32 pow2(Word16 exponant, Word16 fraction, tetra_op_t* top);

/* General signal processing */

void   Autocorr(Word16 x[], Word16 p, Word16 r_h[], Word16 r_l[], tetra_op_t* top);
void   Az_Lsp(Word16 a[], Word16 lsp[], Word16 old_lsp[], tetra_op_t* top);
void   Back_Fil(Word16 x[], Word16 h[], Word16 y[], Word16 L, tetra_op_t* top);
Word16 Chebps(Word16 x, Word16 f[], Word16 n, tetra_op_t* top);
void   Convolve(Word16 x[], Word16 h[], Word16 y[], Word16 L, tetra_op_t* top);
void   Fac_Pond(Word16 gamma, Word16 fac[], tetra_op_t* top);
void   Get_Lsp_Pol(Word16 *lsp, Word32 *f, tetra_op_t* top);
void   Int_Lpc4(Word16 lsp_old[], Word16 lsp_new[], Word16 a_4[], tetra_op_t* top);
void   Lag_Window(Word16 p, Word16 r_h[], Word16 r_l[], tetra_op_t* top);
void   Levin_32(Word16 Rh[], Word16 Rl[], Word16 A[], tetra_op_t* top);
Word32 Lpc_Gain(Word16 a[], tetra_op_t* top);
void   Lsp_Az(Word16 lsp[], Word16 a[], tetra_op_t* top);
void   Pond_Ai(Word16 a[], Word16 fac[], Word16 a_exp[], tetra_op_t* top);
void   Residu(Word16 a[], Word16 x[], Word16 y[], Word16 lg, tetra_op_t* top);
void   Syn_Filt(Word16 a[], Word16 x[], Word16 y[], Word16 lg, Word16 mem[],
                Word16 update, tetra_op_t* top);

/* Specific coder functions */

void   Init_Coder_Tetra(tetra_op_t* top);
void   Coder_Tetra(Word16 ana[], Word16 synth[], tetra_op_t* top);
void   Cal_Rr2(Word16 h[], Word16 *rr, tetra_op_t* top);
void   Clsp_334(Word16 *lsp, Word16 *lsp_q, Word16 *indice, tetra_op_t* top);
Word16 D4i60_16(Word16 dn[], Word16 f[], Word16 h[], Word16 rr[][32],
                Word16 cod[], Word16 y[], Word16 *sign, Word16 *shift_code, tetra_op_t* top);
Word16 Ener_Qua(Word16 A[], Word16 prd_lt[], Word16 code[], Word16 L_subfr,
                Word16 *gain_pit, Word16 *gain_cod, tetra_op_t* top);
Word16 G_Code(Word16 xn2[], Word16 y2[], Word16 L_subfr, tetra_op_t* top);
Word16 G_Pitch(Word16 xn[], Word16 y1[], Word16 L_subfr, tetra_op_t* top);
void   Init_Pre_Process(tetra_op_t* top);
Word16 Lag_Max(Word16 signal[], Word16 sig_dec[], Word16 L_frame,
               Word16 lag_max, Word16 lag_min, Word16 *cor_max, tetra_op_t* top);
Word16 Pitch_Fr(Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
                Word16 t0_min, Word16 t0_max, Word16 i_subfr,
				Word16 *pit_frac, tetra_op_t* top);
Word16 Pitch_Ol_Dec(Word16 signal[], Word16 L_frame, tetra_op_t* top);
void   Pred_Lt(Word16 exc[], Word16 T0, Word16 frac, Word16 L_subfr, tetra_op_t* top);
void   Pre_Process(Word16 signal[], Word16 lg, tetra_op_t* top);
void   Prm2bits_Tetra_8(Word16 prm[], unsigned char bits[], tetra_op_t* top); 

/* Specific decoder functions */

void   Init_Decod_Tetra(tetra_op_t* top);
void   Decod_Tetra(Word16 parm[], Word16 synth[], tetra_op_t* top);
void   Bits2prm_Tetra(Word16 bits[], Word16 prm[], tetra_op_t* top);
void   Bits2prm_Tetra_8(unsigned char bits[], unsigned char bfi, Word16 prm[], tetra_op_t* top);
Word16 Dec_Ener(Word16 index, Word16 bfi, Word16 A[], Word16 prd_lt[],
				Word16 code[], Word16 L_subfr, Word16 *gain_pit, Word16 *gain_cod,
				tetra_op_t* top);
void   D_D4i60(Word16 index,Word16 sign,Word16 shift, Word16 F[], 
			   Word16 cod[], tetra_op_t* top);
void   D_Lsp334(Word16 indice[], Word16 lsp[], Word16 old_lsp[], tetra_op_t* top);
void   Post_Process(Word16 signal[], Word16 lg, tetra_op_t* top);

#endif



/************************************************************************
*
*	DESCRIPTION		:     OPERATOR PROTOTYPES for TETRA codec
*
************************************************************************/

#ifndef C_TETRA_OP_H
#define C_TETRA_OP_H


/*-----------------------*
 * Constants and Globals *
 *-----------------------*/
#define MAX_32 (Word32)0x7fffffff
#define MIN_32 (Word32)0x80000000

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

/*-----------------------*
 * Operators prototypes  *
 *-----------------------*/

Word16 abs_s(Word16 var1);                /* Short abs,           1 */
Word16 add(Word16 var1, Word16 var2, tetra_op_t* top);     /* Short add,           1 */
Word16 div_s(Word16 var1, Word16 var2, tetra_op_t* top);   /* Short division,     18 */
Word16 extract_h(Word32 L_var1);          /* Extract high,        1 */
Word16 extract_l(Word32 L_var1);          /* Extract low,         1 */
Word16 mult(Word16 var1, Word16 var2, tetra_op_t* top);    /* Short mult,          1 */
Word16 mult_r(Word16 var1, Word16 var2, tetra_op_t* top);  /* Mult with round,     2 */
Word16 negate(Word16 var1);               /* Short negate,        1 */
Word16 norm_l(Word32 L_var1);             /* Long norm,          30 */
Word16 norm_s(Word16 var1);               /* Short norm,         15 */
Word16 acelp_round(Word32 L_var1, tetra_op_t* top);              /* Round,               1 */
Word16 shl(Word16 var1, Word16 var2, tetra_op_t* top);     /* Short shift left,    1 */
Word16 shr(Word16 var1, Word16 var2, tetra_op_t* top);     /* Short shift right,   1 */
Word16 sub(Word16 var1, Word16 var2, tetra_op_t* top);     /* Short sub,           1 */

Word32 L_abs(Word32 L_var1);              /* Long abs,            3 */
Word32 L_add(Word32 L_var1, Word32 L_var2, tetra_op_t* top);  /* Long add,         2 */
Word32 L_deposit_h(Word16 var1);          /* 16 bit var1 -> MSB   2 */
Word32 L_deposit_l(Word16 var1);          /* 16 bit var1 -> LSB,  2 */
Word32 L_mac(Word32 L_var3, Word16 var1, Word16 var2, tetra_op_t* top);  /* Mac,   1 */
Word32 L_mac0(Word32 L_var3, Word16 var1, Word16 var2, tetra_op_t* top); /* no shi 1 */
Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2, tetra_op_t* top);  /* Msu,   1 */
Word32 L_msu0(Word32 L_var3, Word16 var1, Word16 var2, tetra_op_t* top); /* no shi 1 */
Word32 L_mult(Word16 var1, Word16 var2, tetra_op_t* top);  /* Long mult,           1 */
Word32 L_mult0(Word16 var1, Word16 var2); /* Long mult no shift,  1 */
Word32 L_negate(Word32 L_var1);           /* Long negate,         2 */
Word32 L_shl(Word32 L_var1, Word16 var2, tetra_op_t* top); /* Long shift left,     2 */
Word32 L_shr(Word32 L_var1, Word16 var2, tetra_op_t* top); /* Long shift right,    2 */
Word32 L_shr_r(Word32 L_var1, Word16 var2, tetra_op_t* top);  /* L_shr with round, 3 */
Word32 L_sub(Word32 L_var1, Word32 L_var2, tetra_op_t* top);  /* Long sub,         2 */

#ifdef __cplusplus
}
#endif 


#endif 



