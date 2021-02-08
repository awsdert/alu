#ifndef INC_ALUP_H
# define INC_ALUP_H

# include "alub.h"

# define ALU_INFO__SIGN 1
# define ALU_INFO_FLOAT 2

size_t alu_man_dig( size_t bits );

typedef struct alup
{
	/* Root pointer for the number (useful for sub math like addition in
	 * multiplication) */
	void *	data;
	/* Position from data */
	size_t	from, last, upto;
	/* Total bits in the number */
	size_t	bits;
	/* 0 means an integer */
	size_t	mdig;
	/* Must be set for floating values, at least for now */
	bool_t	sign;
} alup_t;

#define alup_bit( alup, bit ) alub( (alup)->data, bit )
#define alup___signed( alup ) !!((alup)->sign)
#define alup_floating( alup ) !!((alup)->mdig)
#define alup_until_pos( alup ) ((alup)->upto)
#define alup_until_bit( alup ) alup_bit( alup, alup_until_pos( alup ) )
#define alup_final_pos( alup ) ((alup)->last)
#define alup_final_bit( alup ) alup_bit( alup, alup_final_pos( alup ) )
#define alup_first_pos( alup ) ((alup)->from)
#define alup_first_bit( alup ) alup_bit( alup, alup_first_pos( alup ) )
#define alup_get_sign( alup ) \
	((alup)->sign && alub_get_val( alup_final_bit( alup ) ))
#define alup_set_sign( alup, neg ) \
	do \
	{ \
		if ( (alup)->sign ) \
			alub_set_val( alup_final_bit(alup), neg ); \
	} \
	while (0)

void alup__print
(
	char *file
	, uint_t line
	, const char *func
	, char *name
	, alup_t const * const _PTR
	, bool print_info
	, bool print_value
);

#define alup_print( alup, print_info, print_value ) \
	alup__print \
	( \
		__FILE__ \
		, __LINE__ \
		, __func__ \
		, #alup \
		, alup \
		, print_info \
		, print_value \
	)

#define alup_init_unsigned( alup, DATA, BITS ) \
	do \
	{ \
		(alup).data = DATA; \
		(alup).sign = false; \
		(alup).upto = (alup).bits = BITS; \
		(alup).last = (alup).upto - 1; \
		(alup).from = 0; \
		(alup).mdig = 0; \
	} \
	while (0)
	
#define alup_init___signed( alup, DATA, BITS ) \
	do \
	{ \
		(alup).data = DATA; \
		(alup).sign = true; \
		(alup).upto = (alup).bits = BITS; \
		(alup).last = (alup).upto - 1; \
		(alup).from = 0; \
		(alup).mdig = 0; \
	} \
	while (0)
	
#define alup_init_floating( alup, DATA, BITS ) \
	do \
	{ \
		(alup).data = DATA; \
		(alup).sign = true; \
		(alup).upto = (alup).bits = BITS; \
		(alup).last = (alup).upto - 1; \
		(alup).from = 0; \
		(alup).mdig = alu_man_dig( (alup).bits ); \
	} \
	while (0)

#define alup_init_mantissa( SRC, MAN ) \
	do \
	{ \
		(MAN).data = (SRC)->data; \
		(MAN).sign = false; \
		(MAN).bits = (SRC)->mdig  ? (SRC)->mdig - 1 : 0; \
		(MAN).upto = (SRC)->from + (MAN).bits; \
		(MAN).last = (MAN).upto - 1; \
		(MAN).from = 0; \
		(MAN).mdig = 0; \
	} \
	while (0)
	
#define alup_init_exponent( SRC, EXP ) \
	do \
	{ \
		(EXP).data = (SRC)->data; \
		(EXP).sign = false; \
		(EXP).bits = (SRC)->mdig ? (SRC)->bits - \
			(((SRC)->mdig) + !((SRC)->sign)) : 0; \
		(EXP).upto = alup_until_pos(SRC) - !!((SRC)->sign); \
		(EXP).last = (EXP).upto - 1; \
		(EXP).from = ((SRC)->from + (SRC)->mdig) - 1; \
		(EXP).mdig = 0; \
	} \
	while (0)

alub_t	alup_first_bit_with_val( alup_t const * const _SRC, bool val );
alub_t	alup_final_bit_with_val( alup_t const * const _SRC, bool val );
size_t	alup_get_exponent( alup_t const * const _SRC );
size_t	alup_get_exponent_bias( alup_t const * const _SRC );
int_t	alup_set_exponent( alup_t const * const _SRC, size_t exp );
int_t	alup_set( alup_t const * const _DST, bool fillwith );

int_t	alup_mov_int2int( alup_t const * const _DST, alup_t const * const _SRC );
int_t	alup_mov_int2flt( alup_t const * const _DST, alup_t const * const _SRC );
int_t	alup_mov_flt2flt( alup_t const * const _DST, alup_t const * const _SRC );
int_t	alup_mov_flt2int( alup_t const * const _DST, alup_t const * const _SRC );
int_t	alup_mov( alup_t const * const _DST, alup_t const * const _SRC );

int_t	alup_cmp_int2int( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_cmp( alup_t const * const _NUM, alup_t const * const _VAL );

int_t	alup__shl_int2int( alup_t const * const _NUM, size_t by );
int_t	alup__shr_int2int( alup_t const * const _NUM, size_t by );
int_t	alup__rol_int2int( alup_t const * const _NUM, size_t by );
int_t	alup__ror_int2int( alup_t const * const _NUM, size_t by );

int_t	alup__rol( alup_t const * const _NUM, void *_tmp, size_t by );
int_t	alup__ror( alup_t const * const _NUM, void *_tmp, size_t by );

int_t	alup_not( alup_t const * const _NUM );
int_t	alup_and( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup__or( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_xor( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_shl( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_shr( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_rol( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_ror( alup_t const * const _NUM, alup_t const * const _VAL );

int_t	alup_neg( alup_t const * const _NUM );
int_t	alup_inc( alup_t const * const _NUM );
int_t	alup_dec( alup_t const * const _NUM );
int_t	alup_match_exponents( void *_num, void *_val, size_t size );

int_t	alup__add_int2int( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup__add( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp );

int_t	alup__sub_int2int( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup__sub( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp );

int_t	alup__mul_int2int( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy );
int_t	alup__mul( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp );
int_t	alup__div_int2int( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy );
int_t	alup__div( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp );

#endif
