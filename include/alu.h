#ifndef INC_ALU_H
# define INC_ALU_H

/*
 * ALU uses 4 types of functions and typedefs
 * Anything starting with alub_ is for bit managment
 * Anything starting with alup_ is for ptr managment
 * Anything starting with alur_ is for alu_t register managment
 * Anything starting with alu_ is intended for developer usage
 * 
 * The core of this library focuses as much as possible on using the alup_*
 * variety of functions, the FPN variety has optimisation constraints and will
 * take a while (if at all) to appear in the alup_* set, alur_* and alu_* on
 * the other hand will be available when the 1st stable release of this library
 * is marked
*/

#include "alu/_.h"
#include "alu/alum.h"
#include "alu/aluv.h"
#include "alu/alup.h"
#include "alu/alur.h"

/** @brief IDs of constants that must be available to the ALU
 * @note These registers will always be overwritten with the expected
 * values before internal math handlers are used **/
typedef enum ALU_REG_ID {
	/* Holds booleans that state if number is active */
	ALU_REG_ID_BOOL,
	/* All Numbers */
	ALU_REG_ID_ZERO,
	/* Unsigned Integers */
	ALU_REG_ID_UMAX,
	/* Signed Integers */
	ALU_REG_ID_IMIN,
	ALU_REG_ID_IMAX,
	/* Floating Point Numbers */
	ALU_REG_ID_NINF, /* Negative Infinity */
	ALU_REG_ID_PINF, /* Positive Infinity */
	ALU_REG_ID__NAN, /* Not a Number */
	/* Minimum registers for ALU to allocate */
	ALU_REG_ID_NEED
} ALU_REG_ID_t;

size_t alu_lowest_upto( alur_t NUM, alur_t VAL );

#define alu_check1( alu, num ) \
	IFTRUE( !alu_get_active( alu, num ), EADDRNOTAVAIL )
	
#define alu_check2( alu, num, val ) \
	( alu_check1( alu, num ) | alu_check1( alu, val ) )

#define alu_check3( alu, num, val, reg ) \
	( alu_check2( alu, num, val ) | alu_check1( alu, reg ) )

#define alur_check1( alu, alur ) \
	alu_check1( alu, (alur).node )

#define alur_check2( alu, num, val ) \
	alu_check2( alu, (num).node, (val).node )

#define alur_check3( alu, num, val, reg ) \
	alu_check3( alu, (num).node, (val).node, (reg).node )

typedef int (*alu_func_rdChar32_t)( char32_t *dst, void *src, long *nextpos );
typedef int (*alu_func_wrChar32_t)( char32_t src, void *dst );
typedef void (*alu_func_flipstr_t)( void *dst );

typedef struct alu_src
{
	void *src;
	alu_func_rdChar32_t next;
	long *nextpos;
} alu_src_t;

typedef struct alu_dst
{
	void *dst;
	alu_func_wrChar32_t next;
	alu_func_flipstr_t flip;
} alu_dst_t;

int_t alu_set_flag( alu_t *alu, alur_t *reg, uint_t info );
int_t alu_rem_flag( alu_t *alu, alur_t *reg, uint_t info );

#define alur_getSeg( alu, alur, seg ) \
	(((uchar_t*)(alur_data( (alu), alur )))[seg])

#define alur_getBit( alu, alur, bit ) \
	(alur_getSeg( alu, alur, (bit) / CHAR_BIT ) \
	& (1u << ((bit) % CHAR_BIT)))

#define alu_set_used( alu, count ) \
	do \
	{ \
		alu_used(alu) = count; \
		(alu)->block.taken = count * alu_Nsize(alu); \
	} \
	while ( 0 )

void alu_set_constants( alu_t *alu );

alub_t alu_end_bit( alu_t *alu, uint_t num );

# define ALU_BASE_STR_0to9 "0123456789"
# define ALU_BASE_STR_atoz "abcdefghijklmnopqrstuvwxyz"
# define ALU_BASE_STR_AtoZ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
# define ALU_BASE_STR_0toZtoz \
	ALU_BASE_STR_0to9 ALU_BASE_STR_AtoZ ALU_BASE_STR_atoz
# define ALU_BASE_STR_0toztoZ \
	ALU_BASE_STR_0to9 ALU_BASE_STR_atoz ALU_BASE_STR_AtoZ

enum
{
	ALU_BASE_NUM = 0,
	ALU_BASE_VAL,
	ALU_BASE_REM,
	ALU_BASE_TMP,
	ALU_BASE_COUNT
};

typedef struct alu_base
{
	bool lowercase;
	size_t base;
	char32_t digsep;
} alu_base_t;

enum
{
	ALU_LIT_NUM = 0,
	ALU_LIT_BASE,
	ALU_LIT_VAL,
	ALU_LIT_TMP,
	ALU_LIT_DOT,
	ALU_LIT_ONE,
	ALU_LIT_EXP,
	ALU_LIT_EXP_BIAS,
	ALU_LIT_MAN,
	ALU_LIT_COUNT
};

int_t alu_str2reg( alu_t *alu, alu_src_t src, alur_t dst, alu_base_t base );
int_t alu_str2uint( alu_t *alu, alu_src_t src, alu_uint_t dst, alu_base_t base );
int_t alu_str2int( alu_t *alu, alu_src_t src, alu_int_t dst, alu_base_t base );
int_t alu_str2fpn( alu_t *alu, alu_src_t src, alu_fpn_t dst, alu_base_t base );

int_t alu_lit2reg
(
	alu_t *alu,
	alu_src_t src,
	alur_t dst,
	alu_base_t base
);

int_t alur2str( alu_t *alu, alu_dst_t dst, alur_t src, alu_base_t base );
int_t alu_uint2str( alu_t *alu, alu_dst_t dst, alu_uint_t src, alu_base_t base );
int_t alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base );
int_t alu_fpn2str( alu_t *alu, alu_dst_t dst, alu_fpn_t src, alu_base_t base );

int_t alu_uint2fpn( alu_t *alu, alu_uint_t *val );
int_t alu_int2fpn( alu_t *alu, alu_int_t *val );
int_t alu_fpn2uint( alu_t *alu, alu_fpn_t *val );
int_t alu_fpn2int( alu_t *alu, alu_fpn_t *val );

int_t alu_cmp( alu_t *alu, uint_t num, uint_t val );

bool alur_is_zero( alu_t *alu, alur_t reg, alub_t *end_bit );

int_t alu_mov( alu_t *alu, uint_t num, uint_t val, uint_t tmp );
int_t alu_set( alu_t *alu, uint_t num, bool fillwith );
#define alu_set_nil( alu, num ) alu_set( alu, num, 0 )
#define alu_set_max( alu, num ) alu_set( alu, num, 1 )


int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info );
int_t alu_get_raw( alu_t *alu, uint_t num, uintmax_t *raw );

int_t alu__op1
(
	alu_t *alu
	, uint_t num
	, uint_t info
	, func_alur_op1_t op1
);

int_t alu__op2
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t info
	, func_alur_op2_t op2
);

int_t alu__op3
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t reg
	, uint_t tmp
	, uint_t info
	, func_alur_op4_t op4
);

int_t alu__shift
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t tmp
	, func_alur__shift_t _shift
	, func_alur_shift_t shift
);

int_t alu___shift
(
	alu_t *alu
	, uint_t num
	, uint_t tmp
	, size_t bits
	, func_alur__shift_t _shift
);

#define alu_cmp( alu, num, val ) \
	alu__op2( alu, num, val, alur_cmp )

#define alu_not( alu, num ) \
	alu__op1( alu, num, alur_not )

#define alu__or( alu, num, val ) \
	alu__op2( alu, num, val, alur__or )
#define alu_xor( alu, num, val ) \
	alu__op2( alu, num, val, alur_xor )
#define alu_and( alu, num, val ) \
	alu__op2( alu, num, val, alur_and )
	
#define alu_shl( alu, num, val ) \
	alu__shift( alu, num, val, alur__shl, alur__shift )
#define alu_shr( alu, num, val ) \
	alu__shift( alu, num, val, alur__shr, alur__shift )
#define alu_rol( alu, num, val ) \
	alu__shift( alu, num, val, alur__rol, alur__rotate )
#define alu_ror( alu, num, val ) \
	alu__shift( alu, num, val, alur__ror, alur__rotate )

#define alu__shl( alu, num, bits ) alu___shift( alu, num, bits, alur__shl )
#define alu__shr( alu, num, bits ) alu___shift( alu, num, bits, alur__shr )
#define alu__rol( alu, num, bits ) alu___shift( alu, num, bits, alur__rol )
#define alu__ror( alu, num, bits ) alu___shift( alu, num, bits, alur__ror )
	
#define alu_neg( alu, num ) \
	alu__op1( alu, num, alur_neg )

#define alu_inc( alu, num ) \
	alu__op1( alu, num, alur_inc )
#define alu_add( alu, num, val ) \
	alu__op2( alu, num, val, alur_add )
#define alu_mul( alu, num, val ) \
	alu__op2( alu, num, val, alur_mul )
	
#define alu_dec( alu, num ) \
	alu__op1( alu, num, alur_inc )
#define alu_sub( alu, num, val ) \
	alu__op2( alu, num, val, alur_sub )
#define alu_div( alu, num, val ) \
	alu__op2( alu, num, val, alur_div )
#define alu_rem( alu, num, val ) \
	alu__op2( alu, num, val, alur_rem )
#define alu_divide( alu, num, val, reg ) \
	alu__op3( alu, num, val, reg, alur__div )

#define alu_uint_set_raw( alu, num, raw ) alu_set_raw( alu, num, raw, 0  )
#define alu_uint_get_raw( alu, num, raw ) alu_get_raw( alu, num, raw  )

#define alu__uint_op1( alu, num, op1 ) \
	alu__op1( alu, num, 0, op1 )

#define alu__uint_op2( alu, num, val, op2 ) \
	alu__op2( alu, num, val, 0, op2 )

#define alu__uint_op4( alu, num, val, reg, tmp, op4 ) \
	alu__op4( alu, num, val, reg, tmp, 0, op4 )
	
#define alu__uint__shift alu___shift
#define alu__uint_shift alu__shift

#define alu_uint_cmp( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_cmp )

#define alu_uint_not( alu, num ) \
	alu__uint_op1( alu, num, alur_not )

#define alu_uint__or( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur__or )
#define alu_uint_xor( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_xor )
#define alu_uint_and( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_and )

#define alu_uint_shl( alu, num, val ) \
	alu__uint_shift( alu, num, val, alur__shl, alur__shift )
#define alu_uint_shr( alu, num, val ) \
	alu__uint_shift( alu, num, val, alur__shr, alur__shift )
#define alu_uint_rol( alu, num, val ) \
	alu__uint_shift( alu, num, val, alur__rol, alur__rotate )
#define alu_uint_ror( alu, num, val ) \
	alu__uint_shift( alu, num, val, alur__ror, alur__rotate )
	
#define alu_uint_neg( alu, num ) \
	alu__uint_op1( alu, num, alur_neg )

#define alu_uint_inc( alu, num ) \
	alu__uint_op1( alu, num, alur_inc )
#define alu_uint_add( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_add )
#define alu_uint_mul( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_mul )
	
#define alu_uint_dec( alu, num ) \
	alu__uint_op1( alu, num, alur_dec )
#define alu_uint_sub( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_sub )
#define alu_uint_div( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_div )
#define alu_uint_rem( alu, num, val ) \
	alu__uint_op2( alu, num, val, alur_rem )
#define alu_uint_divide( alu, num, val, reg ) \
	alu__uint_op3( alu, num, val, reg, alur__div )

int_t alu_int_set_raw( alu_t *alu, alu_int_t num, intmax_t val );
int_t alu_int_get_raw( alu_t *alu, alu_int_t num, intmax_t *val );

#define alu__int_op1( alu, num, op1 ) \
	alu__op1( alu, num, 0, op1 )

#define alu__int_op2( alu, num, val, op2 ) \
	alu__op2( alu, num, val, 0, op2 )

#define alu__int_op4( alu, num, val, reg, tmp, op4 ) \
	alu__op4( alu, num, val, reg, tmp, 0, op4 )

int_t alu_int__shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, alu_int_t tmp
	, func_alur__shift_t _shift
	, func_alur_shift_t shift
);

int_t alu_int___shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t tmp
	, size_t bits
	, func_alur__shift_t _shift
);

#define alu_int_cmp( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_cmp )

#define alu_int_not( alu, num ) \
	alu__int_op1( alu, num, alur_not )

#define alu_int__or( alu, num, val ) \
	alu__int_op2( alu, num, val, alur__or )
#define alu_int_xor( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_xor )
#define alu_int_and( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_and )

#define alu_int_shl( alu, num, val ) \
	alu__int_shift( alu, num, val, true, alur__shift )
#define alu_int_shr( alu, num, val ) \
	alu__int_shift( alu, num, val, false, alur__shift )
#define alu_int_rol( alu, num, val ) \
	alu__int_shift( alu, num, val, true, alur__rotate )
#define alu_int_ror( alu, num, val ) \
	alu__int_shift( alu, num, val, false, alur__rotate )
	
#define alu_int_neg( alu, num ) \
	alu__int_op1( alu, num, alur_neg )

#define alu_int_inc( alu, num ) \
	alu__int_op1( alu, num, alur_inc )
#define alu_int_add( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_add )
#define alu_int_mul( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_mul )
	
#define alu_int_dec( alu, num ) \
	alu__int_op1( alu, num, alur_dec )
#define alu_int_sub( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_sub )
#define alu_int_div( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_div )
#define alu_int_rem( alu, num, val ) \
	alu__int_op2( alu, num, val, alur_rem )
#define alu_int_divide( alu, num, val, reg ) \
	alu__int_op3( alu, num, val, reg, alur__div )

/* Deals with +/-, +/-INF and NaN */
int_t alu_fpn_cmp(
	alu_t *alu,
	alu_fpn_t num,
	alu_fpn_t val,
	int *cmp,
	size_t *bit
);

/* Passes math differently where needed */
int_t alu_fpn_inc( alu_t *alu, alu_fpn_t num );
int_t alu_fpn_dec( alu_t *alu, alu_fpn_t num );
int_t alu_fpn_add( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int_t alu_fpn_sub( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int_t alu_fpn_mul( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int_t alu_fpn_divide( alu_t *alu, alu_fpn_t num, alu_fpn_t val, alu_fpn_t rem );
int_t alu_fpn_div( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int_t alu_fpn_rem( alu_t *alu, alu_fpn_t num, alu_fpn_t val );

#endif /* INC_ALU_H */
