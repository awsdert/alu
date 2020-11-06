#ifndef INC_ALU__H
# define INC_ALU__H

# include <errno.h>
# include <uchar.h>
# include <float.h>
# include <ctype.h>
# include <string.h>
# include <stdio.h>
# include <unic/limits.h>
# include <unic/stdint.h>

#ifndef aluout
#define aluout stdout
#endif

# define alu__printf( FORMAT, THEFILE, THELINE, THEFUNC, ... ) \
	fprintf \
	( \
		aluout, "%s:%u: %s() " FORMAT "\n" \
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
 * also use | to use result in same value in the case A == B */
# define LOWEST( A, B )  ( ((A) * ((A) <= (B))) | ((B) * ((B) <= (A))) )
# define HIGHEST( A, B ) ( ((A) * ((A) >= (B))) | ((B) * ((B) >= (A))) )
# define BITS2SIZE( BITS ) \
	(((BITS) / UNIC_CHAR_BIT) + !!((BITS) % UNIC_CHAR_BIT))

#define IFTRUE( CMP, VAL ) ( !!(CMP) * (VAL) )

#define EITHER( CMP, ONTRUE, ONFALSE ) \
	( IFTRUE( CMP, ONTRUE ) | IFTRUE( !(CMP), ONFALSE ) )

#define IFBOTH( CMP1, CMP2 ) ( !!(CMP1) & !!(CMP2) )

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

#endif
