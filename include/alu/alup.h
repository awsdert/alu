#ifndef INC_ALUP_H
# define INC_ALUP_H

# include "alub.h"

# define ALU_INFO__SIGN 1
# define ALU_INFO_FLOAT 2

size_t alu_man_dig( size_t bits );

typedef struct alup
{
	void *data;
	uint_t info;
	size_t from, upto;
} alup_t;

#define alup___signed( alup ) !!((alup).info & ALU_INFO__SIGN)
#define alup_floating( alup ) !!((alup).info & ALU_INFO_FLOAT)

void alup__print
(
	char *file
	, uint_t line
	, const char *func
	, char *name
	, alup_t _PTR
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

#define alup_init_unsigned( alup, ptr, size ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).info = 0; \
		(alup).from = 0; \
		(alup).upto = (size) * UNIC_CHAR_BIT; \
	} \
	while (0)
	
#define alup_init___signed( alup, ptr, size ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).info = ALU_INFO__SIGN; \
		(alup).from = 0; \
		(alup).upto = (size) * UNIC_CHAR_BIT; \
	} \
	while (0)
	
#define alup_init_floating( alup, ptr, size ) \
	do \
	{ \
		(alup).data = ptr; \
		(alup).info = ALU_INFO_FLOAT | ALU_INFO__SIGN; \
		(alup).from = 0; \
		(alup).upto = (size) * UNIC_CHAR_BIT; \
	} \
	while (0)

#define alup_init_mantissa( NUM, MAN ) \
	do \
	{ \
		(MAN) = (NUM); \
		(MAN).info = 0; \
		(MAN).upto = (NUM).from + (alu_man_dig( (NUM).upto - (NUM).from )); \
	} \
	while (0)
	
#define alup_init_exponent( NUM, EXP ) \
	do \
	{ \
		(EXP) = (NUM); \
		(EXP).info = 0; \
		(EXP).upto--; \
		(EXP).from = (NUM).from + (alu_man_dig( (NUM).upto - (NUM).from )); \
	} \
	while (0)

bool_t	alup_below0( alup_t _PTR );
alub_t	alup_end_bit( alup_t _NUM );
size_t	alup_get_exponent( alup_t SRC );
size_t	alup_get_exponent_bias( alup_t SRC );
int_t	alup_set_exponent( alup_t SRC, size_t exp );
int_t	alup_set( alup_t _DST, bool fillwith );

int_t	alup_mov_int2int( alup_t _DST, alup_t _SRC );
int_t	alup_mov_int2flt( alup_t _DST, alup_t _SRC );
int_t	alup_mov_flt2flt( alup_t _DST, alup_t _SRC );
int_t	alup_mov_flt2int( alup_t _DST, alup_t _SRC );
int_t	alup_mov( alup_t _DST, alup_t _SRC );

int_t	alup_cmp_int2int( alup_t _NUM, alup_t _VAL );
int_t	alup_cmp( alup_t _NUM, alup_t _VAL );

int_t	alup__shl( alup_t _NUM, size_t by );
int_t	alup__shr( alup_t _NUM, size_t by );
int_t	alup__rol( alup_t _NUM, void *_tmp, size_t by );
int_t	alup__ror( alup_t _NUM, void *_tmp, size_t by );

int_t	alup_not( alup_t _NUM );
int_t	alup_and( alup_t NUM, alup_t VAL );
int_t	alup__or( alup_t NUM, alup_t VAL );
int_t	alup_xor( alup_t NUM, alup_t VAL );

int_t	alup_neg( alup_t _NUM );
int_t	alup_inc( alup_t _NUM );
int_t	alup_dec( alup_t _NUM );
int_t	alup_match_exponents( void *_num, void *_val, size_t size );

int_t	alup__add_int2int( alup_t _NUM, alup_t _VAL );
int_t	alup__add( alup_t NUM, alup_t VAL, void *_cpy, void *_tmp );

int_t	alup__sub_int2int( alup_t _NUM, alup_t _VAL );
int_t	alup__sub( alup_t NUM, alup_t VAL, void *_cpy, void *_tmp );

int_t	alup__mul_int2int( alup_t NUM, alup_t VAL, void *_cpy );
int_t	alup__mul( alup_t NUM, alup_t VAL, void *_cpy, void *_tmp );
int_t	alup__div( alup_t NUM, alup_t VAL, void *_cpy, void *_tmp );

#endif
