#include "alu.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

# define bitsof(T) (sizeof(T) * CHAR_BIT)
# define rotate( NUM, BITS, OP1, OP2 ) \
	(((NUM) OP1 (BITS))|((NUM) OP2 (bitsof(NUM) - (BITS))))
# define rol( NUM, BITS ) rotate( NUM, BITS, <<, >> )
# define ror( NUM, BITS ) rotate( NUM, BITS, >>, << )

int uint_compare(
	alu_t *alu,
	size_t _num,
	size_t _val,
	bool print_anyways
)
{
	int ret = 0, cmp = 0, expect = 0;
	size_t N = _num, V = _val, bit = 0;
	alu_uint_t num = {0}, val = {0};
	
	num.perN = sizeof(size_t);
	num.qty.last = 0;
	num.qty.upto = num.qty.used = 1;
	num.mem.block = &N;
	num.mem.bytes.last = sizeof(size_t) - 1;
	num.mem.bytes.upto = num.mem.bytes.used = sizeof(size_t);
	
	val = num;
	val.mem.block = &V;
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	ret = alu_uint_cmp( alu, num, val, &cmp, &bit );
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"%zu vs %zu Expected %i, Got %i\n",
			_num, _val, expect, cmp
		);
	}
	
	return ret;
}

int int_compare(
	alu_t *alu,
	ssize_t _num,
	ssize_t _val,
	bool print_anyways
)
{
	int ret = 0, cmp = 0, expect = 0;
	size_t N = _num, V = _val, bit = 0;
	alu_int_t num = {0}, val = {0};
	
	num.perN = sizeof(size_t);
	num.qty.last = 0;
	num.qty.upto = num.qty.used = 1;
	num.mem.block = &N;
	num.mem.bytes.last = sizeof(size_t) - 1;
	num.mem.bytes.upto = num.mem.bytes.used = sizeof(size_t);
	
	val = num;
	val.mem.block = &V;
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	ret = alu_int_cmp( alu, num, val, &cmp, &bit );
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"%zi vs %zi Expected %i, Got %i\n",
			_num, _val, expect, cmp
		);
	}
	
	return ret;
}

int reg_compare(
	alu_t *alu,
	size_t _num,
	size_t _val,
	bool print_anyways
)
{
	int ret = 0, cmp = 0, expect = 0;
	uint_t num = -1, val = -1;
	alu_reg_t *NUM, *VAL;
	size_t *N, *V, bit = 0;
	
	ret = alu_get_reg( alu, &num, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_get_reg( alu, &val, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_rem_reg( alu, num );
		alu_error( ret );
		return ret;
	}
	
	NUM = alu->regv + num;
	VAL = alu->regv + val;
	
	N = NUM->part;
	V = VAL->part;
	
	*N = _num;
	*V = _val;
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	ret = alu_cmp( alu, num, val, &cmp, &bit );
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"0x%016zX vs 0x%016zX Expected %i, Got %i, Bit = %zu\n",
			_num, _val, expect, cmp, bit
		);
	}
	
	return ret;
}

int reg_modify(
	alu_t *alu,
	size_t _num,
	size_t _val,
	int op,
	bool print_anyways
)
{
	int ret = 0;
	uint_t num = -1, val = -1;
	alu_reg_t *NUM, *VAL;
	size_t *N, *V, expect = 0;
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	ret = alu_get_reg( alu, &num, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_get_reg( alu, &val, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_rem_reg( alu, num );
		alu_error( ret );
		return ret;
	}
	
	NUM = alu->regv + num;
	VAL = alu->regv + val;
	
	N = NUM->part;
	V = VAL->part;
	
	*N = _num;
	*V = _val;
	
#if 0
	alu_printf(
		"num = %i, val = %i, "
		"N = 0x%016zX, V = 0x%016zX, _num = 0x%016zX, _val = 0x%016zX",
		num, val, *N, *V,  _num, _val
	);
#endif
	
	expect = _num;
	switch ( op )
	{
	case 'n':
		sprintf( pfx, "-0x%016zX", _num );
		expect = -expect;
		ret = alu_neg( alu, num );
		break;
	case '~':
		sprintf( pfx, "~0x%016zX", _num );
		expect = ~expect;
		ret = alu_not( alu, num );
		break;
	case '&':
		sprintf( pfx, "0x%016zX & 0x%016zX", _num, _val );
		expect &= _val;
		ret = alu_and( alu, num, val );
		break;
	case '|':
		sprintf( pfx, "0x%016zX | 0x%016zX", _num, _val );
		expect |= _val;
		ret = alu__or( alu, num, val );
		break;
	case '^':
		sprintf( pfx, "0x%016zX ^ 0x%016zX", _num, _val );
		expect ^= _val;
		ret = alu_xor( alu, num, val );
		break;
	case 'i':
		sprintf( pfx, "0x%016zX++", _num );
		expect++;
		ret = alu_inc( alu, num );
		break;
	case 'd':
		sprintf( pfx, "0x%016zX--", _num );
		expect--;
		ret = alu_dec( alu, num );
		break;
	case '+':
		sprintf( pfx, "0x%016zX + 0x%016zX", _num, _val );
		expect += _val;
		ret = alu_add( alu, num, val );
		break;
	case '-':
		sprintf( pfx, "0x%016zX - 0x%016zX", _num, _val );
		expect -= _val;
		ret = alu_sub( alu, num, val );
		break;
	case 'l':
		sprintf( pfx, "0x%016zX <<< 0x%016zX", _num, _val );
		expect = rol( expect, _val );
		ret = alu_rol( alu, num, val );
		break;
	case '<':
		sprintf( pfx, "0x%016zX << 0x%016zX", _num, _val );
		expect <<= _val;
		ret = alu_shl( alu, num, val );
		break;
	case 'r':
		sprintf( pfx, "0x%016zX >>> 0x%016zX", _num, _val );
		expect = ror( expect, _val );
		ret = alu_ror( alu, num, val );
		break;
	case '>':
		sprintf( pfx, "0x%016zX >> 0x%016zX", _num, _val );
		expect >>= _val;
		ret = alu_shr( alu, num, val );
		break;
	case '*':
		sprintf( pfx, "0x%016zX * 0x%016zX", _num, _val );
		expect *= _val;
		ret = alu_mul( alu, num, val );
		break;
	case '/':
		sprintf( pfx, "0x%016zX / 0x%016zX", _num, _val );
		if ( _val )
			expect /= _val;
		else
			expect = 0;
		ret = alu_div( alu, num, val );
		break;
	case '%':
		sprintf( pfx, "0x%016zX %% 0x%016zX", _num, _val );
		if ( _val )
			expect %= _val;
		ret = alu_rem( alu, num, val );
		break;
	default: ret = ENOSYS;
	}
	
	/* Pointers may have changed, catch a copy */
	NUM = alu->regv + num;
	VAL = alu->regv + val;
	
	N = NUM->part;
	V = VAL->part;
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_rem_reg( alu, num );
		alu_rem_reg( alu, val );
		alu_error( ret );
		return ret;
	}

	if ( expect != *N || print_anyways )
	{
		alu_printf(
			"%s, Expected 0x%016zX, Got 0x%016zX, op = '%c'\n",
			pfx, expect, *N, op
		);

#if 0
		alu_print_reg( "num#1", *NUM, 0 );
		(void)memset( N, 0, alu->buff.perN );
		*N = expect;
		alu_print_reg( "num#2", *NUM, 0 );
#endif
	}
	
	alu_rem_reg( alu, num );
	alu_rem_reg( alu, val );
	
	return 0;
}

int uint_modify(
	alu_t *alu,
	size_t _num,
	size_t _val,
	int op,
	bool print_anyways
)
{
	int ret = 0;
	size_t N = _num, V = _val, expect = 0;
	alu_uint_t num = {0}, val = {0};
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	num.perN = sizeof(size_t);
	num.qty.last = 0;
	num.qty.upto = num.qty.used = 1;
	num.mem.block = &N;
	num.mem.bytes.last = sizeof(size_t) - 1;
	num.mem.bytes.upto = num.mem.bytes.used = sizeof(size_t);
	
	val = num;
	val.mem.block = &V;
	
	expect = _num;
	switch ( op )
	{
	case 'n':
		sprintf( pfx, "-0x%016zX", _num );
		expect = -expect;
		ret = alu_uint_neg( alu, num );
		break;
	case '~':
		sprintf( pfx, "~%zu", _num );
		expect = ~expect;
		ret = alu_uint_not( alu, num );
		break;
	case '&':
		sprintf( pfx, "%zu & %zu", _num, _val );
		expect &= _val;
		ret = alu_uint_and( alu, num, val );
		break;
	case '|':
		sprintf( pfx, "%zu | %zu", _num, _val );
		expect |= _val;
		ret = alu_uint__or( alu, num, val );
		break;
	case '^':
		sprintf( pfx, "%zu ^ %zu", _num, _val );
		expect ^= _val;
		ret = alu_uint_xor( alu, num, val );
		break;
	case 'i':
		sprintf( pfx, "%zu++", _num );
		expect++;
		ret = alu_uint_inc( alu, num );
		break;
	case 'd':
		sprintf( pfx, "%zu--", _num );
		expect--;
		ret = alu_uint_dec( alu, num );
		break;
	case '+':
		sprintf( pfx, "%zu + %zu", _num, _val );
		expect += _val;
		ret = alu_uint_add( alu, num, val );
		break;
	case '-':
		sprintf( pfx, "%zu - %zu", _num, _val );
		expect -= _val;
		ret = alu_uint_sub( alu, num, val );
		break;
	case 'l':
		sprintf( pfx, "%zu <<< %zu", _num, _val );
		expect = rol( expect, _val );
		ret = alu_uint_rol( alu, num, val );
		break;
	case '<':
		sprintf( pfx, "%zu << %zu", _num, _val );
		expect <<= _val;
		ret = alu_uint_shl( alu, num, val );
		break;
	case 'r':
		sprintf( pfx, "%zu >>> %zu", _num, _val );
		expect = ror( expect, _val );
		ret = alu_uint_ror( alu, num, val );
		break;
	case '>':
		sprintf( pfx, "%zu >> %zu", _num, _val );
		expect >>= _val;
		ret = alu_uint_shr( alu, num, val );
		break;
	case '*':
		sprintf( pfx, "%zu * %zu", _num, _val );
		expect *= _val;
		ret = alu_uint_mul( alu, num, val );
		break;
	case '/':
		sprintf( pfx, "%zu / %zu", _num, _val );
		if ( _val )
			expect /= _val;
		else
			expect = 0;
		ret = alu_uint_div( alu, num, val );
		break;
	case '%':
		sprintf( pfx, "%zu %% %zu", _num, _val );
		if ( _val )
			expect %= _val;
		ret = alu_uint_rem( alu, num, val );
		break;
	default: ret = ENOSYS;
	}
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_error( ret );
		return ret;
	}

	if ( expect != N || print_anyways )
	{
		alu_printf(
			"%s, Expected %zu, Got %zu, op = '%c'\n",
			pfx, expect, N, op
		);
	}
	
	return 0;
}

int int_modify(
	alu_t *alu,
	ssize_t _num,
	ssize_t _val,
	int op,
	bool print_anyways
)
{
	int ret = 0;
	ssize_t expect = _num;
	size_t N = _num, V = _val;
	alu_int_t num = {0}, val = {0};
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	num.perN = sizeof(size_t);
	num.qty.last = 0;
	num.qty.upto = num.qty.used = 1;
	num.mem.block = &N;
	num.mem.bytes.last = sizeof(size_t) - 1;
	num.mem.bytes.upto = num.mem.bytes.used = sizeof(size_t);
	
	val = num;
	val.mem.block = &V;
	
	switch ( op )
	{
	case 'n':
		sprintf( pfx, "-0x%016zX", _num );
		expect = -expect;
		ret = alu_int_neg( alu, num );
		break;
	case '~':
		sprintf( pfx, "~%zi", _num );
		expect = ~expect;
		ret = alu_int_not( alu, num );
		break;
	case '&':
		sprintf( pfx, "%zi & %zi", _num, _val );
		expect &= _val;
		ret = alu_int_and( alu, num, val );
		break;
	case '|':
		sprintf( pfx, "%zi | %zi", _num, _val );
		expect |= _val;
		ret = alu_int__or( alu, num, val );
		break;
	case '^':
		sprintf( pfx, "%zi ^ %zi", _num, _val );
		expect ^= _val;
		ret = alu_int_xor( alu, num, val );
		break;
	case 'i':
		sprintf( pfx, "%zi++", _num );
		expect++;
		ret = alu_int_inc( alu, num );
		break;
	case 'd':
		sprintf( pfx, "%zi--", _num );
		expect--;
		ret = alu_int_dec( alu, num );
		break;
	case '+':
		sprintf( pfx, "%zi + %zi", _num, _val );
		expect += _val;
		ret = alu_int_add( alu, num, val );
		break;
	case '-':
		sprintf( pfx, "%zi - %zi", _num, _val );
		expect -= _val;
		ret = alu_int_sub( alu, num, val );
		break;
	case 'l':
		sprintf( pfx, "%zi <<< %zi", _num, _val );
		expect = rol( expect, _val );
		ret = alu_int_rol( alu, num, val );
		break;
	case '<':
		sprintf( pfx, "%zi << %zi", _num, _val );
		expect <<= _val;
		ret = alu_int_shl( alu, num, val );
		break;
	case 'r':
		sprintf( pfx, "%zi >>> %zi", _num, _val );
		expect = ror( expect, _val );
		ret = alu_int_ror( alu, num, val );
		break;
	case '>':
		sprintf( pfx, "%zi >> %zi", _num, _val );
		expect >>= _val;
		ret = alu_int_shr( alu, num, val );
		break;
	case '*':
		sprintf( pfx, "%zi * %zi", _num, _val );
		expect *= _val;
		ret = alu_int_mul( alu, num, val );
		break;
	case '/':
		sprintf( pfx, "%zi / %zi", _num, _val );
		if ( _val )
			expect /= _val;
		else
			expect = 0;
		ret = alu_int_div( alu, num, val );
		break;
	case '%':
		sprintf( pfx, "%zi %% %zi", _num, _val );
		if ( _val )
			expect %= _val;
		ret = alu_int_rem( alu, num, val );
		break;
	default: ret = ENOSYS;
	}
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_error( ret );
		return ret;
	}

	if ( expect != *((ssize_t*)(&N)) || print_anyways )
	{
		alu_printf(
			"%s, Expected %zi, Got %zi, op = '%c'\n",
			pfx, expect, *((ssize_t*)(&N)), op
		);
	}
	
	return 0;
}

# define RNG_MOD 0xC0FFEE
# define RNG_MAG 0xDAD

size_t rng( size_t n )
{
	char
		matrix[][16] =
		{
			"0123456789ABCDEF",
			"GHIJKLMNOPQRSTUV",
			"XYZabcdefghijklm",
			"nopqrstuvxyz+-*/",
			".,:;@'~#[]{}()!?"
		}
	;
	size_t
		b = n,
		m = matrix[n % 5][n % 16],
		X = matrix[m % 5][m]
	;
	n = rol( n, X % bitsof(size_t) );
	n *= (m ^ X) ^ (n & 0xF);
	n += b;
	m = matrix[n % 5][X % 16];
	X = matrix[m % 5][m];
	n = ror( n, X % bitsof(size_t) );
	n ^= (uint_t) (((n) * 16777215.0) / UINT_MAX );
	return n % RAND_MAX;
}

void print_limits()
{
	alu_printf( "CHAR_BIT = %u", CHAR_BIT );
	alu_printf("CHAR_SIGNED = %i", CHAR_SIGNED);
	alu_printf("CHAR_UNSIGNED = %i", CHAR_UNSIGNED);
#if (char)-1 < 0
	alu_printf("(char)-1 < 0 = %i", 1);
#else
	alu_printf("(char)-1 < 0 = %i", 0);
#endif
	alu_puts( "===========================================" );
	alu_printf( "UMAX_FOR_1BYTE = %llX", (ullong_t)UMAX_FOR_1BYTE );
	alu_printf( "UMAX_FOR_2BYTE = %llX", (ullong_t)UMAX_FOR_2BYTE );
	alu_printf( "UMAX_FOR_4BYTE = %llX", (ullong_t)UMAX_FOR_4BYTE );
	alu_printf( "UMAX_FOR_8BYTE = %llX", (ullong_t)UMAX_FOR_8BYTE );
	alu_puts( "===========================================" );
	alu_printf( "MAX_FOR_1BYTE = %llX", (llong_t)MAX_FOR_1BYTE );
	alu_printf( "MAX_FOR_2BYTE = %llX", (llong_t)MAX_FOR_2BYTE );
	alu_printf( "MAX_FOR_4BYTE = %llX", (llong_t)MAX_FOR_4BYTE );
	alu_printf( "MAX_FOR_8BYTE = %llX", (llong_t)MAX_FOR_8BYTE );
	alu_puts( "===========================================" );
	alu_printf( "MIN_FOR_1BYTE = %llX", (llong_t)MIN_FOR_1BYTE );
	alu_printf( "MIN_FOR_2BYTE = %llX", (llong_t)MIN_FOR_2BYTE );
	alu_printf( "MIN_FOR_4BYTE = %llX", (llong_t)MIN_FOR_4BYTE );
	alu_printf( "MIN_FOR_8BYTE = %llX", (llong_t)MIN_FOR_8BYTE );
	alu_puts( "===========================================" );
	alu_printf( "SIZEOF_CCINT = %i", SIZEOF_CCINT );
	alu_printf( "SIZEOF_CHAR = %i", SIZEOF_CHAR );
	alu_printf( "SIZEOF_SHRT = %i", SIZEOF_SHRT );
	alu_printf( "SIZEOF_INT = %i", SIZEOF_INT );
	alu_printf( "SIZEOF_LONG = %i", SIZEOF_LONG );
#ifdef LLONG_MAX
	alu_printf( "SIZEOF_LLONG = %i", SIZEOF_LLONG );
#endif
	alu_printf( "SIZEOF_SIZE_T = %i, Actual = %" PRI_SIZE_T "u",
		SIZEOF_SIZE_T, sizeof(size_t) );
	alu_printf( "SIZEOF_PTRDIFF_T = %i, Actual = %zu",
		SIZEOF_PTRDIFF_T, sizeof(ptrdiff_t) );
	alu_printf( "SIZEOF_INTPTR_T = %i, Actual = %zu",
		SIZEOF_INTPTR_T, sizeof(intptr_t) );
	alu_printf( "SIZEOF_INTMAX_T = %i, Actual = %zu",
		SIZEOF_INTMAX_T, sizeof(intmax_t) );
	alu_puts( "===========================================" );
	alu_printf( "CHAR_WIDTH = %u", CHAR_WIDTH );
	alu_printf( "SHRT_WIDTH = %u", SHRT_WIDTH );
	alu_printf( "INT_WIDTH = %u", INT_WIDTH );
	alu_printf( "LONG_WIDTH = %u", LONG_WIDTH );
#ifdef LLONG_MAX
	alu_printf( "LLONG_WIDTH = %i", LLONG_WIDTH );
#endif
	alu_printf( "INT8_T_WIDTH = %u", INT8_T_WIDTH );
	alu_printf( "INT_FAST8_T_WIDTH = %u", INT_FAST8_T_WIDTH );
	alu_printf( "INT_LEAST8_T_WIDTH = %u", INT_LEAST8_T_WIDTH );
	alu_puts( "===========================================" );
	alu_printf( "UCHAR_MAX = %X", UCHAR_MAX );
	alu_printf( "USHRT_MAX = %X", USHRT_MAX );
	alu_printf( "UINT_MAX = %X", UINT_MAX );
	alu_printf( "ULONG_MAX = %lX", ULONG_MAX );
#ifdef LLONG_MAX
	alu_printf( "ULLONG_MAX = %llX", ULLONG_MAX );
#endif
	alu_puts( "===========================================" );
	alu_printf( "CHAR_MAX = %X", CHAR_MAX );
	alu_printf( "SCHAR_MAX = %X", SCHAR_MAX );
	alu_printf( "SHRT_MAX = %X", SHRT_MAX );
	alu_printf( "INT_MAX = %X", INT_MAX );
	alu_printf( "LONG_MAX = %lX", LONG_MAX );
#ifdef LLONG_MAX
	alu_printf( "LLONG_MAX = %llX", LLONG_MAX );
#endif
	alu_puts( "===========================================" );
	alu_printf( "CHAR_MIN = %X", CHAR_MIN );
	alu_printf( "SCHAR_MIN = %X", SCHAR_MIN );
	alu_printf( "SHRT_MIN = %X", SHRT_MIN );
	alu_printf( "INT_MIN = %X", INT_MIN );
	alu_printf( "LONG_MIN = %lX", LONG_MIN );
#ifdef LLONG_MIN
	alu_printf( "LLONG_MIN = %llX", LLONG_MIN );
#endif
	alu_puts( "===========================================" );
}

int compare( alu_t *alu, bool print_anyways )
{
	int ret = 0;
	(void)alu_puts( "Comparing values..." );
	(void)alu_puts( "===========================================" );

	ret = reg_compare( alu, 2, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = reg_compare( alu, 1, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = reg_compare( alu, 0, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = uint_compare( alu, 2, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = uint_compare( alu, 1, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = uint_compare( alu, 0, 1, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = int_compare( alu, 1, 0, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = int_compare( alu, 0, 0, print_anyways );
	
	if ( ret != 0 )
		return ret;

	return int_compare( alu, -1, 0, print_anyways );
}

int modify(
	alu_t *alu,
	ssize_t _num,
	ssize_t _val,
	int op,
	bool print_anyways
)
{
	int ret = 0;
	ret = reg_modify( alu, _num, _val, op, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = uint_modify( alu, _num, _val, op, print_anyways );
	
	if ( ret != 0 )
		return ret;

	ret = int_modify( alu, _num, _val, op, print_anyways );
	return ret;
}

int bitwise(
	alu_t *alu,
	bool doShift,
	bool doRotate,
	bool print_anyways
)
{
	int ret = 0;
	alu_puts( "Checking bitwise operations..." );
	alu_puts( "===========================================" );
	
	ret = modify( alu, 0xDEADC0DE, 0, '~', print_anyways );
	
	if ( ret != 0 )
		return ret;
	
	ret = modify( alu, 0xDEADC0DE, 0xC0FFEE, '&', print_anyways );
	
	if ( ret != 0 )
		return ret;
	
	ret = modify( alu, 0xDEADC0DE, 0xC0FFEE, '|', print_anyways );
	
	if ( ret != 0 )
		return ret;
	
	ret = modify( alu, 0xDEADC0DE, 0xC0FFEE, '^', print_anyways );
	
	if ( doShift )
	{
		if ( ret != 0 )
			return ret;
		
		alu_puts( "Shifting values..." );
		
		ret = modify( alu, 0xDEADC0DELL, 4, '<', print_anyways );
		
		if ( ret != 0 )
			return ret;
		
		ret = modify( alu, -0xDEADC0DELL, 4, '>', print_anyways );
	}
	
	if ( doRotate )
	{
		if ( ret != 0 )
			return ret;
		
		alu_puts( "Rotating values..." );
			
		ret = modify( alu, 0xDEADC0DE, 4, 'l', print_anyways );
		
		if ( ret != 0 )
			return ret;
		
		ret = modify( alu, 0xDEADC0DE, 4, 'r', print_anyways );
	
	}
	
	return ret;
}

int mathmatical(
	alu_t *alu,
	bool doInc,
	bool doDec,
	bool print_anyways
)
{
	int ret = 0;
	alu_puts( "Mathematics values..." );
	
	if ( doInc )
	{
		ret = modify( alu, 0xDEADC0DE, 0xBAD, 'i', print_anyways );
		
		if ( ret != 0 )
			return ret;
		
		ret = modify( alu, 0xDEADC0DE, 0xBAD, '+', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, 0xBAD, '*', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, -0xBADLL, '*', print_anyways );
	}
	
	if ( doDec )
	{
		if ( ret != 0 )
			return ret;
		ret = modify( alu, 0xDEADC0DE, 0xBAD, 'd', print_anyways );
		
		if ( ret != 0 )
			return ret;
		
		ret = modify( alu, 0xDEADC0DE, 0xBAD, '-', print_anyways );
		
		if ( ret != 0 )
			return ret;
		
		ret = modify( alu, 0xDEADC0DE, 0, '/', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, 0xDEADC0DE, 0, '%', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, 12, 10, '/', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, 12, 10, '%', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, 0xDEADC0DE, 0xBAD, '/', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, 0xDEADC0DE, 0xBAD, '%', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, 0xBAD, '/', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, 0xBAD, '%', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, -0xBADLL, '/', print_anyways );
		
		if ( ret != 0 )
			return ret;
			
		ret = modify( alu, -0xDEADC0DELL, -0xBADLL, '%', print_anyways );
	}
	
	return ret;
}

int func_rdChar32( char32_t *dst, alu_block_t *src, long *nextpos )
{
	char *str = src->block;
	if ( (size_t)(*nextpos) < src->bytes.used )
	{
		*dst = str[*nextpos];
		return 0;
	}
	
	*dst = 0;
	return EOF;
}

int func_wrChar32( char32_t src, alu_block_t *dst )
{
	int ret;
	char *str;
	if ( dst->bytes.used >= dst->bytes.last )
	{
		ret = alu_block_expand( dst, dst->bytes.used + 50 );
		if ( ret != 0 )
			return ret;
	}
	str = dst->block;
	str[dst->bytes.used] = src;
	dst->bytes.used++;
	return 0;
}

void func_flipstr( alu_block_t *dst )
{
	char *str = dst->block, c;
	size_t n, v;
	
	for ( n = 0, v = dst->bytes.used - 1; n < v; ++n, --v )
	{
		c = str[n];
		str[n] = str[v];
		str[v] = c;
	}
}

int reg_print_value(
	alu_t *alu,
	alu_block_t _src,
	alu_block_t *_dst,
	char digsep,
	size_t base,
	bool lowercase,
	bool print_anyways
)
{
	uint_t tmp = -1;
	int ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	char *src = _src.block, *dst = NULL;
	long nextpos = 0;
	bool noSign = false, noPfx = false;
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	ret = alu_str2reg(
		alu, &_src, tmp, digsep,
		(alu_func_rdChar32_t)func_rdChar32,
		&nextpos,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
	{
		alu_error(ret);
		goto fail;
	}
	
	(void)memset( _dst->block, 0, _dst->bytes.upto );
	ret = alu_reg2str(
		alu, _dst, tmp, digsep,
		(alu_func_wrChar32_t)func_wrChar32,
		(alu_func_flipstr_t)func_flipstr,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = _src.block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alu_rem_reg( alu, tmp );
	return ret;
}

int uint_print_value(
	alu_t *alu,
	alu_block_t _src,
	alu_block_t *_dst,
	char digsep,
	size_t base,
	bool lowercase,
	bool print_anyways
)
{
	size_t _tmp = 0;
	alu_uint_t tmp = {0};
	int ret = 0;
	char *src = _src.block, *dst = NULL;
	long nextpos = 0;
	bool noSign = false, noPfx = false;
	
	tmp.perN = sizeof(size_t);
	tmp.qty.used = 1;
	tmp.qty.last = 0;
	tmp.qty.upto = 1;
	tmp.mem.block = &_tmp;
	tmp.mem.bytes.used = sizeof(size_t);
	tmp.mem.bytes.last = tmp.mem.bytes.used - 1;
	tmp.mem.bytes.upto = tmp.mem.bytes.used;
	
	ret = alu_str2uint(
		alu, &_src, tmp, digsep,
		(alu_func_rdChar32_t)func_rdChar32,
		&nextpos,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)memset( _dst->block, 0, _dst->bytes.upto );
	ret = alu_uint2str(
		alu, _dst, tmp, digsep,
		(alu_func_wrChar32_t)func_wrChar32,
		(alu_func_flipstr_t)func_flipstr,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = _src.block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	return ret;
}

int int_print_value(
	alu_t *alu,
	alu_block_t _src,
	alu_block_t *_dst,
	char digsep,
	size_t base,
	bool lowercase,
	bool print_anyways
)
{
	size_t _tmp = 0;
	alu_int_t tmp = {0};
	int ret = 0;
	char *src = _src.block, *dst = NULL;
	long nextpos = 0;
	bool noSign = false, noPfx = false;
	
	tmp.perN = sizeof(size_t);
	tmp.qty.used = 1;
	tmp.qty.last = 0;
	tmp.qty.upto = 1;
	tmp.mem.block = &_tmp;
	tmp.mem.bytes.used = sizeof(size_t);
	tmp.mem.bytes.last = tmp.mem.bytes.used - 1;
	tmp.mem.bytes.upto = tmp.mem.bytes.used;
	
	ret = alu_str2int(
		alu, &_src, tmp, digsep,
		(alu_func_rdChar32_t)func_rdChar32,
		&nextpos,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)memset( _dst->block, 0, _dst->bytes.upto );
	ret = alu_int2str(
		alu, _dst, tmp, digsep,
		(alu_func_wrChar32_t)func_wrChar32,
		(alu_func_flipstr_t)func_flipstr,
		base, lowercase, noPfx, noSign
	);
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = _src.block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	return ret;
}

int print_value(
	alu_t *alu,
	char *src,
	char digsep,
	size_t size,
	size_t base,
	bool lowercase,
	bool print_anyways
)
{
	int ret = 0;
	alu_block_t _src = {0}, _dst = {0};
	
	_src.block = src;
	_src.bytes.used = strlen(src);
	_src.bytes.last = _src.bytes.used;
	_src.bytes.upto = size ? size : _src.bytes.used + 1;
	
	ret = alu_block_expand( &_dst, _src.bytes.upto * 2 );
	if ( ret != 0 )
		return ret;
	
	ret = reg_print_value(
		alu,
		_src,
		&_dst,
		digsep,
		base,
		lowercase,
		print_anyways
	);
	
	if ( ret != 0 )
		goto fail;
	
	ret = uint_print_value(
		alu,
		_src,
		&_dst,
		digsep,
		base,
		lowercase,
		print_anyways
	);
	
	if ( ret != 0 )
		goto fail;
	
	ret = int_print_value(
		alu,
		_src,
		&_dst,
		digsep,
		base,
		lowercase,
		print_anyways
	);
	
	fail:
	alu_block_release( &_dst );
	return ret;
}


int main()
{
	int ret = 0;
	//uint_t seed = time(NULL);
	bool print_anyways = false;
	
	alu_t alu;
	
	//print_limits();
	
	alu_puts( "Initiating ALU to 0..." );
	memset( &alu, 0, sizeof(alu_t) );

	alu_puts( "Pre-allocating 16 ALU registers..." );
	alu_setup_reg( &alu, 16, sizeof(size_t) );
	
	print_value( &alu, "123", '\'', 4, 10, false, true );

	compare( &alu, print_anyways );
	bitwise( &alu, true, true, print_anyways );
	mathmatical( &alu, false, true, print_anyways );
	
	(void)alu_vec_shrink( &(alu.buff), 0, 0 );
	(void)alu_vec_shrink( &(alu._regv), 0, 0 );
	(void)memset( &alu, 0, sizeof(alu_t) );
	
#if 0
	for ( num = 0; num < 0x10; ++num )
		alu_printf( "%u", (seed = rng(seed)) );
#endif

	if ( ret != 0 )
		alu_error( ret );
	
	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
