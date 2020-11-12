#include <alu.h>
#include <stdlib.h>
#include <time.h>

alu_t *alu = NULL;

#define REG_COUNT 3

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

START_TEST( test_alur_set_raw )
{
	ck_assert( alu_upto( alu ) > 0 );
	
	uint_t num = alur_get_node( alu, 0 );
	uintmax_t val = _i, got;
	alur_t NUM;
	bool active = alu_get_active( alu, num );
	
	ck_assert( num != 0 );
	ck_assert( active == true );
	
	alur_init_unsigned( alu, NUM, num );
	NUM.upto = bitsof(uintmax_t);
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );

	alur_set_raw( alu, NUM, &val, sizeof(uintmax_t), 0 );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	
	alur_get_raw( alu, NUM, &got, sizeof(uintmax_t), 0 );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	
	ck_assert_msg
	(
		val == got
		, "Expected %ju, Got %ju"
		, val
		, got
	);
	
	alur_rem_node( alu, &num );
}
END_TEST

START_TEST( test_alu_uint_set_raw )
{
	uint_t num = alur_get_node( alu, 0 );
	uintmax_t val = _i, got;
	
	ck_assert( num != 0 );

	alu_uint_set_raw( alu, num, val );
	alu_uint_get_raw( alu, num, &got );
	
	ck_assert_msg
	(
		val == got
		, "Expected %ju, Got %ju"
		, val
		, got
	);
	
	alur_rem_node( alu, &num );
}
END_TEST

START_TEST( test_alur_final_one )
{
	uint_t num = alur_get_node( alu, 0 );
	size_t bit = 0;
	void *N;
	alur_t NUM;
	alub_t n, v;
	
	ck_assert( num != 0 );
	
	alur_init_unsigned( alu, NUM, num );
	/* Ensure any realloc occurs BEFORE we get our pointer - prevents it
	 * changing during the following loop */
	alu_set_raw( alu, num, 0, 0 );
	
	N = alu_Ndata( alu, num );
	
	while ( bit < bitsof(uintmax_t) )
	{	
		v = alub( N, bit++ );
		
		alu_set_raw( alu, num, v.mask, 0 );
		
		n = alur_final_one( alu, NUM );
		
		ck_assert_msg
		(
			n.bit == v.bit
			, "Expected %zu, Got %zu"
			, v.bit
			, n.bit
		);
		ck_assert( n.pos == v.pos );
		ck_assert( n.seg == v.seg );
		ck_assert( n.mask == v.mask );
		ck_assert_msg
		(
			n.ptr == v.ptr
			, "Expected %p, Got %p, Val = %c, %c"
			, (void*)(v.ptr)
			, (void*)(n.ptr)
			, '0' + !!(*(n.ptr) & n.mask)
			, '0' + !!(*(v.ptr) & v.mask)
		);
	}
	
	alur_rem_node( alu, &num );
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

START_TEST( test_alur_set_floating )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t nodes[2], num, val;
	ullong_t _num = _i, _val = 1;
	int ret = alur_get_nodes( alu, nodes, 2, 0 );
	double src_num = _num, src_val = _val, got_num, got_val;
	alur_t NUM, VAL;
	void *N, *V;
	
	ck_assert( ret == 0 );
	
	num = nodes[0];
	val = nodes[1];
	
	alur_init_floating( alu, NUM, num );
	alur_init_floating( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(double);
	
	/* Set values */
	
	ret = alur_set_raw( alu, NUM, &src_num, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	ret = alur_set_raw( alu, VAL, &src_val, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	/* Get values */
	
	ret = alur_get_raw( alu, NUM, &got_num, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	ret = alur_get_raw( alu, VAL, &got_val, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	N = alur_data( alu, NUM );
	V = alur_data( alu, VAL );
	
	ck_assert( memcmp( N, &src_num, sizeof(double) ) == 0 );
	ck_assert( memcmp( V, &src_val, sizeof(double) ) == 0 );
	
	/* Set values */
	
	ret = alur_set_raw( alu, NUM, &_num, sizeof(ullong_t), 0 );
	ck_assert( ret == 0 );
	
	ret = alur_set_raw( alu, VAL, &_val, sizeof(ullong_t), 0 );
	ck_assert( ret == 0 );
	
	/* Get values */
	
	ret = alur_get_raw( alu, NUM, &got_num, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	ret = alur_get_raw( alu, VAL, &got_val, sizeof(double), ALU_INFO_FLOAT );
	ck_assert( ret == 0 );
	
	N = alur_data( alu, NUM );
	V = alur_data( alu, VAL );
	
	//ck_assert( memcmp( N, &src_num, sizeof(double) ) == 0 );
	//ck_assert( memcmp( V, &src_val, sizeof(double) ) == 0 );
	
	if
	(
		memcmp( N, &src_num, sizeof(double) ) != 0
		|| memcmp( V, &src_val, sizeof(double) ) != 0
	)
	{
		alur_t EXP, MAN;
		
		alur_init_exponent( NUM, EXP );
		alur_init_mantissa( NUM, MAN );
		
		alu_printf
		(
			"_num = %llu, src_num = %e, _val = %llu, src_val = %e\n"
			"(EXP.upto = %zu) - (EXP.from = %zu) = %zu, "
			"(MAN.upto = %zu) - (MAN.from = %zu) = %zu"
			, _num, src_num
			, _val, src_val
			, EXP.upto, EXP.from, EXP.upto - EXP.from
			, MAN.upto, MAN.from, MAN.upto - MAN.from
		);
		
		alur_print( alu, NUM, 0, 1 );
		(void)memcpy( alur_data( alu, NUM ), &src_num, sizeof(double) );
		alur_print( alu, NUM, 0, 1 );
		
		alur_print( alu, VAL, 0, 1 );
		(void)memcpy( alur_data( alu, VAL ), &src_val, sizeof(double) );
		alur_print( alu, VAL, 0, 1 );
		
		ck_assert( 1 == 0 );
	}
	
	alur_rem_nodes( alu, nodes, 2 );
}
END_TEST

typedef int_t (*func_alup__math_t)
(
	alup_t _NUM
	, alup_t _VAL
	, void *cpy
	, void *tmp
);

func_alup__math_t alup__math[] =
{
	alup__add
	, alup__sub
	, alup__mul
	, alup__div
	, NULL
};

int op_add() { return '+'; }
int op_sub() { return '-'; }
int op_mul() { return '*'; }
int op_div() { return '/'; }
int op_rem() { return '%'; }

typedef int (*func_op_math_t)();

func_op_math_t op_math[] =
{
	op_add
	, op_sub
	, op_mul
	, op_div
	, op_rem
	, NULL
};

ulong_t int_add( ulong_t num, ulong_t val ) { return num + val; }
ulong_t int_sub( ulong_t num, ulong_t val ) { return num - val; }
ulong_t int_mul( ulong_t num, ulong_t val ) { return num * val; }
ulong_t int_div( ulong_t num, ulong_t val ) { return val ? num / val : 0; }
ulong_t int_rem( ulong_t num, ulong_t val ) { return val ? num % val : num; }

typedef ulong_t (*func_int_math_t)( ulong_t num, ulong_t val );

func_int_math_t int_math[] =
{
	int_add
	, int_sub
	, int_mul
	, int_div
	, int_rem
	, NULL
};

double flt_add( double num, double val ) { return num + val; }
double flt_sub( double num, double val ) { return num - val; }
double flt_mul( double num, double val ) { return num * val; }
double flt_div( double num, double val ) { return val ? num / val : 0; }
//double flt_rem( double num, double val ) { return val ? num % val : num; }

typedef double (*func_flt_math_t)( double num, double val );

func_flt_math_t flt_math[] =
{
	flt_add
	, flt_sub
	, flt_mul
	, flt_div
	, NULL
};

const int per_func = LDBL_MANT_DIG;
bool_t stop_checks = false;

START_TEST( test_alup__math_integer_incremental )
{
	if ( !stop_checks )
	{
		ck_assert( alu_upto(alu) > 0 );
		size_t func = _i / per_func;
		ulong_t extra1, extra2
			, value1 = _i
			, value2 = _i % per_func
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_unsigned( _RESULT, &result, sizeof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, sizeof(ulong_t) );
		
		result = value2;
		expect = int_math[func]( value2, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _EXPECT;
			
			alup_init_unsigned( _EXPECT, &expect, sizeof(ulong_t) );
			
			alu_printf
			(
				"%lu %c %lu = %lu, got %lu"
				, value2
				, op_math[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
		
		result = value1;
		expect = int_math[func]( value1, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _EXPECT;
			
			alup_init_unsigned( _EXPECT, &expect, sizeof(ulong_t) );
			
			alu_printf
			(
				"%lu %c %lu = %lu, got %lu"
				, value1
				, op_math[func]()
				, value2
				, expect
				, result
			);

			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__math_integer_randomised )
{
	if ( !stop_checks )
	{
		ck_assert( alu_upto(alu) > 0 );
		size_t func = _i / per_func;
		uint_t seed = time(NULL);
		ulong_t extra1, extra2
			, value1 = rand_r(&seed)
			, value2 = rand_r(&seed)
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_unsigned( _RESULT, &result, sizeof(ulong_t) );
		alup_init_unsigned( _VALUE2, &value2, sizeof(ulong_t) );
		
		result = value1;
		expect = int_math[func]( value1, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(ulong_t) ) != 0 )
		{
			alup_t _EXPECT;
			
			alup_init_unsigned( _EXPECT, &expect, sizeof(ulong_t) );
			
			alu_printf
			(
				"%lu %c %lu = %lu, got %lu"
				, value1
				, op_math[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(ulong_t) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__math_floating_incremental )
{
	if ( !stop_checks )
	{
		ck_assert( alu_upto(alu) > 0 );
		size_t func = _i / per_func;
		double extra1, extra2
			, value1 = _i
			, value2 = _i % per_func
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_floating( _RESULT, &result, sizeof(double) );
		alup_init_floating( _VALUE2, &value2, sizeof(double) );
		
		result = value2;
		expect = flt_math[func]( value2, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(double) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, sizeof(double) );
			alup_init_floating( _VALUE1, &value1, sizeof(double) );
			alup_init_exponent( _VALUE1, _EXP );
			alup_init_mantissa( _VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %c %f = %f, got %f"
				, _EXP.upto - _EXP.from
				, _MAN.upto - _MAN.from
				, value2
				, op_math[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(double) ) == 0 );
		
		result = value1;
		expect = flt_math[func]( value1, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(double) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, sizeof(double) );
			alup_init_floating( _VALUE1, &value1, sizeof(double) );
			alup_init_exponent( _VALUE1, _EXP );
			alup_init_mantissa( _VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %c %f = %f, got %f"
				, _EXP.upto - _EXP.from
				, _MAN.upto - _MAN.from
				, value1
				, op_math[func]()
				, value2
				, expect
				, result
			);

			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(double) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__math_floating_randomised )
{
	if ( !stop_checks )
	{
		ck_assert( alu_upto(alu) > 0 );
		size_t func = _i / per_func;
		uint_t seed = time(NULL);
		double extra1, extra2
			, value1 = rand_r(&seed)
			, value2 = rand_r(&seed)
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_floating( _RESULT, &result, sizeof(double) );
		alup_init_floating( _VALUE2, &value2, sizeof(double) );
		
		result = value1;
		expect = flt_math[func]( value1, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(double) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, sizeof(double) );
			alup_init_floating( _VALUE1, &value1, sizeof(double) );
			alup_init_exponent( _VALUE1, _EXP );
			alup_init_mantissa( _VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %c %f = %f, got %f"
				, _EXP.upto - _EXP.from
				, _MAN.upto - _MAN.from
				, value1
				, op_math[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(double) ) == 0 );
	}
}
END_TEST

START_TEST( test_alup__math_floating_absolute )
{
	if ( !stop_checks )
	{
		ck_assert( alu_upto(alu) > 0 );
		size_t func = _i;
		double extra1, extra2
			, value1 = 1397677567.0
			, value2 = 797987423.0
			, expect, result;
		alup_t _RESULT, _VALUE2;
		
		alup_init_floating( _RESULT, &result, sizeof(double) );
		alup_init_floating( _VALUE2, &value2, sizeof(double) );
		
		result = value1;
		expect = flt_math[func]( value1, value2 );
		alup__math[func]( _RESULT, _VALUE2, &extra1, &extra2 );
		
		if ( memcmp( &result, &expect, sizeof(double) ) != 0 )
		{
			alup_t _EXP, _MAN, _EXPECT, _VALUE1;
			
			alup_init_floating( _EXPECT, &expect, sizeof(double) );
			alup_init_floating( _VALUE1, &value1, sizeof(double) );
			alup_init_exponent( _VALUE1, _EXP );
			alup_init_mantissa( _VALUE1, _MAN );
			
			alu_printf
			(
				"exp_dig = %zu, man_dig = %zu, %f %c %f = %f, got %f"
				, _EXP.upto - _EXP.from
				, _MAN.upto - _MAN.from
				, value1
				, op_math[func]()
				, value2
				, expect
				, result
			);
			
			//alup_print( _VALUE2, 0, 1 );
			alup_print( _EXPECT, 0, 1 );
			alup_print( _RESULT, 0, 1 );
			stop_checks = true;
		}
		
		ck_assert( memcmp( &result, &expect, sizeof(double) ) == 0 );
	}
}
END_TEST

/* Modified copy & paste of code given in check's guide */
Suite * alu_suite(void)
{
	Suite *s;
	TCase *tc_core;
	uint_t f = 0;

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

	tcase_add_loop_test( tc_core, test_alur_set_raw, 0, per_func );
	tcase_add_loop_test( tc_core, test_alu_uint_set_raw, 0, per_func );

	tcase_add_test( tc_core, test_alur_final_one );
	
	tcase_add_loop_test( tc_core, test_alur_set_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup__math_integer_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__math_integer_randomised, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__math_floating_incremental, 0, per_func * 4 );
	tcase_add_loop_test( tc_core, test_alup__math_floating_absolute, 0, 4 );
	tcase_add_loop_test( tc_core, test_alup__math_floating_randomised, 0, per_func * 4 );
	
	tcase_add_test( tc_core, test_alur2str );
	tcase_add_test( tc_core, test_aluv_release );
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
