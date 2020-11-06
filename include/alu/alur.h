#ifndef INC_ALUR_H
# define INC_ALU_H

# include "alub.h"
# include "alup.h"
# include "aluv.h"

/** @brief Block of RAM based registers for optimal calculations
 * @note Reduces the need for RAM to load different nodes when the CPU
 * requests the blocks to peform software based math on.
 * Each thread MUST have it's own ALU to use, cannot be shared at this
 * time. **/
typedef aluv_t alu_t;

typedef struct alur
{
	uint_t node;
	uint_t info;
	size_t from, upto;
} alur_t;

int_t alur_ensure( alu_t *alu, uint_t want, size_t Nsize );
#define alur__empty( alu ) aluv_release( alu, 0 )

#define alur___signed( alur ) !!((alur).info & ALU_INFO__SIGN)
#define alur_floating( alur ) !!((alur).info & ALU_INFO_FLOAT)

void alur__print
(
	char *file
	, uint_t line
	, const char *func
	, char *name
	, alu_t *alu
	, alur_t REG
	, bool print_info
	, bool print_value
);

void alur_print_flags( char *pfx, alu_t *alu, alur_t reg, uint_t flags );

#define alur_print( alu, alur, print_info, print_value ) \
	alur__print \
	( \
		__FILE__ \
		, __LINE__ \
		, __func__ \
		, #alur \
		, alu \
		, alur \
		, print_info \
		, print_value \
	)

#define alur_init_unsigned( alu, alur, reg ) \
	do \
	{ \
		(alur).node = reg; \
		(alur).info = 0; \
		(alur).from = 0; \
		(alur).upto = alu_Nbits( alu ); \
	} \
	while (0)
	
#define alur_init___signed( alu, alur, reg ) \
	do \
	{ \
		(alur).node = reg; \
		(alur).info = ALU_INFO__SIGN; \
		(alur).from = 0; \
		(alur).upto = alu_Nbits( alu ); \
	} \
	while (0)
	
#define alur_init_floating( alu, alur, reg ) \
	do \
	{ \
		(alur).node = reg; \
		(alur).info = ALU_INFO_FLOAT | ALU_INFO__SIGN; \
		(alur).from = 0; \
		(alur).upto = alu_Nbits( alu ) / 4; \
		(alur).upto -= UNIC_CHAR_BIT; \
	} \
	while (0)
	
#define alur_init_mantissa( NUM, MAN ) \
	do \
	{ \
		(MAN) = (NUM); \
		(MAN).info = 0; \
		(MAN).upto = (NUM).from + (alu_man_dig( (NUM).upto - (NUM).from )); \
	} \
	while (0)
	
#define alur_init_exponent( NUM, EXP ) \
	do \
	{ \
		(EXP) = (NUM); \
		(EXP).info = 0; \
		(EXP).upto--; \
		(EXP).from = (NUM).from + (alu_man_dig( (NUM).upto - (NUM).from )); \
	} \
	while (0)

#define alu_Nsize( alu ) ((alu)->Nsize)
#define alu_Nbits( alu ) (alu_Nsize(alu) * CHAR_BIT)
#define alu_nodes( alu ) ((uchar_t*)((alu)->block.block))
#define alu_Ndata( alu, node ) (alu_nodes(alu) + ((node) * alu_Nsize(alu)))
#define alu_valid( alu ) alu_Ndata( alu, 0 )
#define alu_upto( alu ) ((alu)->given)
#define alu_used( alu ) ((alu)->taken)
#define alu_errno( alu ) ((alu)->block.fault)

#define alur_data( alu, alur ) alu_Ndata( alu, (alur).node )

#define alu_get_active( alu, reg ) alub_get( (void*)alu_valid(alu), reg )
#define alu_clr_active( alu, reg ) alub_set( (void*)alu_valid(alu), reg, 0 )
#define alu_set_active( alu, reg ) alub_set( (void*)alu_valid(alu), reg, 1 )

#define alur_get_active( alu, alur ) alu_get_active( alu, (alur).node )
#define alur_clr_active( alu, alur ) alu_clr_active( alu, (alur).node )
#define alur_set_active( alu, alur ) alu_set_active( alu, (alur).node )

#define alup_init_register( alu, alup, alur ) \
	do \
	{ \
		(alup).data = alur_data( alu, alur ); \
		(alup).info = (alur).info; \
		(alup).from = (alur).from; \
		(alup).upto = (alur).upto; \
	} \
	while (0)

bool_t	alur_below0( alu_t *alu, alur_t REG );
alub_t	alur_final_one( alu_t *alu, alur_t NUM );
int_t	alur_set( alu_t *alu, alur_t NUM, bool fillwith );
int_t	alur_get_exponent( alu_t *alu, alur_t SRC, size_t *exp );
size_t	alur_get_exponent_bias( alur_t SRC );
int_t	alur_set_exponent( alu_t *alu, alur_t SRC, size_t exp );
uint_t	alur_get_node( alu_t *alu, size_t need );
int_t	alur_get_nodes( alu_t *alu, uint_t *regv, uint_t count, size_t need );
#define alur_rem_node( alu, node ) \
	do \
	{ \
		alu_clr_active( alu, *(node) ); \
		*(node) = 0; \
	} \
	while ( 0 )
int_t alur_rem_nodes( alu_t *alu, uint_t *nodes, int count );

int_t alur_cmp( alu_t *alu, alur_t NUM, alur_t VAL );

int_t alur_mov_int2int( alu_t *alu, alur_t DST, alur_t SRC );
int_t alur_mov_int2flt( alu_t *alu, alur_t DST, alur_t SRC );
int_t alur_mov_flt2flt( alu_t *alu, alur_t DST, alur_t SRC );
int_t alur_mov_flt2int( alu_t *alu, alur_t DST, alur_t SRC );
int_t alur_mov( alu_t *alu, alur_t DST, alur_t SRC );

int_t alur__shl( alu_t *alu, alur_t NUM, size_t by );
int_t alur__shr( alu_t *alu, alur_t NUM, size_t by );
int_t alur__rol( alu_t *alu, alur_t NUM, uint_t tmp, size_t by );
int_t alur__ror( alu_t *alu, alur_t NUM, uint_t tmp, size_t by );

int_t alur_not( alu_t *alu, alur_t NUM );
int_t alur_and( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur__or( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur_xor( alu_t *alu, alur_t NUM, alur_t VAL );

int_t alur__add( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp );
int_t alur__sub( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp );
int_t alur__div( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp );
int_t alur__mul( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp );

int_t alur_neg( alu_t *alu, alur_t NUM );
int_t alur_inc( alu_t *alu, alur_t NUM );
int_t alur_dec( alu_t *alu, alur_t NUM );
int_t alur_add( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur_sub( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur_mul( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur_div( alu_t *alu, alur_t NUM, alur_t VAL );
int_t alur_rem( alu_t *alu, alur_t NUM, alur_t VAL );

typedef int_t (*func_alur_op1_t)
(
	alu_t* alu
	, alur_t NUM
);

typedef int_t (*func_alur_op2_t)
(
	alu_t* alu
	, alur_t NUM
	, alur_t VAL
);

typedef int_t (*func_alur_op4_t)
(
	alu_t* alu
	, alur_t NUM
	, alur_t VAL
	, uint_t reg
	, uint_t tmp
);

typedef int_t (*func_alur__shift_t)
(
	alu_t* alu
	, alur_t NUM
	, size_t bits
);

int_t alur__shift
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, func_alur__shift_t _shift
);

typedef int_t (*func_alur__rotate_t)
(
	alu_t* alu
	, alur_t NUM
	, uint_t tmp
	, size_t bits
);

int_t alur__rotate
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t tmp
	, func_alur__rotate_t _rotate
);

# define alur_shl( ALU, NUM, VAL ) \
	alur__shift( ALU, NUM, VAL, alur__shl )
# define alur_shr( ALU, NUM, VAL ) \
	alur__shift( ALU, NUM, VAL, alur__shr )
# define alur_rol( ALU, NUM, VAL, TMP ) \
	alur__rotate( ALU, NUM, VAL, TMP, alur__rol )
# define alur_ror( ALU, NUM, VAL, TMP ) \
	alur__rotate( ALU, NUM, VAL, TMP, alur__ror )

#define alur_clr( alu, DST ) alur_set( alu, DST, 0 )
#define alur_set_max( alu, DST ) alur_set( alu, DST, 1 )
int_t alur_set_raw
(
	alu_t *alu
	, alur_t NUM
	, void *raw
	, size_t size
	, uint_t info
);

int_t alur_get_raw
(
	alu_t *alu
	, alur_t NUM
	, void *raw
	, size_t size
	, uint_t info
);

#endif
