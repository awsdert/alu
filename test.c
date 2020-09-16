#include "alu.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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
	size_t bit = 0;
	alu_uint_t regv[2], num = {0}, val = {0};
	
	ret = alu_get_reg_nodes( alu, regv, 2, 0 );
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	num = regv[0];
	val = regv[1];
	
	(void)alu_set_raw( alu, num, _num, 0 );
	(void)alu_set_raw( alu, val, _val, 0 );
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	cmp = alu_uint_cmp( alu, num, val, &bit );
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"%zu vs %zu Expected %i, Got %i\n",
			_num, _val, expect, cmp
		);
	}
	
	alu_rem_reg_nodes( alu, regv, 2 );
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
	size_t bit = 0;
	alu_int_t regv[2], num = {0}, val = {0};
	
	ret = alu_get_reg_nodes( alu, regv, 2, 0 );
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	num = regv[0];
	val = regv[1];
	
	(void)alu_set_raw( alu, num, _num, 0 );
	(void)alu_set_raw( alu, val, _val, 0 );
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	cmp = alu_int_cmp( alu, num, val, &bit );
	
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
	uint_t nodes[2] = {0};
	alu_reg_t num = {0}, val = {0};
	size_t bit = 0;
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alu_reg_init( alu, num, nodes[0], 0 );
	alu_reg_init( alu, val, nodes[1], 0 );
	
	alu_printf( "1: num.node = %u, val.node = %u", num.node, val.node );
	
	alu_reg_set_raw( alu, num, &_num, num.info, sizeof(size_t) );
	
	alu_printf( "2: num.node = %u, val.node = %u", num.node, val.node );
	
	alu_reg_set_raw( alu, val, &_val, val.info, sizeof(size_t) );
	
	alu_printf( "3: num.node = %u, val.node = %u", num.node, val.node );
	
	if ( _num > _val )
		expect = 1;
	
	if ( _num < _val )
		expect = -1;

	cmp = alu_reg_cmp( alu, num, val, &bit );
	
	alu_printf( "4: num.node = %u, val.node = %u", num.node, val.node );
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"0x%016zX vs 0x%016zX Expected %i, Got %i, Bit = %zu\n",
			_num, _val, expect, cmp, bit
		);
		alu_printf( "5: num.node = %u, val.node = %u", num.node, val.node );
		alu_print_reg( "num", alu, num, false, true );
		alu_print_reg( "val", alu, val, false, true );
	}
	
	alu_rem_reg_nodes( alu, nodes, 2 );
	
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
	uint_t regv[3] = {-1};
	alu_reg_t num = {0}, val = {0}, tmp = {0};
	size_t expect = 0;
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	ret = alu_get_reg_nodes( alu, regv, 3, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alu_reg_init( alu, num, regv[0], 0 );
	alu_reg_init( alu, val, regv[1], 0 );
	alu_reg_init( alu, tmp, regv[2], 0 );
	
	alu_set_raw( alu, num.node, _num, 0 );
	alu_set_raw( alu, val.node, _val, 0 );
	
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
		alu_reg_neg( alu, num );
		break;
	case '~':
		sprintf( pfx, "~0x%016zX", _num );
		expect = ~expect;
		alu_reg_not( alu, num );
		break;
	case '&':
		sprintf( pfx, "0x%016zX & 0x%016zX", _num, _val );
		expect &= _val;
		alu_reg_and( alu, num, val );
		break;
	case '|':
		sprintf( pfx, "0x%016zX | 0x%016zX", _num, _val );
		expect |= _val;
		alu_reg__or( alu, num, val );
		break;
	case '^':
		sprintf( pfx, "0x%016zX ^ 0x%016zX", _num, _val );
		expect ^= _val;
		alu_reg_xor( alu, num, val );
		break;
	case 'i':
		sprintf( pfx, "0x%016zX++", _num );
		expect++;
		ret = alu_reg_inc( alu, num );
		break;
	case 'd':
		sprintf( pfx, "0x%016zX--", _num );
		expect--;
		ret = alu_reg_dec( alu, num );
		break;
	case '+':
		sprintf( pfx, "0x%016zX + 0x%016zX", _num, _val );
		expect += _val;
		ret = alu_reg_add( alu, num, val );
		break;
	case '-':
		sprintf( pfx, "0x%016zX - 0x%016zX", _num, _val );
		expect -= _val;
		ret = alu_reg_sub( alu, num, val );
		break;
	case 'l':
		sprintf( pfx, "0x%016zX <<< 0x%016zX", _num, _val );
		expect = rol( expect, _val );
		alu_reg_rol( alu, num, val, tmp );
		break;
	case '<':
		sprintf( pfx, "0x%016zX << 0x%016zX", _num, _val );
		expect <<= _val;
		alu_reg_shl( alu, num, val, tmp );
		break;
	case 'r':
		sprintf( pfx, "0x%016zX >>> 0x%016zX", _num, _val );
		expect = ror( expect, _val );
		alu_reg_ror( alu, num, val, tmp );
		break;
	case '>':
		sprintf( pfx, "0x%016zX >> 0x%016zX", _num, _val );
		expect >>= _val;
		alu_reg_shr( alu, num, val, tmp );
		break;
	case '*':
		sprintf( pfx, "0x%016zX * 0x%016zX", _num, _val );
		expect *= _val;
		ret = alu_reg_mul( alu, num, val, tmp );
		break;
	case '/':
		sprintf( pfx, "0x%016zX / 0x%016zX", _num, _val );
		if ( _val )
			expect /= _val;
		else
			expect = 0;
		ret = alu_reg_div( alu, num, val, tmp );
		break;
	case '%':
		sprintf( pfx, "0x%016zX %% 0x%016zX", _num, _val );
		if ( _val )
			expect %= _val;
		ret = alu_reg_rem( alu, num, val, tmp );
		break;
	default: ret = ENOSYS;
	}
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_rem_reg_nodes( alu, regv, 3 );
		alu_error( ret );
		return ret;
	}
	
	alu_get_raw( alu, num.node, &_num );

	if ( expect != _num || print_anyways )
	{
		alu_printf(
			"%s, Expected 0x%016zX, Got 0x%016zX, op = '%c'\n",
			pfx, expect, _num, op
		);

#if 0
		alu_print_reg( "num#1", num, 0 );
		alu_set_raw( alu, num.node, expect, 0 );
		alu_print_reg( "num#2", num, 0 );
#endif
	}
	
	alu_rem_reg_nodes( alu, regv, 3 );
	
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
	size_t N = _num, expect = 0;
	alu_uint_t regv[2], num = {0}, val = {0};
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	ret = alu_get_reg_nodes( alu, regv, 2, 0 );
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	num = regv[0];
	val = regv[1];
	
	(void)alu_set_raw( alu, num, _num, 0 );
	(void)alu_set_raw( alu, val, _val, 0 );
	
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
	size_t N = _num;
	uint_t regv[2] = {-1};
	alu_int_t num = {0}, val = {0};
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	ret = alu_get_reg_nodes( alu, regv, 2, 0 );
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	num = regv[0];
	val = regv[1];
	
	(void)alu_set_raw( alu, num, _num, 0 );
	(void)alu_set_raw( alu, val, _val, 0 );
	
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

int reg_print_value
(
	alu_t *alu
	, alu_src_t _src
	, alu_dst_t _dst
	, alu_base_t base
	, bool print_anyways
)
{
	uint_t tmp = -1;
	alu_reg_t _tmp = {0};
	int ret = alu_get_reg_node( alu, &tmp, 0 );
	alu_block_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_lit2reg() & alu_reg2str()" );
		alu_puts( "-------------------------------------" );
	}
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	ret = alu_lit2reg( alu, _src, _tmp, base );
	
	switch ( ret )
	{
	case 0: case EILSEQ: break;
	default:
		alu_error(ret);
		goto fail;
	}
	
	if ( ret != 0 )
	{
		alu_error(ret);
		goto fail;
	}
	
	(void)memset( __dst->block, 0, __dst->bytes.upto );
	ret = alu_reg2str( alu, _dst, _tmp, base );
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alu_rem_reg_node( alu, &tmp );
	return ret;
}

int uint_print_value
(
	alu_t *alu
	, alu_src_t _src
	, alu_dst_t _dst
	, alu_base_t base
	, bool print_anyways
)
{
	alu_uint_t tmp = -1;
	int ret = alu_get_reg_node( alu, &tmp, 0 );
	alu_block_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_str2uint() & alu_uint2str()" );
		alu_puts( "---------------------------------------" );
	}
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	ret = alu_str2uint( alu, _src, tmp, base );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		goto fail;
	}
	
	(void)memset( __dst->block, 0, __dst->bytes.upto );
	ret = alu_uint2str( alu, _dst, tmp, base );
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alu_rem_reg_node( alu, &tmp );
	return ret;
}

int int_print_value
(
	alu_t *alu
	, alu_src_t _src
	, alu_dst_t _dst
	, alu_base_t base
	, bool print_anyways
)
{
	alu_int_t tmp = -1;
	int ret = alu_get_reg_node( alu, &tmp, 0 );
	alu_block_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_str2int() & alu_int2str()" );
		alu_puts( "-------------------------------------" );
	}
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	ret = alu_str2int( alu, _src, tmp, base );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		goto fail;
	}
	
	(void)memset( __dst->block, 0, __dst->bytes.upto );
	ret = alu_int2str( alu, _dst, tmp, base );
	
	
	if ( ret != 0 )
		alu_error(ret);
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alu_rem_reg_node( alu, &tmp );
	return ret;
}

char *pos_int[] = {
	"123",
	"+145",
	"0xA",
	"0b1",
NULL };

char *neg_int[] = {
	"-123",
	"-0xA",
	"-0b1",
NULL };

char *pos_fpn[] = {
	"0",
	"1",
	"1.0",
	"0.1",
	"1.1",
	"1e+10",
	"1.0e+10",
	"0.1e+10",
	"1.1e+10",
	"1e-10",
	"1.0e-10",
	"0.1e-10",
	"1.1e-10",
	"0xA",
	"0xA.0",
	"0x0.A",
	"0xA.A",
	"0xAp+10",
	"0xA.0p+10",
	"0x0.Ap+10",
	"0xA.Ap+10",
	"0xAp-10",
	"0xA.0p-10",
	"0x0.Ap-10",
	"0xA.Ap-10",
	"0b1",
	"0b1.0",
	"0b0.1",
	"0b1.1",
	"0b1e+10",
	"0b1.0e+10",
	"0b0.1e+10",
	"0b1.1e+10",
	"0b1e-10",
	"0b1.0e-10",
	"0b0.1e-10",
	"0b1.1e-10",
NULL };

char *neg_fpn[] = {
	"-0.0",
	"-1",
	"-1.0",
	"-0.1",
	"-1.1",
	"-1e+10",
	"-1.0e+10",
	"-0.1e+10",
	"-1.1e+10",
	"-1e-10",
	"-1.0e-10",
	"-0.1e-10",
	"-1.1e-10",
	"-0xA",
	"-0xA.0",
	"-0x0.A",
	"-0xA.A",
	"-0xAp+10",
	"-0xA.0p+10",
	"-0x0.Ap+10",
	"-0xA.Ap+10",
	"-0xAp-10",
	"-0xA.0p-10",
	"-0x0.Ap-10",
	"-0xA.Ap-10",
	"-0b1",
	"-0b1.0",
	"-0b0.1",
	"-0b1.1",
	"-0b1e+10",
	"-0b1.0e+10",
	"-0b0.1e+10",
	"-0b1.1e+10",
	"-0b1e-10",
	"-0b1.0e-10",
	"-0b0.1e-10",
	"-0b1.1e-10",
NULL };

int print_value( alu_t *alu, bool print_anyways )
{
	int ret = 0, i;
	alu_src_t _src = {NULL};
	alu_dst_t _dst = {NULL};
	alu_base_t base = {0};
	alu_block_t __src = {0}, __dst = {0};
	char *src;
	size_t size;
	long nextpos;
	
	base.digsep = '\'';
	base.base = 10;
	
	_src.src = &__src;
	_src.next = (alu_func_rdChar32_t)func_rdChar32;
	_src.nextpos = &nextpos;
	
	_dst.dst = &__dst;
	_dst.next = (alu_func_wrChar32_t)func_wrChar32;
	_dst.flip = (alu_func_flipstr_t)func_flipstr;
	
	for ( i = 0; pos_int[i]; ++i )
	{
		src = pos_int[i];
		size = strlen(src);
		switch ( src[1] )
		{
		case 'x': case 'X': base.base = 16; break;
		case 'b': case 'B': base.base = 2; break;
		default: base.base = 10;
		}
		
		nextpos = 0;
		__src.block = src;
		__src.bytes.used = strlen(src);
		__src.bytes.last = __src.bytes.used;
		__src.bytes.upto = size ? size : __src.bytes.used + 1;
		
		ret = alu_block_expand( &__dst, __src.bytes.upto * 2 );
		if ( ret != 0 )
			return ret;
		
		ret = reg_print_value( alu, _src, _dst, base, print_anyways );
		
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
		
		ret = uint_print_value( alu, _src, _dst, base, print_anyways );
		
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
		
		ret = int_print_value( alu, _src, _dst, base, print_anyways );
		
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
	}
	
	for ( i = 0; neg_int[i]; ++i )
	{
		src = neg_int[i];
		size = strlen(src);
		switch ( src[1] )
		{
		case 'x': case 'X': base.base = 16; break;
		case 'b': case 'B': base.base = 2; break;
		default: base.base = 10;
		}
		
		nextpos = 0;
		__src.block = src;
		__src.bytes.used = strlen(src);
		__src.bytes.last = __src.bytes.used;
		__src.bytes.upto = size ? size : __src.bytes.used + 1;
		
		ret = alu_block_expand( &__dst, __src.bytes.upto * 2 );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
		
		ret = int_print_value( alu, _src, _dst, base, print_anyways );
		
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
	}
	
	fail:
	alu_block_release( &__dst );
	return ret;
}


int main()
{
	int ret = 0;
	uint_t preallocate = 32;
	//uint_t seed = time(NULL);
	bool print_anyways = false;
	
	alu_t _alu = {0}, *alu = &_alu;
	
	//print_limits();

	alu_printf( "Pre-allocating %u ALU registers...", preallocate );
	ret = alu_setup_reg( alu, preallocate, 0, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return EXIT_FAILURE;
	}

#if 1
	ret = compare( alu, print_anyways );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
#endif

#if 0
	ret = bitwise( alu, true, true, print_anyways );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
#endif

#if 0
	ret = mathmatical( alu, false, true, print_anyways );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
#endif

	ret = print_value( alu, true );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
#if 0
	for ( num = 0; num < 0x10; ++num )
		alu_printf( "%u", (seed = rng(seed)) );
#endif

	if ( ret != 0 )
		alu_error( ret );
		
		
	fail:
	
	(void)alu_vec_shrink( alu, 0, 0 );
	(void)memset( alu, 0, sizeof(alu_t) );
	
	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
