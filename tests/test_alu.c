#include <alu.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int reg_compare(
	alu_t *alu,
	intmax_t _num,
	intmax_t _val,
	uint_t info,
	bool print_anyways
)
{
	int ret = 0, cmp = 0, expect = 0;
	uint_t nodes[2] = {0};
	alur_t NUM, VAL;
	
	ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alur_init_unsigned( alu, NUM, nodes[0] );
	alur_init_unsigned( alu, VAL, nodes[1] );
	
	NUM.info = info;
	VAL.info = info;
	
	NUM.upto = VAL.upto = bitsof(intmax_t);
	
	alu_int_set_raw( alu, NUM.node, _num );
	alu_int_set_raw( alu, VAL.node, _val );
	
	expect = EITHER( _num > _val, 1, expect );
	expect = EITHER( _num < _val, -1, expect );

	cmp = alur_cmp( alu, NUM, VAL );
	
	if ( expect != cmp || print_anyways )
	{
		alu_printf(
			"0x%016zX vs 0x%016zX Expected %i, Got %i\n",
			_num, _val, expect, cmp
		);
		alur_print( alu, NUM, false, true );
		alur_print( alu, VAL, false, true );
	}
	
	alur_rem_nodes( alu, nodes, 2 );
	
	return ret;
}

int modify(
	alu_t *alu
	, intmax_t _num
	, intmax_t _val
	, uint_t info
	, bool print_anyways
	, int op
)
{
	int ret = 0;
	uint_t nodes[3] = {0}, tmp;
	alur_t NUM, VAL;
	intmax_t expect = 0;
	char pfx[sizeof(size_t) * CHAR_BIT] = {0};
	
	ret = alur_get_nodes( alu, nodes, 3, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alur_init_unsigned( alu, NUM, nodes[0] );
	alur_init_unsigned( alu, VAL, nodes[1] );
	tmp = nodes[2];
	
	NUM.info = info;
	VAL.info = info;
	
	alu_int_set_raw( alu, NUM.node, _num );
	alu_int_set_raw( alu, VAL.node, _val );
	
	NUM.upto = VAL.upto = bitsof(intmax_t);
	
	expect = _num;
	switch ( op )
	{
	case 'n':
		sprintf( pfx, "-0x%016jX", (uintmax_t)_num );
		expect = -expect;
		ret = alur_neg( alu, NUM );
		break;
	case '~':
		sprintf( pfx, "~0x%016jX", (uintmax_t)_num );
		expect = ~expect;
		ret = alur_not( alu, NUM );
		break;
	case '&':
		sprintf( pfx, "0x%016jX & 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect &= _val;
		ret = alur_and( alu, NUM, VAL );
		break;
	case '|':
		sprintf( pfx, "0x%016jX | 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect |= _val;
		ret = alur__or( alu, NUM, VAL );
		break;
	case '^':
		sprintf( pfx, "0x%016jX ^ 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect ^= _val;
		ret = alur_xor( alu, NUM, VAL );
		break;
	case 'i':
		sprintf( pfx, "0x%016jX++", (uintmax_t)_num );
		expect++;
		ret = alur_inc( alu, NUM );
		break;
	case 'd':
		sprintf( pfx, "0x%016jX--", (uintmax_t)_num );
		expect--;
		ret = alur_dec( alu, NUM );
		break;
	case '+':
		sprintf( pfx, "0x%016jX + 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect += _val;
		ret = alur_add( alu, NUM, VAL );
		break;
	case '-':
		sprintf( pfx, "0x%016jX - 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect -= _val;
		ret = alur_sub( alu, NUM, VAL );
		break;
	case 'l':
		sprintf( pfx, "0x%016jX <<< 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect = unic_rol( expect, _val );
		ret = alur_rol( alu, NUM, VAL, tmp );
		break;
	case '<':
		sprintf( pfx, "0x%016jX << 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect <<= _val;
		ret = alur_shl( alu, NUM, VAL );
		break;
	case 'r':
		sprintf( pfx, "0x%016jX >>> 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect = unic_ror( expect, _val );
		ret = alur_ror( alu, NUM, VAL, tmp );
		break;
	case '>':
		sprintf( pfx, "0x%016jX >> 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect >>= _val;
		ret = alur_shr( alu, NUM, VAL );
		break;
	case '*':
		sprintf( pfx, "0x%016jX * 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		expect *= _val;
		ret = alur_mul( alu, NUM, VAL );
		break;
	case '/':
		sprintf( pfx, "0x%016jX / 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		if ( _val )
			expect /= _val;
		else
			expect = 0;
		ret = alur_div( alu, NUM, VAL );
		break;
	case '%':
		sprintf( pfx, "0x%016jX %% 0x%016jX", (uintmax_t)_num, (uintmax_t)_val );
		if ( _val )
			expect %= _val;
		ret = alur_rem( alu, NUM, VAL );
		break;
	default: ret = ENOSYS;
	}
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_error( ret );
		alu_printf( "op = '%c'", op );
		goto fail;
	}
	
	ret = alu_int_get_raw( alu, NUM.node, &_num );

	if ( expect != _num || print_anyways )
	{
		alu_printf(
			"%s, Expected 0x%016jX, Got 0x%016jX, op = '%c'\n",
			pfx, expect, _num, op
		);
		alur_print( alu, NUM, false, true );
		alur_print( alu, VAL, false, true );
	}
	
	fail:
	
	alur_rem_nodes( alu, nodes, 3 );
	
	return ret;
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
	n = unic_rol( n, X % bitsof(size_t) );
	n *= (m ^ X) ^ (n & 0xF);
	n += b;
	m = matrix[n % 5][X % 16];
	X = matrix[m % 5][m];
	n = unic_ror( n, X % bitsof(size_t) );
	n ^= (uint_t) (((n) * 16777215.0) / UINT_MAX );
	return n % RAND_MAX;
}

void print_limits()
{
	alu_printf( "UNIC_CHAR_BIT = %u", UNIC_CHAR_BIT );
	alu_printf("UNIC_CHAR_SIGNED = %i", UNIC_CHAR_SIGNED);
	alu_printf("UNIC_CHAR_UNSIGNED = %i", UNIC_CHAR_UNSIGNED);
#if (char)-1 < 0
	alu_printf("(char)-1 < 0 = %i", 1);
#else
	alu_printf("(char)-1 < 0 = %i", 0);
#endif
	alu_puts( "===========================================" );
#define _print( BYTES, SFX ) \
	alu_printf \
	( \
		"UNIC_" #BYTES "_" #SFX " = %llX" \
		, (ullong_t)UNIC_##BYTES##BYTE_##SFX \
	);
#define print( BYTES ) \
	_print( BYTES, UMAX ); _print( BYTES, MAX ); _print( BYTES, MIN )
	print( 1 );
	print( 2 );
	print( 4 );
	print( 8 );
#undef print
#undef _print
	alu_puts( "===========================================" );
	alu_printf( "UNIC_SIZEOF_CHAR = %i", UNIC_SIZEOF_CHAR );
	alu_printf( "UNIC_SIZEOF_SHRT = %i", UNIC_SIZEOF_SHRT );
	alu_printf( "UNIC_SIZEOF_INT = %i", UNIC_SIZEOF_INT );
	alu_printf( "UNIC_SIZEOF_LONG = %i", UNIC_SIZEOF_LONG );
#ifdef LLONG_MAX
	alu_printf( "UNIC_SIZEOF_LLONG = %i", UNIC_SIZEOF_LLONG );
#endif
#define print( NAME, TYPE ) \
	alu_printf \
	( \
		"UNIC_SIZEOF_" #NAME " = %i, Actual = %zu" \
		, UNIC_SIZEOF_##NAME \
		, sizeof(TYPE) \
	)
	print( SIZE, size_t );
	print( PTRDIFF, ptrdiff_t );
	print( INTPTR, intptr_t );
	print( INTMAX, intmax_t );
#undef print
	alu_puts( "===========================================" );
#define print( NAME ) alu_printf( "UNIC_" #NAME " = %u", UNIC_##NAME )
	print( CHAR_WIDTH );
	print( SHRT_WIDTH );
	print( INT_WIDTH );
	print( LONG_WIDTH );
#ifdef LLONG_MAX
	print( LLONG_WIDTH );
#endif
	print( INT8_WIDTH );
	print( INT_FAST8_WIDTH );
	print( INT_LEAST8_WIDTH );
#undef print
	alu_puts( "===========================================" );
#define print( NAME ) \
	alu_printf( "UNIC_" #NAME " = %016llX", (ullong_t)UNIC_##NAME )
	print( UCHAR_MAX );
	print( USHRT_MAX );
	print( UINT_MAX );
	print( ULONG_MAX );
#ifdef LLONG_MAX
	print( ULLONG_MAX );
#endif
#undef print
	alu_puts( "===========================================" );
#define print( NAME ) \
	alu_printf( "UNIC_" #NAME " = %016llX", (llong_t)UNIC_##NAME )
	print( SCHAR_MAX );
	print( SHRT_MAX );
	print( INT_MAX );
	print( LONG_MAX );
#ifdef LLONG_MAX
	print( LLONG_MAX );
#endif
	alu_puts( "===========================================" );
	print( SCHAR_MIN );
	print( SHRT_MIN );
	print( INT_MIN );
	print( LONG_MIN );
#ifdef LLONG_MIN
	print( LLONG_MIN );
#endif
#undef print
	alu_puts( "===========================================" );
}

int compare( alu_t *alu, bool print_anyways )
{
	int ret, a, b;
	(void)alu_puts( "Comparing values..." );
	(void)alu_puts( "===========================================" );
	
	for ( a = 2, b = 1; a >= 0; --a )
	{
		ret = reg_compare( alu, a, b, 0, print_anyways );
		
		if ( ret != 0 )
			return ret;
	}

	for ( a = 1, b = 0; a >= -1; --a )
	{
		ret = reg_compare( alu, a, b, ALU_INFO__SIGN, print_anyways );
		
		if ( ret != 0 )
			return ret;
	}

	return 0;
}

int bitwise
(
	alu_t *alu,
	bool doShift,
	bool doRotate,
	bool print_anyways
)
{
	int ret = 0;
	size_t i;
	intmax_t num = 0xDEADC0DE, val;
	uint_t info = ALU_INFO__SIGN;
	char bit_ops[] = "~&|^";
	
	alu_puts( "Checking Bitwise Operations..." );
	alu_puts( "===========================================" );
	
	val = 0xC0FFEE;
	
	for ( i = 0; bit_ops[i]; ++i )
	{
		ret = modify( alu, num, val, info, print_anyways, bit_ops[i] );
		
		if ( ret != 0 )
			return ret;
	}
	
	val = 4;
	
	if ( doShift )
	{
		char mov_ops[] = "<>";
		
		alu_puts( "Shifting values..." );
		(void)alu_puts( "===========================================" );
		
		for ( i = 0; mov_ops[i]; ++i )
		{
			ret = modify( alu, num, val, info, print_anyways, mov_ops[i] );
			
			if ( ret != 0 )
			{
				alu_error( ret );
				return ret;
			}
		}
	}
	
	if ( doRotate )
	{	
		char rot_ops[] = "lr";
		
		alu_puts( "Rotating values..." );
		(void)alu_puts( "===========================================" );
			
		for ( i = 0; rot_ops[i]; ++i )
		{
			ret = modify( alu, num, val, info, print_anyways, rot_ops[i] );
			
			if ( ret != 0 )
			{
				alu_error( ret );
				return ret;
			}
		}
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
	alu_puts( "Checking Mathematic Operations..." );
	(void)alu_puts( "===========================================" );
	size_t i;
	uint_t info = ALU_INFO__SIGN;
	intmax_t num = 0xDEADC0DE, val;
	char inc_ops[] = "i+*";
	
	val = 0xBAD;
	
	if ( doInc )
	{		
		for ( i = 0; inc_ops[i]; ++i )
		{
			alu_printf( "Trying '%c", inc_ops[i] );
			ret = modify( alu, num, val, info, print_anyways, inc_ops[i] );
			
			if ( ret != 0 )
			{
				alu_error( ret );
				return ret;
			}
		}
	}
	
	if ( doDec )
	{
		char dec_ops[] = "d-/%";
		
		for ( i = 0; dec_ops[i]; ++i )
		{
			alu_printf( "Trying '%c, both positive", dec_ops[i] );
			ret = modify( alu, num, val, info, print_anyways, dec_ops[i] );
			
			if ( ret != 0 && ret != EOVERFLOW )
			{
				alu_error( ret );
				return ret;
			}
		}
			
		for ( i = 0; dec_ops[i]; ++i )
		{
			alu_printf( "Trying '%c, negative num", inc_ops[i] );
			ret = modify( alu, -num, val, info, print_anyways, dec_ops[i] );
			
			if ( ret != 0 && ret != EOVERFLOW )
			{
				alu_error( ret );
				return ret;
			}
		}
		
		for ( i = 0; dec_ops[i]; ++i )
		{
			alu_printf( "Trying '%c, both negative", inc_ops[i] );
			ret = modify( alu, -num, -val, info, print_anyways, dec_ops[i] );
			
			if ( ret != 0 && ret != EOVERFLOW )
			{
				alu_error( ret );
				return ret;
			}
		}
		
		for ( i = 0; dec_ops[i]; ++i )
		{
			alu_printf( "Trying '%c, val is 0", inc_ops[i] );
			ret = modify( alu, num, 0, info, print_anyways, dec_ops[i] );
			
			if ( ret != 0 && ret != EOVERFLOW )
			{
				alu_error( ret );
				return ret;
			}
		}
	}
	
	return ret;
}

int func_rdChar32( char32_t *dst, alum_t *src, long *nextpos )
{
	char *str = src->block;
	if ( (size_t)(*nextpos) < src->taken )
	{
		*dst = str[*nextpos];
		return 0;
	}
	
	*dst = 0;
	return EOF;
}

int func_wrChar32( char32_t src, alum_t *dst )
{
	char *str;
	if ( dst->taken >= dst->given )
	{
		str = alum_expand( dst, dst->taken + 50 );
		if ( !str )
		{
			int ret = dst->fault;
			alu_error( ret );
			return ret;
		}
	}
	else
	{
		str = dst->block;
	}
	str[dst->taken] = src;
	dst->taken++;
	return 0;
}

void func_flipstr( alum_t *dst )
{
	char *str = dst->block;
	size_t n, v;
	
	for ( n = 0, v = dst->taken - 1; n < v; ++n, --v )
	{
		char c = str[n];
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
	int ret;
	uint_t tmp = alur_get_node( alu, 0 );
	alur_t _tmp = {0};
	alum_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_lit2reg() & alur2str()" );
		alu_puts( "-------------------------------------" );
	}
	
	if ( tmp == 0 )
	{
		ret = alu_errno(alu);
		alu_error( ret );
		return ret;
	}
	
	alur_init_unsigned( alu, _tmp, tmp );
	
	ret = alu_lit2reg( alu, _src, _tmp, base );
	
	switch ( ret )
	{
	case 0: case EILSEQ: break;
	default:
		alu_error( ret );
		goto fail;
	}
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
	__dst->taken = 0;
	(void)memset( __dst->block, 0, __dst->given );
	ret = alur2str( alu, _dst, _tmp, base );
	
	if ( ret != 0 )
		alu_error( ret );
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alur_rem_node( alu, &tmp );
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
	int ret;
	alu_uint_t tmp = alur_get_node( alu, 0 );
	alum_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_str2uint() & alu_uint2str()" );
		alu_puts( "---------------------------------------" );
	}
	
	if ( tmp == 0 )
	{
		ret = alu_errno(alu);
		alu_error( ret );
		return ret;
	}
	
	ret = alu_str2uint( alu, _src, tmp, base );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
	(void)memset( __dst->block, 0, __dst->given );
	ret = alu_uint2str( alu, _dst, tmp, base );
	
	if ( ret != 0 )
		alu_error( ret );
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alur_rem_node( alu, &tmp );
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
	int ret;
	alu_int_t tmp = alur_get_node( alu, 0 );
	alum_t *__src = _src.src, *__dst = _dst.dst;
	char *src = __src->block, *dst = NULL;
	
	if ( print_anyways )
	{
		alu_puts( "Testing alu_str2int() & alu_int2str()" );
		alu_puts( "-------------------------------------" );
	}
	
	if ( tmp == 0 )
	{
		ret = alu_errno(alu);
		alu_error( ret );
		return ret;
	}
	
	ret = alu_str2int( alu, _src, tmp, base );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
	(void)memset( __dst->block, 0, __dst->given );
	ret = alu_int2str( alu, _dst, tmp, base );
	
	
	if ( ret != 0 )
		alu_error( ret );
	
	dst = __src->block;
	if ( strcmp( src, dst ) != 0 || print_anyways )
		(void)alu_printf( "Expected = '%s', Got = '%s'", src, dst );
	
	fail:
	alur_rem_node( alu, &tmp );
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
	alum_t __src = {0}, __dst = {0};
	char *src, *dst;
	long nextpos;
	
	(void)alu_puts( "Printing values..." );
	(void)alu_puts( "===========================================" );
	
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
		switch ( src[1] )
		{
		case 'x': case 'X': base.base = 16; break;
		case 'b': case 'B': base.base = 2; break;
		default: base.base = 10;
		}
		
		nextpos = 0;
		__src.block = src;
		__src.given = __src.taken = strlen(src);
		
		dst = alum_expand( &__dst, __src.given * 2 );
		if ( !dst )
		{
			ret = __dst.fault;
			alu_error( ret );
			goto fail;
		}
		
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
		switch ( src[1] )
		{
		case 'x': case 'X': base.base = 16; break;
		case 'b': case 'B': base.base = 2; break;
		default: base.base = 10;
		}
		
		nextpos = 0;
		__src.block = src;
		__src.given = __src.taken = strlen(src);
		
		dst = alum_expand( &__dst, __src.given * 2 );
		
		if ( !dst )
		{
			ret = __dst.fault;
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
	alum_release( &__dst );
	return ret;
}


int main()
{
	int ret = 0;
	uint_t preallocate = 64;
	//uint_t seed = time(NULL);
	bool print_anyways = false;
	
	alu_t _alu = {0}, *alu = &_alu;
	
	//print_limits();

	alu_printf( "Pre-allocating %u ALU registers...", preallocate );
	ret = alur_ensure( alu, preallocate, 0 );
	
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

#if 1
	ret = bitwise( alu, true, true, print_anyways );
	
	if ( ret != 0 && ret != EOVERFLOW )
	{
		alu_error( ret );
		goto fail;
	}
#endif

#if 1
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
	
	(void)aluv_shrink( alu, 0, 0 );
	(void)memset( alu, 0, sizeof(alu_t) );
	
	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
