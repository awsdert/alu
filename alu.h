#ifndef INC_ALU_H
# define INC_ALU_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <errno.h>
#include <uchar.h>
#include <string.h>
#include <stdio.h>
#include <fbstdint.h>

# define alu_printf( FORMAT, ... )\
	fprintf( stderr, "%s:%i: %s() " FORMAT "\n",\
		__FILE__, __LINE__, __func__, __VA_ARGS__\
	)

# define alu_puts( MSG ) \
	alu_printf( "'%s'", MSG )

# define alu_error( ERRNO ) \
	alu_printf( "Error 0x%08X %i '%s'", ERRNO, ERRNO, strerror(ERRNO) )

/* Shorthands that avoid the prefix/suffix requirements */
typedef signed char char_t;
typedef signed short int short_t;
typedef signed int int_t;
typedef signed long int long_t;
typedef signed long long int llong_t;
typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned short int ushort_t;
typedef unsigned long int ulong_t;
typedef unsigned long long int ullong_t;

typedef struct alu_bytes {
	size_t upto, used, last;
} alu_bytes_t;

typedef struct alu_count {
	int upto, used, last;
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
int alu_block( struct alu_block *mem, size_t want, int_t dir );
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
} alu_vec_t, alu_int_t, alu_uint_t, alu_fpn_t;

int alu_vec( alu_vec_t *vec, int want, size_t perN, int dir );
# define alu_vec_expand( VEC, WANT, PERN ) alu_vec( VEC, WANT, PERN, 1 )
# define alu_vec_shrink( VEC, WANT, PERN ) alu_vec( VEC, WANT, PERN, -1 )
# define alu_vec_release( VEC, PERN ) (void)alu_vec( VEC, 0, PERN, -1 )

/** @brief IDs of constants that must be available to the ALU
 * @note These registers will always be overwritten with the expected
 * values before internal math handlers are used **/
typedef enum ALU_REG_ID {
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

# define ALU_REG_F_VALID 1
# define ALU_REG_F__SIGN 2
# define ALU_REG_F_FLOAT 4

typedef struct alu_reg
{
	uint_t info;
	alu_bit_t upto, last, init;
	alu_int_t regv;
	void *part;
} alu_reg_t;

/** @brief Block of RAM based registers for optimal calculations
 * @param buff Manages the allocated memory block split into regv
 * @param count Records total available and the number used of regv
 * @param Registers the alu software math uses, always of equal size
 * @note Reduces the need for RAM to load different nodes when the CPU
 * requests the blocks to peform software based math on.
 * Each thread MUST have it's own ALU to use, cannot be shared at this
 * time. **/
typedef struct alu
{
	alu_uint_t buff, _regv;
	alu_reg_t *regv;
} alu_t;

int alu_check_reg( alu_t *alu, int num );
int alu_check1( alu_t *alu, int num );
int alu_check2( alu_t *alu, int num, int val );
int alu_check3( alu_t *alu, int num, int val, int rem );

typedef int (*alu_func_nextchar_t)( char32_t *dst, void *src, size_t *nextpos );

int alu_setup_reg( alu_t *alu, int want, size_t perN );
int alu_reset_reg( alu_t *alu, int reg, bool preserve_positions );
int alu_pri_reg( alu_t *alu, int reg );
int alu_get_reg( alu_t *alu, int *reg, size_t need );
int alu_rem_reg( alu_t *alu, int reg );

# define ALU_BASE_STR_0to9 "0123456789"
# define ALU_BASE_STR_atoz "abcdefghijklmnopqrstuvwxyz"
# define ALU_BASE_STR_AtoZ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
# define ALU_BASE_STR_0toZtoz \
	ALU_BASE_STR_0to9 ALU_BASE_STR_AtoZ ALU_BASE_STR_atoz
# define ALU_BASE_STR_0toztoZ \
	ALU_BASE_STR_0to9 ALU_BASE_STR_atoz ALU_BASE_STR_AtoZ

int alu_str2uint
(
	alu_t *alu,
	void *val,
	alu_func_nextchar_t nextchar,
	size_t *nextpos,
	size_t base,
	bool lowercase
);

int alu_str2int
(
	alu_t *alu,
	void *val,
	alu_func_nextchar_t nextchar,
	size_t *nextpos,
	size_t base,
	bool lowercase
);

int alu_str2fpn
(
	alu_t *alu,
	void *val,
	alu_func_nextchar_t nextchar,
	size_t *nextpos,
	size_t base,
	bool lowercase
);

int alu_uint2fpn( alu_t *alu, alu_uint_t *val );
int alu_int2fpn( alu_t *alu, alu_int_t *val );
int alu_fpn2uint( alu_t *alu, alu_fpn_t *val );
int alu_fpn2int( alu_t *alu, alu_fpn_t *val );

int alu_check1( alu_t *alu, int num );
int alu_check2( alu_t *alu, int num, int val );
int alu_check3( alu_t *alu, int num, int val, int rem );
int alu_cmp( alu_t *alu, int num, int val, int *cmp, size_t *bit );

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
int alu_mov( alu_t *alu, uintptr_t num, uintptr_t val );
int alu_not( alu_t *alu, int num );
int alu_and( alu_t *alu, int num, int val );
int alu__or( alu_t *alu, int num, int val );
int alu_xor( alu_t *alu, int num, int val );
int alu__shl( alu_t *alu, int num, size_t by );
int alu__shr( alu_t *alu, int num, size_t by );
int alu__shift( alu_t *alu, int num, int val, bool left );
# define alu_shl( ALU, NUM, VAL ) alu__shift( ALU, NUM, VAL, 1 )
# define alu_shr( ALU, NUM, VAL ) alu__shift( ALU, NUM, VAL, 0 )
int alu__rol( alu_t *alu, int num, size_t by );
int alu__ror( alu_t *alu, int num, size_t by );
int alu__rotate( alu_t *alu, int num, int val, bool left );
# define alu_rol( ALU, NUM, VAL ) alu__rotate( ALU, NUM, VAL, 1 )
# define alu_ror( ALU, NUM, VAL ) alu__rotate( ALU, NUM, VAL, 0 )

int alu_inc( alu_t *alu, int num );
int alu_dec( alu_t *alu, int num );
int alu_add( alu_t *alu, int num, int val );
int alu_sub( alu_t *alu, int num, int val );
int alu_mul( alu_t *alu, int num, int val );
int alu_divide( alu_t *alu, int num, int val, int rem );
int alu_div( alu_t *alu, int num, int val );
int alu_rem( alu_t *alu, int num, int val );

int alu_uint_cmp( alu_t *alu, alu_uint_t num, alu_uint_t val );

/* Clamps between ALU_REG_ID_ZERO & ALU_REG_ID_UMAX */
int alu_uint_inc( alu_t *alu, alu_uint_t num );
int alu_uint_dec( alu_t *alu, alu_uint_t num );
int alu_uint_and( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint__or( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_xor( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_shl( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_shr( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_rol( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_ror( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_add( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_sub( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_mul( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_divide( alu_t *alu, alu_uint_t num, alu_uint_t val, alu_uint_t rem );
int alu_uint_div( alu_t *alu, alu_uint_t num, alu_uint_t val );
int alu_uint_rem( alu_t *alu, alu_uint_t num, alu_uint_t val );

int alu_int_cmp( alu_t *alu, alu_int_t num, alu_int_t val );

/* Clamps between ALU_REG_ID_IMIN & ALU_REG_ID_IMAX */
int alu_int_inc( alu_t *alu, alu_int_t num );
int alu_int_dec( alu_t *alu, alu_int_t num );
int alu_int_add( alu_t *alu, alu_int_t num, alu_int_t val );
int alu_int_sub( alu_t *alu, alu_int_t num, alu_int_t val );
int alu_int_mul( alu_t *alu, alu_int_t num, alu_int_t val );
int alu_int_div( alu_t *alu, alu_int_t num, alu_int_t val, alu_int_t rem );

/* Deals with +/-, +/-INF and NaN */
int alu_fpn_cmp( alu_t *alu, alu_fpn_t num, alu_fpn_t val );

/* Passes math differently where needed */
int alu_fpn_inc( alu_t *alu, alu_fpn_t num );
int alu_fpn_dec( alu_t *alu, alu_fpn_t num );
int alu_fpn_add( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int alu_fpn_sub( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int alu_fpn_mul( alu_t *alu, alu_fpn_t num, alu_fpn_t val );
int alu_fpn_div( alu_t *alu, alu_fpn_t num, alu_fpn_t val, alu_fpn_t rem );

#endif /* INC_ALU_H */
