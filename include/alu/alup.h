#ifndef INC_ALUP_H
# define INC_ALUP_H

# include "alub.h"

# define ALU_INFO__SIGN 1
# define ALU_INFO_FLOAT 2

size_t alu_man_dig( size_t bits );

typedef struct alup
{
	void *	data;
	size_t	from;
	/* Total bits in the number */
	size_t	leng;
	/* 0 means an integer */
	size_t	mant;
	/* Must be set for floating values */
	bool_t	sign;
} alup_t;

#define alup___signed( alup ) !!((alup)->sign)
#define alup_floating( alup ) !!((alup)->mant)
#define alup_until_pos( alup ) ((alup)->from + (alup)->leng + 1)
#define alup_until_bit( alup ) alub( (alup)->data, alup_until_pos( alup ) )
#define alup_final_pos( alup ) (alup_until_pos( alup ) - 1)
#define alup_final_bit( alup ) alub( (alup)->data, alup_final_pos( alup ) )
#define alup_first_bit( alup ) alub( (alup)->data, (alup)->from )
#define alup_set_sign( alup, neg ) \
	alub_set( (alup)->data, alup_until_pos(alup) - 1, neg )

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

#define alup_init_unsigned( alup, ptr, bits ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).sign = false; \
		(alup).leng = bits; \
		(alup).from = 0; \
		(alup).mant = 0; \
	} \
	while (0)
	
#define alup_init___signed( alup, ptr, bits ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).sign = true; \
		(alup).leng = bits; \
		(alup).from = 0; \
		(alup).mant = 0; \
	} \
	while (0)
	
#define alup_init_floating( alup, ptr, bits ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).sign = true; \
		(alup).leng = bits; \
		(alup).from = 0; \
		(alup).mant = alu_man_dig( (alup).leng ); \
	} \
	while (0)

#define alup_init_mantissa( SRC, MAN ) \
	do \
	{ \
		(MAN).data = (SRC)->data; \
		(MAN).sign = false; \
		(MAN).leng = IFTRUE( (SRC)->mant, ((SRC)->mant - 1) ); \
		(MAN).from = 0; \
		(MAN).mant = 0; \
	} \
	while (0)
	
#define alup_init_exponent( SRC, EXP ) \
	do \
	{ \
		(EXP).data = (SRC)->data; \
		(EXP).sign = false; \
		(EXP).leng = IFTRUE \
		( \
			(SRC)->mant \
			, ((SRC)->leng - (SRC)->mant) + !((SRC)->sign) \
		); \
		(EXP).from = IFTRUE( (SRC)->mant, ((SRC)->from + (SRC)->mant) - 1 ); \
		(EXP).mant = 0; \
	} \
	while (0)

bool_t	alup_below0( alup_t const * const _PTR );
alub_t	alup_final_one( alup_t const * const _NUM );
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

int_t	alup__shl( alup_t const * const _NUM, size_t by );
int_t	alup__shr( alup_t const * const _NUM, size_t by );
int_t	alup__rol_int2int( alup_t const * const _NUM, size_t by );
int_t	alup__rol( alup_t const * const _NUM, void *_tmp, size_t by );
int_t	alup__ror_int2int( alup_t const * const _NUM, size_t by );
int_t	alup__ror( alup_t const * const _NUM, void *_tmp, size_t by );

int_t	alup_rol( alup_t const * const _NUM, alup_t const * const _VAL, void *_tmp );

int_t	alup_not( alup_t const * const _NUM );
int_t	alup_and( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup__or( alup_t const * const _NUM, alup_t const * const _VAL );
int_t	alup_xor( alup_t const * const _NUM, alup_t const * const _VAL );

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
