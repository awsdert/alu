#include <alu.h>
#include <stdlib.h>
#include <time.h>

alu_t *alu = NULL;
const int per_func = LDBL_MANT_DIG;
bool_t stop_checks = false;

#define REG_COUNT 3
#define rotate_range( NUM, BITS ) ((BITS) ? bitsof(NUM) % (BITS) : 0)
#define rotate_shift( NUM, DIR, BITS ) ((NUM) DIR rotate_range(NUM,BITS))
#define rotate_minus( NUM, DIR, BITS ) ((NUM) DIR (bitsof(NUM) - rotate_range(NUM,BITS)))
#define rol( NUM, BITS ) (rotate_shift( NUM, <<, BITS ) | rotate_minus( NUM, >>, BITS ))
#define ror( NUM, BITS ) (rotate_minus( NUM, >>, BITS ) | rotate_shift( NUM, <<, BITS ))

int wrChar( char32_t c, void *dst )
{
	alum_t *ALUSTR = dst;
	char *alustr = alum_expand( dst, ALUSTR->taken + 1 );
	
	if ( !alustr )
	{
		alu_error(ALUSTR->fault);
		return ALUSTR->fault;
	}
	
	alustr = ALUSTR->block;
	alustr[ALUSTR->taken] = (char)c;
	ALUSTR->taken++;
	return 0;
}

void flip( void *dst )
{
	alum_t *ALUSTR = dst;
	uint_t i, j;
	char *alustr = ALUSTR->block;
	
	for ( i = 0, j = ALUSTR->taken - 1; i < j; ++i, --j )
	{
		char c = alustr[i];
		alustr[i] = alustr[j];
		alustr[j] = c;
	}
}

#include <check.h>

START_TEST( test_alur_ensure )
{
	int ret;
	uint_t want = bitsof(uintmax_t);

	ret = alur_ensure( alu, want, 0 );
	
	ck_assert( ret == 0 );
	ck_assert( alu_used( alu ) > 0 );
	ck_assert( alu_upto( alu ) > 0 );
	ck_assert_msg
	(
		alu_upto( alu ) >= want
		, "Expected at least %u, Got %u"
		, want
		, alu_upto(alu)
	);
	ck_assert( alu_valid( alu ) != NULL );
	ck_assert( alu_Nsize( alu ) >= sizeof(uintmax_t) );
}
END_TEST

START_TEST( test_alur2str )
{
	int ret;
	uintmax_t want = 16;
	alum_t ALUSTR = {0};
	alu_dst_t alu_dst = {0};
	alu_base_t base = {0};
	char stdstr[bitsof(uint_t) * 2] = {0};
	uint_t num = alur_get_node( alu, 0 );
	
	ck_assert( num != 0 );
	
	alu_dst.dst = &ALUSTR;
	alu_dst.next = wrChar;
	alu_dst.flip = flip;
	
	base.base = 10;
	base.digsep = '\'';
	sprintf( stdstr, "%ju", want );
	alu_uint_set_raw( alu, num, want );
	ret = alu_uint2str( alu, alu_dst, num, base );
	
	if ( ret != 0 )
	{
		alu_error( ret );
	}
	else
	{
		char *alustr = ALUSTR.block;
		ck_assert_msg
		(
			memcmp( stdstr, alustr, strlen(stdstr) ) == 0
			, "Expected '%s', Got '%s'"
			, stdstr
			, alustr
		);
	}
	
	alum_release( &ALUSTR );
	
	alur_rem_node( alu, &num );
}
END_TEST

START_TEST( test_alur_get_node )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t num = alur_get_node( alu, 0 );
	bool active;
	
	ck_assert( num != 0 );
	
	active = alu_get_active( alu, num );
	
	ck_assert( num != 0 );
	ck_assert( active == true );
	
	alur_rem_node( alu, &num );
	
	ck_assert( num == 0 );
}
END_TEST

START_TEST( test_alur_get_nodes )
{
	ck_assert( alu_upto(alu) > 0 );
	
	int ret;
	uint_t nodes[REG_COUNT] = {0}, n;
	
	ret = alur_get_nodes( alu, nodes, REG_COUNT, 0 );
	
	ck_assert_msg
	(
		ret == 0
		, "Error: %08X, %i, '%s'"
		, ret
		, ret
		, strerror(ret)
	);
	
	for ( n = 0; n < REG_COUNT; ++n )
	{
		ck_assert( nodes[n] != 0 );
	}
	
	alur_rem_nodes( alu, nodes, REG_COUNT );
	
	for ( n = 0; n < REG_COUNT; ++n )
	{
		ck_assert( nodes[n] == 0 );
	}
}
END_TEST

START_TEST( test_alub )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t num = alur_get_node( alu, 0 );
	uintmax_t *N;
	alub_t n;
	
	ck_assert( num != 0 );
	
	N = (void*)alu_Ndata( alu, num );
	
	n = alub( N, _i );

	ck_assert( n.bit == (size_t)_i );
	ck_assert( n.seg == (size_t)(_i / bitsof(uintmax_t)) );
	ck_assert( n.pos == (size_t)(_i % bitsof(uintmax_t)) );
	ck_assert( n.ptr == (void*)(N + (_i / bitsof(uintmax_t))) );
	ck_assert( n.mask == 1uLL << (_i % bitsof(uintmax_t)) );
	
	alur_rem_node( alu, &num );
}
END_TEST

START_TEST( test_alub_inc )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t i = _i;
	alub_t n, v;
	
	n = alub( &i, i );
	v = alub( &i, ++i );
	alub_inc(&n);
	
	ck_assert( n.bit == i );
	ck_assert( n.seg == (i / bitsof(uintmax_t)) );
	ck_assert( n.pos == (i % bitsof(uintmax_t)) );
	ck_assert_msg
	(
		n.ptr == v.ptr
		, "Expected %p, Got %p"
		, (void*)(v.ptr)
		, (void*)(n.ptr)
	);
	ck_assert( n.mask == v.mask );
}
END_TEST

START_TEST( test_alub_dec )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t i = _i;
	alub_t n, v;
	
	n = alub( &i, i );
	v = alub( &i, --i );
	alub_dec(&n);
	
	ck_assert( n.bit == i );
	ck_assert( n.seg == (i / bitsof(uintmax_t)) );
	ck_assert( n.pos == (i % bitsof(uintmax_t)) );
	ck_assert_msg
	(
		n.ptr == v.ptr
		, "Expected %p, Got %p"
		, (void*)(v.ptr)
		, (void*)(n.ptr)
	);
	ck_assert( n.mask == v.mask );
}
END_TEST

START_TEST( test_alup_first_bit_with_val )
{
	if ( !stop_checks )
	{
		uintmax_t before = 0x86 << _i, expect = 0x2 << _i;
		alup_t _BEFORE;
		alub_t result;
		
		alup_init_unsigned( _BEFORE, &before, bitsof(uint_t) );
		
		result = alup_first_bit_with_val( &_BEFORE, true );
		
		stop_checks = ((*(result.ptr) & result.mask) != expect);
		ck_assert_msg
		(
			(*(result.ptr) & result.mask) == expect
			, "Expected 0x%016jX, Got 0x%016jX, Before = 0x%016jX"
			, expect
			, result.mask
			, before
		);
		
		expect = _i + 1;
		stop_checks = (stop_checks || result.pos != expect);
		ck_assert_msg
		(
			result.pos == expect
			, "Expected %ju, Got %zu"
			, expect
			, result.pos
		);
		
		
		stop_checks = (stop_checks || result.bit != expect);
		ck_assert_msg
		(
			result.bit == expect
			, "Expected %ju, Got %zu"
			, expect
			, result.bit
		);
	}
}
END_TEST

START_TEST( test_alup_final_bit_with_val )
{
	if ( !stop_checks )
	{
		uintmax_t before = 0x86 << _i, expect = 0x80 << _i;
		alup_t _BEFORE;
		alub_t result;
		
		alup_init_unsigned( _BEFORE, &before, bitsof(uint_t) );
		
		result = alup_final_bit_with_val( &_BEFORE, true );
		
		stop_checks = ((*(result.ptr) & result.mask) != expect);
		ck_assert_msg
		(
			(*(result.ptr) & result.mask) == expect
			, "Expected 0x%016jX, Got 0x%016jX, Before = 0x%016jX"
			, expect
			, result.mask
			, before
		);
		
		expect = _i + 7;
		stop_checks = (stop_checks || result.pos != expect);
		ck_assert_msg
		(
			result.pos == expect
			, "Expected %ju, Got %zu"
			, expect
			, result.pos
		);
		
		
		stop_checks = (stop_checks || result.bit != expect);
		ck_assert_msg
		(
			result.bit == expect
			, "Expected %ju, Got %zu"
			, expect
			, result.bit
		);
	}
}
END_TEST

START_TEST( test_aluv_release )
{
	aluv_release( alu, 0 );
	
	ck_assert( alu->Nsize == 0 );
	ck_assert( alu->taken == 0 );
	ck_assert( alu->given == 0 );
	ck_assert( alu->block.block == NULL );
	ck_assert( alu->block.given == 0 );
	ck_assert( alu->block.taken == 0 );
	ck_assert( alu->block.fault == 0 );
}
END_TEST

START_TEST( test_alub_set )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t num[] = {0,0}, val[] = {0,0}, i = _i;
	
	if ( i >= bitsof(uintmax_t) )
		num[1] = 1uLL << _i % bitsof(uintmax_t);
	else
		num[0] = 1uLL << _i;
		
	alub_set( val, _i, 1 );
	
	ck_assert_msg
	(
		memcmp( num, val, sizeof(uintmax_t) * 2 ) == 0
		, "Expected 0x%016jX%016jX, Got 0x%016jX%016jX"
		, num[0], num[1]
		, val[0], val[1]
	);
}
END_TEST

START_TEST( test_alup_mov_int2int )
{
	if ( !stop_checks )
	{
		alup_t _EXPECT, _RESULT, _RVALUE, _EVALUE;
		ulong_t expect = _i, result = 0;
		
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		
		_RVALUE = _RESULT;
		_EVALUE = _EXPECT;
		
		alup_mov_int2int( &_RESULT, &_EXPECT );
		
		ck_assert( memcmp( &_RESULT, &_RVALUE, sizeof(alup_t) ) == 0 );
		ck_assert( memcmp( &_EXPECT, &_EVALUE, sizeof(alup_t) ) == 0 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			//alup_init_floating( _EXPECT, &expect, bitsof(ulong_t) );
			
			alu_printf
			(
				"Expect %lu, Result %lu"
				, expect
				, result
			);
			
			alup_print( &_RESULT, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_mov_neg2int )
{
	if ( !stop_checks )
	{
		alup_t _EXPECT, _RESULT, _RVALUE, _EVALUE;
		long_t expect = -_i, result = 0;
		
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		
		_RVALUE = _RESULT;
		_EVALUE = _EXPECT;
		
		alup_mov_int2int( &_RESULT, &_EXPECT );
		
		ck_assert( memcmp( &_RESULT, &_RVALUE, sizeof(alup_t) ) == 0 );
		ck_assert( memcmp( &_EXPECT, &_EVALUE, sizeof(alup_t) ) == 0 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			//alup_init_floating( _EXPECT, &expect, bitsof(ulong_t) );
			
			alu_printf
			(
				"Expect %ld, Result %ld"
				, expect
				, result
			);
			
			alup_print( &_RESULT, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_mov_flt2flt )
{	
	if ( !stop_checks )
	{
		alup_t _BEFORE, _RESULT;
		double before = _i * 1.0e+100;
		float expect = before, result = 0;
		
		alup_init_floating( _BEFORE, &before, bitsof(double) );
		alup_init_floating( _RESULT, &result, bitsof(float) );
		
		alup_mov_flt2flt( &_RESULT, &_BEFORE );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _EXPECT;
			alup_init_floating( _EXPECT, &expect, bitsof(float) );
			
			alu_printf
			(
				"%d, %f, Expect %f, Result %f"
				, _i
				, before
				, expect
				, result
			);
			
			alup_print( &_BEFORE, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_mov_int2flt )
{	
	if ( !stop_checks )
	{
		alup_t _EXPECT, _RESULT, _BEFORE, _RPRIOR, _EPRIOR;
		ulong_t before = _i;
		float expect = before, result = 0;
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _EXPECT, &expect, bitsof(float) );
		alup_init_unsigned( _BEFORE, &before, bitsof(ulong_t) );
		
		_RPRIOR = _RESULT;
		_EPRIOR = _EXPECT;
		
		alup_mov_int2flt( &_RESULT, &_BEFORE );
		
		ck_assert( memcmp( &_RESULT, &_RPRIOR, sizeof(alup_t) ) == 0 );
		ck_assert( memcmp( &_EXPECT, &_EPRIOR, sizeof(alup_t) ) == 0 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			//alup_init_floating( _EXPECT, &expect, bitsof(float) );
			
			alu_printf
			(
				"Before = %lu, Expect %f, Result %f"
				, before
				, expect
				, result
			);
			
			alup_print( &_RESULT, 1, 1 );
			alup_print( &_EXPECT, 1, 1 );
			alup_print( &_BEFORE, 1, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_mov_neg2flt )
{	
	if ( !stop_checks )
	{
		alup_t _EXPECT, _RESULT, _BEFORE, _RPRIOR, _EPRIOR;
		long_t before = -((long_t)_i);
		float expect = before, result = 0;
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _EXPECT, &expect, bitsof(float) );
		alup_init___signed( _BEFORE, &before, bitsof(long_t) );
		
		_RPRIOR = _RESULT;
		_EPRIOR = _EXPECT;
		
		alup_mov_int2flt( &_RESULT, &_BEFORE );
		
		ck_assert( memcmp( &_RESULT, &_RPRIOR, sizeof(alup_t) ) == 0 );
		ck_assert( memcmp( &_EXPECT, &_EPRIOR, sizeof(alup_t) ) == 0 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			//alup_init_floating( _EXPECT, &expect, bitsof(float) );
			
			alu_printf
			(
				"Before = %ld, Expect %f, Result %f"
				, before
				, expect
				, result
			);
			
			alup_print( &_RESULT, 1, 1 );
			alup_print( &_EXPECT, 1, 1 );
			alup_print( &_BEFORE, 1, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

char* text_not() { return "~"; }
char* text_inc() { return "++"; }
char* text_dec() { return "--"; }
char* text_neg() { return "-"; }
char* text_and() { return "&"; }
char* text__or() { return "|"; }
char* text_xor() { return "^"; }
char* text_shl() { return "<<"; }
char* text_shr() { return ">>"; }
char* text_rol() { return "<<<"; }
char* text_ror() { return ">>>"; }
char* text_add() { return "+"; }
char* text_sub() { return "-"; }
char* text_mul() { return "*"; }
char* text_div() { return "/"; }
char* text_rem() { return "%"; }

long_t long_not( long_t num ) { return ~num; }
long_t long_inc( long_t num ) { return ++num; }
long_t long_dec( long_t num ) { return --num; }
long_t long_neg( long_t num ) { return -num; }
long_t long_and( long_t num, long_t val ) { return num & val; }
long_t long__or( long_t num, long_t val ) { return num | val; }
long_t long_xor( long_t num, long_t val ) { return num ^ val; }
/* Why are the instructions implemented incorrectly? */
long_t long_shl( long_t num, long_t val )
	{ return (size_t)val < bitsof(long_t) ? num << val : 0; }
long_t long_shr( long_t num, long_t val )
	{ return (size_t)val < bitsof(long_t) ? num >> val : 0; }
long_t long_rol( long_t num, long_t val ) { return rol( num, val ); }
long_t long_ror( long_t num, long_t val ) { return ror( num, val ); }
long_t long_add( long_t num, long_t val ) { return num + val; }
long_t long_sub( long_t num, long_t val ) { return num - val; }
long_t long_mul( long_t num, long_t val ) { return num * val; }
long_t long_div( long_t num, long_t val ) { return val ? num / val : 0; }
long_t long_rem( long_t num, long_t val ) { return val ? num % val : num; }

float flt_inc( float num ) { return ++num; }
float flt_dec( float num ) { return --num; }
float flt_neg( float num ) { return -num; }
float flt_add( float num, float val ) { return num + val; }
float flt_sub( float num, float val ) { return num - val; }
float flt_mul( float num, float val ) { return num * val; }
float flt_div( float num, float val ) { return val ? num / val : 0; }

typedef char*	(*func_text_op1_t)();
typedef long_t	(*func_long_op1_t)( long_t num );
typedef float	(*func_flt_op1_t)( float num );
typedef int_t	(*func_alup_op1_t)( alup_t const * const _NUM );
typedef char*	(*func_text_op2_t)();
typedef long_t	(*func_long_op2_t)( long_t num, long_t val );
typedef float	(*func_flt_op2_t)( float num, float val );
typedef int_t	(*func_alup_op2_t)( alup_t const * const _NUM, alup_t const * const _VAL );
typedef char*	(*func_text_op4_t)();
typedef long_t	(*func_long_op4_t)( long_t num, long_t val );
typedef float	(*func_flt_op4_t)( float num, float val );
typedef int_t	(*func_alup_op4_t)( alup_t const * const _NUM, alup_t const * const _VAL, void *cpy, void *tmp );

func_text_op1_t	text_op1[] = { text_not, text_inc, text_dec, text_neg, NULL };
func_alup_op1_t	alup_op1[] = { alup_not, alup_inc, alup_dec, alup_neg, NULL };
func_long_op1_t	long_op1[] = { long_not, long_inc, long_dec, long_neg, NULL };
func_flt_op1_t	flt_op1[] = { NULL, flt_inc, flt_dec, flt_neg, NULL };
func_alup_op2_t	alup_op2[] = { alup_and, alup__or, alup_xor, alup_shl, alup_shr, alup_rol, alup_ror, NULL };
func_text_op2_t	text_op2[] = { text_and, text__or, text_xor, text_shl, text_shr, text_rol, text_ror, NULL };
func_long_op2_t	long_op2[] = { long_and, long__or, long_xor, long_shl, long_shr, long_rol, long_ror, NULL };
func_alup_op4_t	alup_op4[] = { alup__add, alup__sub, alup__mul, alup__div, NULL };
func_text_op4_t	text_op4[] = { text_add, text_sub, text_mul, text_div, text_rem, NULL };
func_long_op4_t	long_op4[] = { long_add, long_sub, long_mul, long_div, long_rem, NULL };
func_flt_op4_t	flt_op4[] = { flt_add, flt_sub, flt_mul, flt_div, NULL };

START_TEST( test_alup__cmp_integer_incremental )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		int expect, result;
		ulong_t value1 = _i, value2 = _i % per_func;
		alup_t _VALUE1, _VALUE2;
		
		alup_init_unsigned( _VALUE1, &value1, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		
		expect = (value1 > value2) - (value1 < value2);
		result = alup_cmp( &_VALUE1, &_VALUE2 );
		
		if ( expect != result )
		{	
			alu_printf
			(
				"%lu <> %lu = %i, got %i"
				, value2
				, value2
				, expect
				, result
			);

#if 1
			alup_print( &_VALUE1, 1, 1 );
			alup_print( &_VALUE2, 1, 1 );
#endif
			
			stop_checks = true;
		}
		
		ck_assert( expect == result );
	}
}
END_TEST

START_TEST( test_alup__cmp_integer_randomised )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		uint_t seed = time(NULL);
		int expect, result;
		ulong_t value1 = rand_r(&seed), value2 = rand_r(&seed);
		alup_t _VALUE1, _VALUE2;
		
		alup_init_unsigned( _VALUE1, &value1, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		
		expect = (value1 > value2) - (value1 < value2);
		result = alup_cmp( &_VALUE1, &_VALUE2 );
		
		if ( expect != result )
		{	
			alu_printf
			(
				"%lu <> %lu = %i, got %i"
				, value2
				, value2
				, expect
				, result
			);
			
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
			
			stop_checks = true;
		}
		
		ck_assert( expect == result );
	}
}
END_TEST

START_TEST( test_alup__cmp_floating_incremental )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		int expect, result, _j = _i % per_func;
		float value1 = _i, value2 = _j;
		alup_t _VALUE1, _VALUE2;
		
		alup_init_floating( _VALUE1, &value1, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		expect = (value1 > value2) - (value1 < value2);
		result = alup_cmp( &_VALUE1, &_VALUE2 );
		
		if ( expect != result )
		{	
			alu_printf
			(
				"%f <> %f = %i (source: %i <> %i), got %i"
				, value2
				, value2
				, expect
				, _i
				, _j
				, result
			);
			
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
			
			stop_checks = true;
		}
		
		ck_assert( expect == result );
	}
}
END_TEST

START_TEST( test_alup__cmp_floating_randomised )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		uint_t seed = time(NULL);
		int expect, result;
		float value1 = rand_r(&seed), value2 = rand_r(&seed);
		alup_t _VALUE1, _VALUE2;
		
		alup_init_floating( _VALUE1, &value1, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		expect = (value1 > value2) - (value1 < value2);
		result = alup_cmp( &_VALUE1, &_VALUE2 );
		
		if ( expect != result )
		{	
			alu_printf
			(
				"%f <> %f = %i, got %i"
				, value2
				, value2
				, expect
				, result
			);
			
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
			
			stop_checks = true;
		}
		
		ck_assert( expect == result );
	}
}
END_TEST

START_TEST( test_alup_op1_integer_incremental )
{
	size_t func = _i / per_func;
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks && long_op1[func] )
	{
		ulong_t before = _i, expect = before, result = before;
		alup_t _RESULT, _EXPECT;
		
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		
		expect = long_op1[func]( expect );
		alup_op1[func]( &_RESULT );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _BEFORE;
			
			alup_init_unsigned( _BEFORE, &before, bitsof(ulong_t) );
			
			alu_printf
			(
				"%d, %s0x%lX = 0x%lX, got 0x%lX"
				, _i
				, text_op1[func]()
				, before
				, expect
				, result
			);
			
			alup_print( &_BEFORE, 0, 1 );
			alup_print( &_EXPECT, 1, 1 );
			alup_print( &_RESULT, 1, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_op1_floating_incremental )
{
	size_t func = _i / per_func;
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks && flt_op1[func] )
	{
		float before = _i % per_func, expect = before, result = before;
		alup_t _RESULT, _EXPECT;
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _EXPECT, &expect, bitsof(float) );
		
		expect = flt_op1[func]( expect );
		alup_op1[func]( &_RESULT );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{	
			int testno = _i;
			alup_t _BEFORE, _TESTNO;
			
			alup_init_floating( _BEFORE, &before, bitsof(float) );
			alup_init___signed( _TESTNO, &testno, bitsof(int) );
			
			alu_printf
			(
				"%d, %s(%f) = %f, got %f"
				, _i
				, text_op1[func]()
				, before
				, expect
				, result
			);
			
			alup_print( &_TESTNO, 0, 1 );
			alup_print( &_BEFORE, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_op1_floating_randomised )
{
	size_t func = _i / per_func;
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks && flt_op1[func] )
	{
		uint_t seed = time(NULL);
		float before = rand_r(&seed), expect = before, result = before;
		alup_t _RESULT, _EXPECT;
		
		alup_init_floating( _EXPECT, &expect, bitsof(float) );
		alup_init_floating( _RESULT, &result, bitsof(float) );

		expect = flt_op1[func]( expect );
		alup_op1[func]( &_RESULT );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _BEFORE;
			
			alup_init_floating( _BEFORE, &before, bitsof(float) );
			
			alu_printf
			(
				"%d, %s%f = %f, got %f"
				, _i
				, text_op1[func]()
				, before
				, expect
				, result
			);
			
			alup_print( &_BEFORE, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup_op1_integer_randomised )
{
	size_t func = _i / per_func;
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks && long_op1[func] )
	{
		uint_t seed = time(NULL);
		ulong_t before = rand_r(&seed), expect = before, result = before;
		alup_t _RESULT, _EXPECT;
		
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );

		expect = long_op1[func]( expect );
		alup_op1[func]( &_RESULT );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{	
			alu_printf
			(
				"%s%lu = %lu, got %lu"
				, text_op1[func]()
				, before
				, expect
				, result
			);
			
			//alup_print( _BEFORE, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op2_integer_incremental )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		ulong_t
			value1 = _i
			, value2 = _i % per_func
			, expect, result;
		alup_t _RESULT, _VALUE2, _EXPECT;
		
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		
		result = value2;
		expect = long_op2[func]( value2, value2 );
		alup_op2[func]( &_RESULT, &_VALUE2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{	
			alup_t _VALUE1;
			
			alup_init_unsigned( _VALUE1, &value1, bitsof(ulong_t) );
			
			alu_printf
			(
				"%lu %s %lu = %lu, got %lu"
				, value2
				, text_op2[func]()
				, value2
				, expect
				, result
			);
			
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
		
		result = value1;
		expect = long_op2[func]( value1, value2 );
		alup_op2[func]( &_RESULT, &_VALUE2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{	
			alu_printf
			(
				"%lu %s %lu = %lu, got %lu"
				, value1
				, text_op2[func]()
				, value2
				, expect
				, result
			);

			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op2_integer_randomised )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		uint_t seed = time(NULL);
		ulong_t 
			value1 = rand_r(&seed)
			, value2 = rand_r(&seed)
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		
		result = value1;
		expect = long_op2[func]( value1, value2 );
		alup_op2[func]( &_RESULT, &_VALUE2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _EXPECT;
			
			alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
			
			alu_printf
			(
				"0x%08lX %s 0x%08lX = 0x%016lX, got %lu"
				, value1
				, text_op2[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op4_integer_incremental )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		ulong_t extra1, extra2
			, value1 = _i
			, value2 = _i % per_func
			, expect, result;
		alup_t _RESULT, _VALUE2, _EXPECT;
		
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
		
		result = value2;
		expect = long_op4[func]( value2, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{	
			alu_printf
			(
				"%lu %s %lu = %lu, got %lu"
				, value2
				, text_op4[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _BEFORE, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
		
		result = value1;
		expect = long_op4[func]( value1, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{	
			alu_printf
			(
				"%lu %s %lu = %lu, got %lu"
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);

			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op4_integer_randomised )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		uint_t seed = time(NULL);
		ulong_t extra1, extra2
			, value1 = rand_r(&seed)
			, value2 = rand_r(&seed)
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_unsigned( _RESULT, &result, bitsof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, bitsof(ulong_t) );
		
		result = value1;
		expect = long_op4[func]( value1, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _EXPECT;
			
			alup_init_unsigned( _EXPECT, &expect, bitsof(ulong_t) );
			
			alu_printf
			(
				"%lu %s %lu = %lu, got %lu"
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op4_floating_incremental )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		float extra1[2], extra2[2]
			, value1 = _i % per_func
			, value2 = _i
			, expect, result;
		alup_t _RESULT, _VALUE2, _EXPECT, _VALUE1;
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _EXPECT, &expect, bitsof(float) );
		alup_init_floating( _VALUE1, &value1, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		result = value1;
		expect = flt_op4[func]( value1, value2 );
		
		alup_op4[func]( &_RESULT, &_VALUE2, extra1, extra2 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _EXP, _MAN;
			
			alup_init_exponent( &_VALUE1, _EXP );
			alup_init_mantissa( &_VALUE1, _MAN );
			
			alu_printf
			(
				"%zu: %f %s %f = %f, got %f"
				, func
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);
			
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op4_floating_negative )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		float extra1[2], extra2[2]
			, value1 = _i
			, value2 = -value1
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		result = value1;
		expect = flt_op4[func]( value1, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, extra1, extra2 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, bitsof(float) );
			alup_init_floating( _VALUE1, &value1, bitsof(float) );
			alup_init_exponent( &_VALUE1, _EXP );
			alup_init_mantissa( &_VALUE1, _MAN );
			
			alu_printf
			(
				"%f %s %f = %f, got %f"
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);
#if 1
			alup_print( &_VALUE1, 0, 1 );
			alup_print( &_VALUE2, 0, 1 );
#endif
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__op4_floating_randomised )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i / per_func;
		uint_t seed = time(NULL);
		float extra1, extra2
			, value1 = rand_r(&seed)
			, value2 = rand_r(&seed)
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alu_puts("floating randomised");
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		result = value1;
		expect = flt_op4[func]( value1, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, bitsof(float) );
			alup_init_floating( _VALUE1, &value1, bitsof(float) );
			alup_init_exponent( &_VALUE1, _EXP );
			alup_init_mantissa( &_VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %s %f = %f, got %f"
				, _EXP.bits
				, _MAN.bits
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

const int abs_count = 2;

const float abs_float1[] =
{
	1397677567.0,
	1327753883.0
};
const float abs_float2[] =
{
	797987423.0,
	1770012269.0
};

START_TEST( test_alup__op4_floating_absolute )
{
	ck_assert( alu_upto(alu) > 0 );
	
	if ( !stop_checks )
	{
		size_t func = _i % 4, val = _i % abs_count;
		float extra1[2], extra2[2]
			, value1
			, value2
			, expect
			, result
		;
		alup_t _RESULT, _VALUE2;
		
		value1 = abs_float1[val];
		value2 = abs_float2[val];
		
		alup_init_floating( _RESULT, &result, bitsof(float) );
		alup_init_floating( _VALUE2, &value2, bitsof(float) );
		
		result = value1;
		expect = flt_op4[func]( value1, value2 );
		alup_op4[func]( &_RESULT, &_VALUE2, extra1, extra2 );
		
		if ( memcmp( &result, &expect, sizeof(float) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, bitsof(float) );
			alup_init_floating( _VALUE1, &value1, bitsof(float) );
			alup_init_exponent( &_VALUE1, _EXP );
			alup_init_mantissa( &_VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %s %f = %f, got %f"
				, _EXP.bits
				, _MAN.bits
				, value1
				, text_op4[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( &_EXPECT, 0, 1 );
			alup_print( &_RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(float) ) == 0 );
	}
}
END_TEST

//#define TEST_SQUARE_ROOT_ALGORITHM
#ifdef TEST_SQUARE_ROOT_ALGORITHM
#include "square_root.c"
#endif

/* Modified copy & paste of code given in check's guide */
Suite * alu_suite(void)
{
	Suite *s;
	TCase *tc_core;
	//uint_t f = 0;

	s = suite_create("ALU");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test( tc_core, test_alur_ensure );
	tcase_add_test( tc_core, test_alur_get_node );
	tcase_add_test( tc_core, test_alur_get_nodes );
	
	tcase_add_loop_test( tc_core, test_alub, 0, bitsof(uintmax_t) - 1 );
	tcase_add_loop_test( tc_core, test_alub_set, 0, (bitsof(uintmax_t) * 2) );
	tcase_add_loop_test( tc_core, test_alub_inc, 0, bitsof(uintmax_t) - 1 );
	tcase_add_loop_test( tc_core, test_alub_dec, 1, bitsof(uintmax_t) );
	
	/* */
	tcase_add_loop_test( tc_core, test_alup_first_bit_with_val, 0, 3 );
	tcase_add_loop_test( tc_core, test_alup_final_bit_with_val, 1, 3 );

	/*
	 * Plan to patent the method if I get it working, avoids the incrementation
	 * method, testing heere because the alup functions are useful for seeing
	 * various data.
	 */
#ifdef TEST_SQUARE_ROOT_ALGORITHM
	tcase_add_loop_test( tc_core, squareX, 0, 100 );
#endif

#ifndef TEST_SQUARE_ROOT_ALGORITHM
	tcase_add_loop_test( tc_core, test_alup_mov_int2int, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_mov_neg2int, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup__cmp_integer_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__cmp_integer_randomised, 0, per_func * 4 );
	
	tcase_add_loop_test( tc_core, test_alup_op1_integer_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup_op1_integer_randomised, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__op2_integer_incremental, 0, per_func * 5 );
	tcase_add_loop_test( tc_core, test_alup__op2_integer_randomised, 0, per_func * 5 );
	tcase_add_loop_test( tc_core, test_alup__op4_integer_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__op4_integer_randomised, 0, per_func * 4 );
	
	tcase_add_loop_test( tc_core, test_alup_mov_flt2flt, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_mov_int2flt, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_mov_neg2flt, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup__cmp_floating_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__cmp_floating_randomised, 0, per_func * 4 );
	
	tcase_add_loop_test( tc_core, test_alup_op1_floating_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup_op1_floating_randomised, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__op4_floating_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__op4_floating_negative, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__op4_floating_absolute, 0, abs_count * 4 );
	tcase_add_loop_test( tc_core, test_alup__op4_floating_randomised, 0, per_func * 4 );

	tcase_add_test( tc_core, test_alur2str );
	tcase_add_test( tc_core, test_aluv_release );
#endif

	suite_add_tcase( s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	alu_t _alu = {0};
	alu = &_alu;

	s = alu_suite();
	sr = srunner_create(s);
	
	srunner_set_fork_status(sr,CK_NOFORK);
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
