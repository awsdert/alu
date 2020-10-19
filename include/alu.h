#ifndef INC_ALU_H
# define INC_ALU_H

#include <errno.h>
#include <uchar.h>
#include <float.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <unic/limits.h>
#include <unic/stdint.h>

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

# define alu__printf( FORMAT, THEFILE, THELINE, THEFUNC, ... ) \
	fprintf \
	( \
		stderr, "%s:%u: %s() " FORMAT "\n" \
		, THEFILE, THELINE, THEFUNC, __VA_ARGS__ \
	)

# define _INT2STR(VAL) #VAL
# define INT2STR(VAL) _INT2STR(VAL)

# define alu_printf( FORMAT, ... ) \
	alu__printf( FORMAT, __FILE__, __LINE__, __func__, __VA_ARGS__ )

# define alu_puts( MSG ) \
	alu_printf( "'%s'", MSG )

# define alu_error( ERRNO ) \
	alu_printf( "Error 0x%08X %i '%s'", ERRNO, ERRNO, strerror(ERRNO) )

/* Use boolean of 1 or 0 to either use or cancel a value,
 * also use | to use result in same value in the case A == B
 * */
#define LOWEST( A, B )  ( ((A) * ((A) <= (B))) | ((B) * ((B) <= (A))) )
#define HIGHEST( A, B ) ( ((A) * ((A) >= (B))) | ((B) * ((B) >= (A))) )

typedef struct alu_block {
	int fault;
	void *block;
	size_t given, taken;
} alu_block_t;

/** @brief malloc/realloc/free wrapper
 * @param mem Where you want allocations to go
 * @param want How many bytes you want (0 to free)
 * @param dir Which direction to restrain allocation to
 * 	positive value = expansion only -
 * 		will not shrink memory if wanted amount is smaller than current,
 * 		will return success in that case. Will also clear any memory,
 * 		between that wanted memory end point and the end point of
 * 		current memory
 * 	negative value = shrinkage only -
 * 		will fail if wanted amount is larger than current
 * 	0 = normal realloc behaviour
 * @return 0 on success, errno.h value on failure
 * **/
void* alu_block( alu_block_t *mem, size_t want, int_t dir );
# define alu_block_expand( MEM, WANT ) alu_block( MEM, WANT, 1 )
# define alu_block_shrink( MEM, WANT ) alu_block( MEM, WANT, -1 )
# define alu_block_release( MEM ) (void)alu_block( MEM, 0, -1 )

/** @brief Holds various values for bit position in a number
 * @param seg Index number of size_t sized data segments
 * @param bit Overall bit from 0 to alur.upto
 * @param pos Position of bit from 0 to bitsof(size_t) - 1
 * @param ptr Segment that is bit is in
 * @param mask Mask needed to reference specific bit
**/
typedef struct alub {
	uintmax_t seg, bit, pos, *ptr, mask;
} alub_t;

void alu_print_bit( char *pfx, alub_t pos, bool getbit );

/** @brief sets bit position by bit count
 * @param ptr Base pointer that bit 0 starts from
 * @param pos Bit index to calculate position from
 * **/
alub_t alub( uintmax_t *ptr, size_t bit );
bool alu_get_bit( uintmax_t *ptr, size_t bit );
void alu_set_bit( uintmax_t *ptr, size_t bit, bool val );
/** @brief sets bit position by byte count
 * @param ptr Base pointer that byte 0 starts from
 * @param pos Byte index to calculate position from
 * **/
#define alu_byte( ptr, byte ) alub( ptr, byte * CHAR_BIT )

/** @brief increments bit position
 * @param pos object holding various values needed for bit position
 * **/
void alub_inc( alub_t *pos );
/** @brief decrements bit position
 * @param pos object holding various values needed for bit position
 * **/
void alub_dec( alub_t *pos );

typedef struct alu_vec
{
	size_t Nsize;
	uint_t taken, given;
	alu_block_t block;
	void **ref;
} alu_vec_t;

typedef uint_t alu_int_t, alu_uint_t, alu_fpn_t;

void* alu_vec( alu_vec_t *vec, uint_t want, size_t Nsize, int dir );
# define alu_vec_expand( VEC, WANT, PERN ) alu_vec( VEC, WANT, PERN, 1 )
# define alu_vec_shrink( VEC, WANT, PERN ) alu_vec( VEC, WANT, PERN, -1 )
# define alu_vec_release( VEC, PERN ) (void)alu_vec( VEC, 0, PERN, -1 )

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

# define _BIT_FROM_END( MIN, POS )\
	(((((MIN)>>(POS))^-1)<<1) & ((MIN)>>(POS)))
# define BIT_FROM_END( NEG_MIN, POS ) _BIT_FROM_END( NEG_MIN, POS )

# define ALU_INFO__SIGN 1
# define ALU_INFO_FLOAT 2

typedef struct alur
{
	uint_t node;
	uint_t info;
	size_t from, upto;
} alur_t;

typedef struct alup
{
	void *data;
	uint_t info;
	size_t from, upto;
} alup_t;

/** @brief Block of RAM based registers for optimal calculations
 * @note Reduces the need for RAM to load different nodes when the CPU
 * requests the blocks to peform software based math on.
 * Each thread MUST have it's own ALU to use, cannot be shared at this
 * time. **/
typedef alu_vec_t alu_t;

#define IFTRUE( CMP, VAL ) ( !!(CMP) * (VAL) )

#define EITHER( CMP, ONTRUE, ONFALSE ) \
	( IFTRUE( CMP, ONTRUE ) | IFTRUE( !(CMP), ONFALSE ) )

#define IFBOTH( CMP1, CMP2 ) ( !!(CMP1) & !!(CMP2) )

size_t alu_man_dig( size_t bits );

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
	
int_t alur_get_exponent( alu_t *alu, alur_t SRC, size_t *exp );
size_t alur_get_exponent_bias( alur_t SRC );

size_t alu_lowest_upto( alur_t num, alur_t val );

#define alu_Nsize( alu ) ((alu)->Nsize)
#define alu_Nbits( alu ) (alu_Nsize(alu) * CHAR_BIT)
#define alu_nodes( alu ) ((uchar_t*)((alu)->block.block))
#define alu_data( alu, reg ) (alu_nodes(alu) + ((reg) * alu_Nsize(alu)))
#define alu_valid( alu ) alu_data( alu, 0 )
#define alu_upto( alu ) ((alu)->given)
#define alu_used( alu ) ((alu)->taken)

#define alu_errno( alu ) ((alu)->block.fault)

#define alur_data( alu, alur ) alu_data( alu, (alur).node )

#define alup_init_register( alu, alup, alur ) \
	do \
	{ \
		(alup).data = alur_data( alu, alur ); \
		(alup).info = (alur).info; \
		(alup).from = (alur).from; \
		(alup).upto = (alur).upto; \
	} \
	while (0)

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
		(alup).node = reg; \
		(alup).info = ALU_INFO__SIGN; \
		(alup).from = 0; \
		(alup).upto = (size) * UNIC_CHAR_BIT; \
	} \
	while (0)
	
#define alup_init_floating( alup, ptr, size ) \
	do \
	{ \
		(alup).node = ptr; \
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

#define alu_get_active( alu, reg ) alu_get_bit( (void*)alu_valid(alu), reg )
#define alu_clr_active( alu, reg ) alu_set_bit( (void*)alu_valid(alu), reg, 0 )
#define alu_set_active( alu, reg ) alu_set_bit( (void*)alu_valid(alu), reg, 1 )
#define alur_get_active( alu, alur ) alu_get_active( alu, (alur).node )
#define alur_clr_active( alu, alur ) alu_clr_active( alu, (alur).node )
#define alur_set_active( alu, alur ) alu_set_active( alu, (alur).node )

#define alup___signed( alup ) !!((alup).info & ALU_INFO__SIGN)
#define alup_floating( alup ) !!((alup).info & ALU_INFO_FLOAT)

#define alur___signed( alur ) !!((alur).info & ALU_INFO__SIGN)
#define alur_floating( alur ) !!((alur).info & ALU_INFO_FLOAT)

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

int_t alu__err_null_ptr
(
	char const * const file
	, uint_t line
	, char const * const func
	, char const * const name
);
int_t alu__err_range
(
	char const * const file
	, uint_t line
	, char const * const func
	, char const * const name
);

#define alu_err_null_ptr( name ) \
	alu__err_null_ptr( __FILE__, __LINE__, __func__, name )

int_t alu_set_flag( alu_t *alu, alur_t *reg, uint_t info );
int_t alu_rem_flag( alu_t *alu, alur_t *reg, uint_t info );

#define alur_getSeg( alu, alur, seg ) \
	(((uchar_t*)(alur_data( (alu), alur )))[seg])

#define alur_getBit( alu, alur, bit ) \
	(alur_getSeg( alu, alur, (bit) / CHAR_BIT ) \
	& (1u << ((bit) % CHAR_BIT)))
	
bool_t alup_below0( alup_t _PTR );
bool_t alur_below0( alu_t *alu, alur_t REG );

#define alu_set_used( alu, count ) \
	do \
	{ \
		alu_used(alu) = count; \
		(alu)->block.taken = count * alu_Nsize(alu); \
	} \
	while ( 0 )
int_t alu_setup_reg( alu_t *alu, uint_t want, size_t Nsize );
void alu__print_reg
(
	char *file
	, uint_t line
	, const char *func
	, char *name
	, alu_t *alu
	, alur_t reg
	, bool print_info
	, bool print_value
);

#define alu_print_reg( alu, reg, print_info, print_value ) \
	alu__print_reg \
	( \
		__FILE__ \
		, __LINE__ \
		, __func__ \
		, #reg \
		, alu \
		, reg \
		, print_info \
		, print_value \
	)
void alu_print_info( char *pfx, alu_t *alu, alur_t reg, uint_t flags );
size_t alu_set_bounds( alu_t *alu, alur_t *REG, size_t from, size_t upto );
void alu_set_constants( alu_t *alu );
int_t alu_update_bounds( alu_t *alu, uint_t reg );
int_t alur_update_bounds( alu_t *alu, alur_t reg );

uint_t alu_get_reg_node( alu_t *alu, size_t need );
int_t alu_get_reg_nodes( alu_t *alu, uint_t *regv, uint_t count, size_t need );

#define alu_rem_reg_node( alu, reg ) \
	do \
	{ \
		alu_clr_active( alu, *(reg) ); \
		*(reg) = 0; \
	} \
	while ( 0 )
int_t alu_rem_reg_nodes( alu_t *alu, uint_t *nodes, int count );

alub_t alur_end_bit( alu_t *alu, alur_t num );
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


int_t alur_cmp( alu_t *alu, alur_t num, alur_t val );
int_t alu_cmp( alu_t *alu, uint_t num, uint_t val );

/** @brief Copy to/from registers &/or addresses
 * @return 0 on success, errno.h code on failure
 * @note
 * 1. If num &/or val >= * ALU_REG_ID_LIMIT then they're treated as
 * an address
 * 2. If moving between a register and address then if size is higher
 * than register size it will be clamped down to the register size
 * before movement, prior to clampage if the target is not a register
 * then size is used as is to zero the target block of memory
 * 3. All movement is handled by memmove(), alu_xor() and alu__or()
 * will be utilized with a temporary register to prevent misaligned
 * values
**/
bool alur_is_zero( alu_t *alu, alur_t reg, alub_t *end_bit );

int_t alur_mov( alu_t *alu, alur_t dst, alur_t src );
int_t alu_mov( alu_t *alu, uint_t num, uint_t val );
int_t alur_set( alu_t *alu, alur_t num, bool fillwith );
int_t alu_set( alu_t *alu, uint_t num, bool fillwith );
#define alur_clr( alu, num ) alur_set( alu, num, 0 )
#define alur_set_max( alu, num ) alur_set( alu, num, 1 )
#define alu_set_nil( alu, num ) alu_set( alu, num, 0 )
#define alu_set_max( alu, num ) alu_set( alu, num, 1 )
int_t alur_set_raw
(
	alu_t *alu
	, alur_t num
	, void *raw
	, size_t size
	, uint_t info
);

int_t alur_get_raw
(
	alu_t *alu
	, alur_t num
	, void *raw
	, size_t size
);


int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info );
int_t alu_get_raw( alu_t *alu, uint_t num, uintmax_t *raw );

int_t alup__shl( alup_t _NUM, void *_tmp, size_t by );
int_t alup__shr( alup_t _NUM, void *_tmp, size_t by );
int_t alup__rol( alup_t _NUM, void *_tmp, size_t by );
int_t alup__ror( alup_t _NUM, void *_tmp, size_t by );

int_t alur__shl( alu_t *alu, alur_t num, uint_t tmp, size_t by );
int_t alur__shr( alu_t *alu, alur_t num, uint_t tmp, size_t by );
int_t alur__rol( alu_t *alu, alur_t num, uint_t tmp, size_t by );
int_t alur__ror( alu_t *alu, alur_t num, uint_t tmp, size_t by );

int_t alur_divide
(
	alu_t *alu
	, alur_t num
	, alur_t val
	, uint_t rem
	, uint_t tmp
);

typedef int_t (*func_alur_op1_t)
(
	alu_t* alu
	, alur_t num
);

typedef int_t (*func_alur_op2_t)
(
	alu_t* alu
	, alur_t num
	, alur_t val
);

typedef int_t (*func_alur_op4_t)
(
	alu_t* alu
	, alur_t num
	, alur_t val
	, uint_t reg
	, uint_t tmp
);

typedef int_t (*func_alur__shift_t)
(
	alu_t* alu
	, alur_t num
	, uint_t tmp
	, size_t bits
);

typedef int_t (*func_alur_shift_t)
(
	alu_t* alu
	, alur_t num
	, alur_t val
	, uint_t tmp
	, func_alur__shift_t _shift
);

int_t alur__shift
(
	alu_t *alu
	, alur_t num
	, alur_t val
	, uint_t tmp
	, func_alur__shift_t _shift
);
int_t alur__rotate
(
	alu_t *alu
	, alur_t num
	, alur_t val
	, uint_t tmp
	, func_alur__shift_t _shift
);

int_t alur_not( alu_t *alu, alur_t num );
int_t alur_and( alu_t *alu, alur_t num, alur_t val );
int_t alur__or( alu_t *alu, alur_t num, alur_t val );
int_t alur_xor( alu_t *alu, alur_t num, alur_t val );

# define alur_shl( ALU, NUM, VAL, TMP ) \
	alur__shift( ALU, NUM, VAL, TMP, alur__shl )
# define alur_shr( ALU, NUM, VAL, TMP ) \
	alur__shift( ALU, NUM, VAL, TMP, alur__shr )
# define alur_rol( ALU, NUM, VAL, TMP ) \
	alur__rotate( ALU, NUM, VAL, TMP, alur__rol )
# define alur_ror( ALU, NUM, VAL, TMP ) \
	alur__rotate( ALU, NUM, VAL, TMP, alur__ror )
	
int_t alur_neg( alu_t *alu, alur_t num );
int_t alur_inc( alu_t *alu, alur_t num );
int_t alur_dec( alu_t *alu, alur_t num );
int_t alur_add( alu_t *alu, alur_t num, alur_t val );
int_t alur_sub( alu_t *alu, alur_t num, alur_t val );
int_t alur_mul( alu_t *alu, alur_t num, alur_t val );
int_t alur_div( alu_t *alu, alur_t num, alur_t val );
int_t alur_rem( alu_t *alu, alur_t num, alur_t val );

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
	alu__op3( alu, num, val, reg, alur_divide )

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
	alu__uint_op3( alu, num, val, reg, alur_divide )

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
	alu__int_op3( alu, num, val, reg, alur_divide )

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
