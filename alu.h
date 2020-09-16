#ifndef INC_ALU_H
# define INC_ALU_H

#include <fbstdint.h>
#include <errno.h>
#include <uchar.h>
#include <float.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
# define alu__printf( FORMAT, THEFILE, THELINE, THEFUNC, ... ) \
	fprintf \
	( \
		stderr, "%s:%u: %s() " FORMAT "\n" \
		, THEFILE, THELINE, THEFUNC, __VA_ARGS__ \
	)

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

typedef struct alu_bytes {
	size_t upto, used, last;
} alu_bytes_t;

typedef struct alu_count {
	uint_t upto, used, last;
} alu_count_t;

typedef struct alu_block {
	alu_bytes_t bytes;
	void *block;
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
int_t alu_block( struct alu_block *mem, size_t want, int_t dir );
# define alu_block_expand( MEM, WANT ) alu_block( MEM, WANT, 1 )
# define alu_block_shrink( MEM, WANT ) alu_block( MEM, WANT, -1 )
# define alu_block_release( MEM ) (void)alu_block( MEM, 0, -1 )

typedef struct alu_bit {
	size_t s, b, p, *S, B;
} alu_bit_t;

void alu_print_bit( char *pfx, alu_bit_t pos, bool getbit );

/** @brief sets bit position by bit count
 * @param pos object holding various values needed for bit position
 * **/
alu_bit_t alu_bit_set_bit
(
	size_t *init,
	size_t bit
);

/** @brief sets bit position by byte count
 * @param pos object holding various values needed for bit position
 * **/
alu_bit_t alu_bit_set_byte
(
	size_t *init,
	size_t byte
);

/** @brief increments bit position
 * @param pos object holding various values needed for bit position
 * **/
alu_bit_t alu_bit_inc( alu_bit_t pos );
/** @brief decrements bit position
 * @param pos object holding various values needed for bit position
 * **/
alu_bit_t alu_bit_dec( alu_bit_t pos );

typedef struct alu_vec
{
	size_t perN;
	alu_count_t qty;
	alu_block_t mem;
	void **ref;
} alu_vec_t;

typedef uint_t alu_int_t, alu_uint_t, alu_fpn_t;

int_t alu_vec( alu_vec_t *vec, uint_t want, size_t perN, int dir );
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

#if defined( _SC_PAGESIZE )
#define ALU_REG_ID_LIMIT sysconf(_SC_PAGESIZE)
#elif defined( SCHAR_MAX )
#define ALU_REG_ID_LIMIT SCHAR_MAX
#else
#define ALU_REG_ID_LIMIT INT_MAX
#endif

# define _BIT_FROM_END( MIN, POS )\
	(((((MIN)>>(POS))^-1)<<1) & ((MIN)>>(POS)))
# define BIT_FROM_END( NEG_MIN, POS ) _BIT_FROM_END( NEG_MIN, POS )

# define ALU_INFO__SIGN 1
# define ALU_INFO_FLOAT 2

typedef struct alu_reg
{
	uint_t node;
	uint_t info;
	size_t from, upto, mant;
} alu_reg_t;

/** @brief Block of RAM based registers for optimal calculations
 * @note Reduces the need for RAM to load different nodes when the CPU
 * requests the blocks to peform software based math on.
 * Each thread MUST have it's own ALU to use, cannot be shared at this
 * time. **/
typedef alu_vec_t alu_t;

#define alu_reg_init( alu, alu_reg, reg, inf ) \
	do \
	{ \
		(alu_reg).node = reg; \
		(alu_reg).info = inf; \
		(alu_reg).mant = 0; \
		(alu_reg).from = 0; \
		(alu_reg).upto = alu_size_perN( alu ) * CHAR_BIT; \
	} \
	while (0)

size_t alu_lowest_upto( alu_reg_t num, alu_reg_t val );

#define SET1IF( CMP, VAL ) ( (VAL) * !!(CMP) )

#define SET2IF( CMP, ONTRUE, ONFALSE ) \
	( SET1IF( CMP, ONTRUE ) | SET1IF( !(CMP), ONFALSE ) )

#define TRUEIF( CMP1, CMP2 ) ( !!(CMP1) & !!(CMP2) )

#define alu_size_perN( alu ) ((alu)->perN)
#define alu_bits_perN( alu ) (alu_size_perN(alu) * CHAR_BIT)
#define alu_nodes( alu ) ((uchar_t*)((alu)->mem.block))
#define alu_valid( alu ) ((bool*)((alu)->mem.block))
#define alu_upto( alu ) ((alu)->qty.upto)
#define alu_used( alu ) ((alu)->qty.used)

#define alu_data( alu, reg ) (alu_nodes(alu) + ((reg) * alu_size_perN(alu)))
#define alu_active( alu, reg ) (alu_valid(alu)[reg])

#define alu_reg_data( alu, alu_reg ) alu_data( alu, (alu_reg).node )
#define alu_reg_active( alu, alu_reg ) alu_active( alu, (alu_reg).node )
#define alu_reg_signed( alu_reg ) !!((alu_reg).info & ALU_INFO__SIGN)
#define alu_reg_floating( alu_reg ) !!((alu_reg).info & ALU_INFO_FLOAT)

#define alu_check1( alu, num ) \
	SET1IF( !alu_active( alu, num ), EADDRNOTAVAIL )
	
#define alu_check2( alu, num, val ) \
	( alu_check1( alu, num ) | alu_check1( alu, val ) )

#define alu_check3( alu, num, val, reg ) \
	( alu_check2( alu, num, val ) | alu_check1( alu, reg ) )

#define alu_reg_check1( alu, alu_reg ) \
	alu_check1( alu, (alu_reg).node )

#define alu_reg_check2( alu, num, val ) \
	alu_check2( alu, (num).node, (val).node )

#define alu_reg_check3( alu, num, val, reg ) \
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

int_t alu_set_flag( alu_t *alu, alu_reg_t *reg, uint_t info );
int_t alu_rem_flag( alu_t *alu, alu_reg_t *reg, uint_t info );

#define alu_reg_getSeg( alu, alu_reg, seg ) \
	(((uchar_t*)(alu_reg_data( (alu), alu_reg )))[seg])

#define alu_reg_getBit( alu, alu_reg, bit ) \
	(alu_reg_getSeg( alu, alu_reg, (bit) / CHAR_BIT ) \
	& (1u << ((bit) % CHAR_BIT)))
	
#define alu_reg_below0( alu, alu_reg ) \
	TRUEIF( \
		alu_reg_signed( alu_reg ) \
		, alu_reg_getBit( alu, alu_reg, (alu_reg).upto - 1) \
	)

#define alu_set_used( alu, count ) \
	do \
	{ \
		alu_used(alu) = count; \
		(alu)->mem.bytes.used = count * alu_size_perN(alu); \
	} \
	while ( 0 )
int_t alu_setup_reg( alu_t *alu, uint_t want, uint_t used, size_t perN );
void alu_print_reg( char *pfx, alu_t *alu, alu_reg_t reg, bool print_info, bool print_value );
void alu_print_info( char *pfx, alu_t *alu, alu_reg_t reg, uint_t flags );
size_t alu_set_bounds( alu_t *alu, alu_reg_t *REG, size_t from, size_t upto );
void alu_set_constants( alu_t *alu );
int_t alu_update_bounds( alu_t *alu, uint_t reg );
int_t alu_reg_update_bounds( alu_t *alu, alu_reg_t reg );

int_t alu_get_reg_node( alu_t *alu, uint_t *reg, size_t need );
int_t alu_get_reg_nodes( alu_t *alu, uint_t *regv, uint_t count, size_t need );

#define alu_rem_reg_node( alu, reg ) \
	do \
	{ \
		alu_active( alu, *(reg) ) = false; \
		*(reg) = 0; \
	} \
	while ( 0 )
int_t alu_rem_reg_nodes( alu_t *alu, uint_t *nodes, int count );

alu_bit_t alu_reg_end_bit( alu_t *alu, alu_reg_t num );
alu_bit_t alu_end_bit( alu_t *alu, uint_t num );

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

int_t alu_str2reg( alu_t *alu, alu_src_t src, alu_reg_t dst, alu_base_t base );
int_t alu_str2uint( alu_t *alu, alu_src_t src, alu_uint_t dst, alu_base_t base );
int_t alu_str2int( alu_t *alu, alu_src_t src, alu_int_t dst, alu_base_t base );
int_t alu_str2fpn( alu_t *alu, alu_src_t src, alu_fpn_t dst, alu_base_t base );

int_t alu_lit2reg
(
	alu_t *alu,
	alu_src_t src,
	alu_reg_t dst,
	alu_base_t base
);

int_t alu_reg2str( alu_t *alu, alu_dst_t dst, alu_reg_t src, alu_base_t base );
int_t alu_uint2str( alu_t *alu, alu_dst_t dst, alu_uint_t src, alu_base_t base );
int_t alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base );
int_t alu_fpn2str( alu_t *alu, alu_dst_t dst, alu_fpn_t src, alu_base_t base );

int_t alu_uint2fpn( alu_t *alu, alu_uint_t *val );
int_t alu_int2fpn( alu_t *alu, alu_int_t *val );
int_t alu_fpn2uint( alu_t *alu, alu_fpn_t *val );
int_t alu_fpn2int( alu_t *alu, alu_fpn_t *val );


int_t alu_reg_cmp(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, size_t *bit
);

int_t alu_cmp(
	alu_t *alu
	, uint_t num
	, uint_t val
	, size_t *bit
);

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
bool alu_reg_is_zero( alu_t *alu, alu_reg_t reg, alu_bit_t *end_bit );

int_t alu_mov( alu_t *alu, uintptr_t num, uintptr_t val );
int_t alu_reg_copy( alu_t *alu, alu_reg_t dst, alu_reg_t src );
int_t alu_copy( alu_t *alu, uint_t num, uint_t val );
int_t alu_reg_fill( alu_t *alu, alu_reg_t num, bool fillwith );
int_t alu_fill( alu_t *alu, uint_t num, bool fillwith );
#define alu_reg_set_nil( alu, num ) alu_reg_fill( alu, num, 0 )
#define alu_reg_set_max( alu, num ) alu_reg_fill( alu, num, 1 )
#define alu_set_nil( alu, num ) alu_fill( alu, num, 0 )
#define alu_set_max( alu, num ) alu_fill( alu, num, 1 )
int_t alu_reg_set_raw
(
	alu_t *alu
	, alu_reg_t num
	, void *raw
	, uint_t info
	, size_t size
);

int_t alu_reg_get_raw
(
	alu_t *alu
	, alu_reg_t num
	, void *raw
	, size_t size
);

int_t alu_set_raw( alu_t *alu, uint_t num, size_t raw, uint_t info );
int_t alu_get_raw( alu_t *alu, uint_t num, size_t *raw );

int_t alu_reg__shl( alu_t *alu, alu_reg_t num, size_t by );
int_t alu_reg__shr( alu_t *alu, alu_reg_t num, size_t by );
int_t alu_reg__shift
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
	, bool left
);
int_t alu_reg__rotate
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
	, bool left
);

int_t alu_reg_not( alu_t *alu, alu_reg_t num );
int_t alu_reg_and( alu_t *alu, alu_reg_t num, alu_reg_t val );
int_t alu_reg__or( alu_t *alu, alu_reg_t num, alu_reg_t val );
int_t alu_reg_xor( alu_t *alu, alu_reg_t num, alu_reg_t val );

# define alu_reg_shl( ALU, NUM, VAL, TMP ) \
	alu_reg__shift( ALU, NUM, VAL, TMP, true )
# define alu_reg_shr( ALU, NUM, VAL, TMP ) \
	alu_reg__shift( ALU, NUM, VAL, TMP, false )
# define alu_reg_rol( ALU, NUM, VAL, TMP ) \
	alu_reg__rotate( ALU, NUM, VAL, TMP, true )
# define alu_reg_ror( ALU, NUM, VAL, TMP ) \
	alu_reg__rotate( ALU, NUM, VAL, TMP, false )

int_t alu_reg_neg( alu_t *alu, alu_reg_t num );
int_t alu_reg_inc( alu_t *alu, alu_reg_t num );
int_t alu_reg_dec( alu_t *alu, alu_reg_t num );
int_t alu_reg_add( alu_t *alu, alu_reg_t num, alu_reg_t val );
int_t alu_reg_sub( alu_t *alu, alu_reg_t num, alu_reg_t val );
int_t alu_reg_mul( alu_t *alu, alu_reg_t num, alu_reg_t val, alu_reg_t tmp );
int_t alu_reg_divide( alu_t *alu, alu_reg_t num, alu_reg_t val, alu_reg_t rem );
int_t alu_reg_div( alu_t *alu, alu_reg_t num, alu_reg_t val, alu_reg_t tmp );
int_t alu_reg_rem( alu_t *alu, alu_reg_t num, alu_reg_t val, alu_reg_t tmp );


int_t alu__shift( alu_t *alu, uint_t num, uint_t val, uint_t tmp, bool left );
int_t alu__shl( alu_t *alu, uint_t num, size_t by );
int_t alu__shr( alu_t *alu, uint_t num, size_t by );
int_t alu__rol( alu_t *alu, uint_t num, uint_t tmp, size_t by );
int_t alu__ror( alu_t *alu, uint_t num, uint_t tmp, size_t by );
int_t alu__rotate( alu_t *alu, uint_t num, uint_t val, uint_t tmp, bool left );

int_t alu_not( alu_t *alu, uint_t num );
int_t alu_and( alu_t *alu, uint_t num, uint_t val );
int_t alu__or( alu_t *alu, uint_t num, uint_t val );
int_t alu_xor( alu_t *alu, uint_t num, uint_t val );
# define alu_shl( ALU, NUM, VAL, TMP ) alu__shift( ALU, NUM, VAL, TMP, true )
# define alu_shr( ALU, NUM, VAL, TMP ) alu__shift( ALU, NUM, VAL, TMP, false )
# define alu_rol( ALU, NUM, VAL, TMP ) alu__rotate( ALU, NUM, VAL, TMP, true )
# define alu_ror( ALU, NUM, VAL, TMP ) alu__rotate( ALU, NUM, VAL, TMP, false )

int_t alu_neg( alu_t *alu, uint_t num );
int_t alu_inc( alu_t *alu, uint_t num );
int_t alu_dec( alu_t *alu, uint_t num );
int_t alu_add( alu_t *alu, uint_t num, uint_t val );
int_t alu_sub( alu_t *alu, uint_t num, uint_t val );
int_t alu_mul( alu_t *alu, uint_t num, uint_t val, uint_t tmp );
int_t alu_divide( alu_t *alu, uint_t num, uint_t val, uint_t rem );
int_t alu_div( alu_t *alu, uint_t num, uint_t val, uint_t tmp );
int_t alu_rem( alu_t *alu, uint_t num, uint_t val, uint_t tmp );

int_t alu_uint_cmp( alu_t *alu, alu_uint_t num, alu_uint_t val, size_t *bit );

/* Clamps between ALU_REG_ID_ZERO & ALU_REG_ID_UMAX */
int_t alu_uint_not( alu_t *alu, alu_uint_t num );
int_t alu_uint_and( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint__or( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_xor( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_shl( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_shr( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_rol( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_ror( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_neg( alu_t *alu, alu_uint_t num );
int_t alu_uint_inc( alu_t *alu, alu_uint_t num );
int_t alu_uint_dec( alu_t *alu, alu_uint_t num );
int_t alu_uint_add( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_sub( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_mul( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_divide( alu_t *alu, alu_uint_t num, alu_uint_t val, alu_uint_t rem );
int_t alu_uint_div( alu_t *alu, alu_uint_t num, alu_uint_t val );
int_t alu_uint_rem( alu_t *alu, alu_uint_t num, alu_uint_t val );

int_t alu_int_cmp( alu_t *alu, alu_int_t num, alu_int_t val, size_t *bit );

/* Clamps between ALU_REG_ID_IMIN & ALU_REG_ID_IMAX */

int_t alu_int_not( alu_t *alu, alu_int_t num );
int_t alu_int_and( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int__or( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_xor( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_shl( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_shr( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_rol( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_ror( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_neg( alu_t *alu, alu_int_t num );
int_t alu_int_inc( alu_t *alu, alu_int_t num );
int_t alu_int_dec( alu_t *alu, alu_int_t num );
int_t alu_int_add( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_sub( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_mul( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_divide( alu_t *alu, alu_int_t num, alu_int_t val, alu_int_t rem );
int_t alu_int_div( alu_t *alu, alu_int_t num, alu_int_t val );
int_t alu_int_rem( alu_t *alu, alu_int_t num, alu_int_t val );

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
