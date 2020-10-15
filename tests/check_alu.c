#include <alu.h>
#include <stdlib.h>

alu_t *alu = NULL;

#define REG_COUNT 3

int wrChar( char32_t c, void *dst )
{
	alu_block_t *ALUSTR = dst;
	char *alustr = alu_block_expand( dst, ALUSTR->taken + 1 );
	
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
	alu_block_t *ALUSTR = dst;
	uint_t i, j;
	char c, *alustr = ALUSTR->block;
	
	for ( i = 0, j = ALUSTR->taken - 1; i < j; ++i, --j )
	{
		c = alustr[i];
		alustr[i] = alustr[j];
		alustr[j] = c;
	}
}

const char* op1_str_not() { return "~"; }
uintmax_t op1_ret_not( uintmax_t a ) { return ~a; }
const char* op1_str_inc() { return "++"; }
uintmax_t op1_ret_inc( uintmax_t a ) { return ++a; }
const char* op1_str_dec() { return "--"; }
uintmax_t op1_ret_dec( uintmax_t a ) { return --a; }

const char* op2_str_and() { return "&"; }
uintmax_t op2_ret_and( uintmax_t a, uintmax_t b ) { return a & b; }
const char* op2_str__or() { return "|"; }
uintmax_t op2_ret__or( uintmax_t a, uintmax_t b ) { return a | b; }
const char* op2_str_xor() { return "^"; }
uintmax_t op2_ret_xor( uintmax_t a, uintmax_t b ) { return a ^ b; }

const char* op3_str_shl() { return "<<"; }
uintmax_t op3_ret_shl( uintmax_t a, uintmax_t b ) { return a << b; }
const char* op3_str_shr() { return ">>"; }
uintmax_t op3_ret_shr( uintmax_t a, uintmax_t b ) { return a >> b; }
const char* op3_str_rol() { return "<<<"; }
uintmax_t op3_ret_rol( uintmax_t a, uintmax_t b ) { return unic_rol( a, b ); }
const char* op3_str_ror() { return ">>>"; }
uintmax_t op3_ret_ror( uintmax_t a, uintmax_t b ) { return unic_ror( a, b ); }

const char* op2_str_add() { return "+"; }
uintmax_t op2_ret_add( uintmax_t a, uintmax_t b ) { return a + b; }
const char* op2_str_mul() { return "*"; }
uintmax_t op2_ret_mul( uintmax_t a, uintmax_t b ) { return a * b; }
const char* op2_str_sub() { return "-"; }
uintmax_t op2_ret_sub( uintmax_t a, uintmax_t b ) { return a - b; }
const char* op2_str_div() { return "/"; }
uintmax_t op2_ret_div( uintmax_t a, uintmax_t b ) { return b ? a / b : 0; }
const char* op2_str_rem() { return "%"; }
uintmax_t op2_ret_rem( uintmax_t a, uintmax_t b ) { return b ? a % b : a; }

typedef const char* (*func_op1_str)();
typedef const char* (*func_op2_str)();
typedef const char* (*func_op3_str)();
typedef uintmax_t (*func_op1_ret)( uintmax_t a );
typedef uintmax_t (*func_op2_ret)( uintmax_t a, uintmax_t b );
typedef uintmax_t (*func_op3_ret)( uintmax_t a, uintmax_t b );

const uintmax_t ops_loop_until = bitsof(uintmax_t);

func_op1_str op1_str = NULL, op1_str_array[] =
{
	op1_str_not
	, op1_str_inc
	, op1_str_dec
	, NULL
};

func_op1_ret op1_ret = NULL, op1_ret_array[] =
{
	op1_ret_not
	, op1_ret_inc
	, op1_ret_dec
	, NULL
};

func_alu_reg_op1_t op1_reg = NULL, op1_reg_array[] =
{
	alu_reg_not
	, alu_reg_inc
	, alu_reg_dec
	, NULL
};

func_op3_str op3_str = NULL, op3_str_array[] =
{
	op3_str_shl
	, op3_str_shr
	, op3_str_rol
	, op3_str_ror
	, NULL
};

func_op3_ret op3_ret = NULL, op3_ret_array[] =
{
	op3_ret_shl
	, op3_ret_shr
	, op3_ret_rol
	, op3_ret_ror
	, NULL
};

func_alu_reg__shift_t op3__reg = NULL, op3__reg_array[] =
{
	alu_reg__shl
	, alu_reg__shr
	, alu_reg__rol
	, alu_reg__ror
	, NULL
};

func_alu_reg_shift_t op3_reg = NULL, op3_reg_array[] =
{
	alu_reg__shift
	, alu_reg__shift
	, alu_reg__rotate
	, alu_reg__rotate
	, NULL
};

func_op2_str op2_str = NULL, op2_str_array[] =
{
	op2_str_and
	, op2_str__or
	, op2_str_xor
	, op2_str_add
	, op2_str_mul
	, op2_str_sub
	, op2_str_div
	, op2_str_rem
	, NULL
};

func_op2_ret op2_ret = NULL, op2_ret_array[] =
{
	op2_ret_and
	, op2_ret__or
	, op2_ret_xor
	, op2_ret_add
	, op2_ret_mul
	, op2_ret_sub
	, op2_ret_div
	, op2_ret_rem
	, NULL
};

func_alu_reg_op2_t op2_reg = NULL, op2_reg_array[] =
{
	alu_reg_and
	, alu_reg__or
	, alu_reg_xor
	, alu_reg_add
	, alu_reg_mul
	, alu_reg_sub
	, alu_reg_div
	, alu_reg_rem
	, NULL
};

#include <check.h>

START_TEST( test_alu_setup_reg )
{
	int ret;
	uint_t want = bitsof(uintmax_t);

	ret = alu_setup_reg( alu, want, 0 );
	
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

START_TEST( test_alu_reg_cmp )
{
	int ret, expect;
	uint_t nodes[REG_COUNT], num, val;
	alu_reg_t NUM, VAL;
	intmax_t cmp_n = ops_loop_until, cmp_v = _i, got_n, got_v;
	cmp_n  -= _i;
	bool active;
	
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	ck_assert_msg
	(
		ret == 0
		, "Error 0x%08X, %i, '%s'"
		, ret
		, ret
		, strerror(ret)
	);
	
	num = nodes[0];
	val = nodes[1];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val > num );
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	active = alu_get_active( alu, val );
	ck_assert( active == true );
	
	alu_reg_init_unsigned( alu, NUM, num );
	alu_reg_init_unsigned( alu, VAL, val );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	active = alu_get_active( alu, val );
	ck_assert( active == true );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
		
	/* alu_reg_cmp() can only return -1,0 or 1 for valid results, anything
	 * else is an error code from errno.h */
	expect = (cmp_n > cmp_v) - (cmp_n < cmp_v);
	
	alu_int_set_raw( alu, num, cmp_n );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	active = alu_get_active( alu, val );
	ck_assert( active == true );
	
	alu_int_get_raw( alu, num, &got_n );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	active = alu_get_active( alu, val );
	ck_assert( active == true );
	
	ck_assert_msg
	(
		cmp_n == got_n
		, "Expected %ju, Got %ju"
		, cmp_n
		, got_n
	);
	
	alu_int_set_raw( alu, val, cmp_v );
	alu_int_get_raw( alu, val, &got_v );
	
	ck_assert_msg
	(
		cmp_v == got_v
		, "Expected %ju, Got %ju"
		, cmp_v
		, got_v
	);
	
	ret = alu_reg_cmp( alu, NUM, VAL );
	ck_assert_msg
	(
		ret >= -1 && ret <= 1
		, "Error %08X, %i, '%s'"
		, ret
		, ret
		, strerror(ret)
	);
	ck_assert_msg
	(
		ret == expect
		, "Expected %ju vs %ju == %i, Got %ju vs %ju = %i"
		, cmp_n
		, cmp_v
		, expect
		, got_n
		, got_v
		, ret
	);
	
	alu_rem_reg_nodes( alu, nodes, REG_COUNT );
	
	--cmp_n;
	++cmp_v;
}
END_TEST

START_TEST( test_alu_op1 )
{
	uint_t num = alu_get_reg_node( alu, 0 );
	alu_reg_t NUM;
	uintmax_t expect
		, op1_f = _i / ops_loop_until
		, op1_n = _i % ops_loop_until
		, got, got_n;
	
	ck_assert( num != 0 );
	
	alu_reg_init_unsigned( alu, NUM, num );
	
	NUM.upto = bitsof(uintmax_t);
	
	op1_str = op1_str_array[op1_f];
	op1_ret = op1_ret_array[op1_f];
	op1_reg = op1_reg_array[op1_f];
	
	if ( op1_str )
	{	
		expect = op1_ret(op1_n);
		
		alu_reg_set_raw( alu, NUM, &op1_n, sizeof(uintmax_t), 0 );
		alu_reg_get_raw( alu, NUM, &got_n, sizeof(uintmax_t) );
		op1_reg( alu, NUM );
		alu_reg_get_raw( alu, NUM, &got, sizeof(uintmax_t) );
		
		ck_assert_msg
		(
			expect == got
			, "Expected %s%ju = %ju, Got %s%ju = %ju"
			, op1_str()
			, op1_n
			, expect
			, op1_str()
			, got_n
			, got
		);
	}
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_op3 )
{
	int ret;
	uintmax_t expect
		, op3_f = _i / ops_loop_until
		, op3_n = 0xDEADC0DE
		, op3_v = _i % ops_loop_until
		, got, got_n, got_v;
	uint_t nodes[REG_COUNT] = {0}, num, val, tmp;
	alu_reg_t NUM, VAL, TMP;
	
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	num = nodes[0];
	val = nodes[1];
	tmp = nodes[2];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val != 0 );
	ck_assert( tmp != 0 );
	
	alu_reg_init_unsigned( alu, NUM, num );
	alu_reg_init_unsigned( alu, VAL, val );
	alu_reg_init_unsigned( alu, TMP, tmp );
	
	NUM.upto = VAL.upto = TMP.upto = bitsof(uintmax_t);
	
	op3_str = op3_str_array[op3_f];
	op3_ret = op3_ret_array[op3_f];
	op3_reg = op3_reg_array[op3_f];
	op3__reg = op3__reg_array[op3_f];
	
	if ( op3_str )
	{
		expect = op3_ret( op3_n, op3_v );
		
		alu_uint_set_raw( alu, num, op3_n );
		alu_uint_get_raw( alu, num, &got_n );
		
		alu_uint_set_raw( alu, val, op3_v );
		alu_uint_get_raw( alu, val, &got_v );
		
		op3_reg( alu, NUM, VAL, TMP, op3__reg );
		alu_reg_get_raw( alu, NUM, &got, sizeof(uintmax_t) );
		
#if 0
		if ( expect != got )
		{
			alu_print_reg( "VAL", alu, VAL, 1, 1, 0 );
		}
#endif
			
		ck_assert_msg
		(
			expect == got
			, "Expected %jX %s %jX = %jX, Got %jX %s %jX = %jX"
			, op3_n
			, op3_str()
			, op3_v
			, expect
			, got_n
			, op3_str()
			, got_v
			, got
		);
	}
	
	alu_rem_reg_nodes( alu, nodes, REG_COUNT );
}
END_TEST

START_TEST( test_alu_op2 )
{
	int ret;
	uint_t nodes[REG_COUNT] = {0}, num, val;
	alu_reg_t NUM, VAL;
	uintmax_t expect
		, op2_f = _i / ops_loop_until
		, op2_n = 0xDEADC0DE
		, op2_v = _i % ops_loop_until
		, got, got_n, got_v;
	
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	num = nodes[0];
	val = nodes[1];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val != 0 );
	
	alu_reg_init_unsigned( alu, NUM, num );
	alu_reg_init_unsigned( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
	
	op2_str = op2_str_array[op2_f];
	op2_ret = op2_ret_array[op2_f];
	op2_reg = op2_reg_array[op2_f];
	
	if ( op2_str )
	{		
		expect = op2_ret( op2_n, op2_v );

		alu_uint_set_raw( alu, num, op2_n );
		alu_uint_get_raw( alu, num, &got_n );
		
		alu_uint_set_raw( alu, val, op2_v );
		alu_uint_get_raw( alu, val, &got_v );
			
		op2_reg( alu, NUM, VAL );
		
		alu_uint_get_raw( alu, num, &got );
		
		ck_assert_msg
		(
			expect == got
			, "Expected %ju %s %ju = %ju, Got %ju %s %ju = %ju"
			, op2_n
			, op2_str()
			, op2_v
			, expect
			, got_n
			, op2_str()
			, got_v
			, got
		);
	}
	
	alu_rem_reg_nodes( alu, nodes, REG_COUNT );
}
END_TEST

START_TEST( test_alu_reg2str )
{
	int ret;
	uintmax_t want = 16;
	alu_block_t ALUSTR = {0};
	alu_dst_t alu_dst = {0};
	alu_base_t base = {0};
	char stdstr[bitsof(uint_t) * 2] = {0}, *alustr;
	uint_t num = alu_get_reg_node( alu, 0 );
	
	ck_assert( ret == 0 );
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
		alustr = ALUSTR.block;
		ck_assert_msg
		(
			memcmp( stdstr, alustr, strlen(stdstr) ) == 0
			, "Expected '%s', Got '%s'"
			, stdstr
			, alustr
		);
	}
	
	alu_block_release( &ALUSTR );
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_get_reg_node )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t num = alu_get_reg_node( alu, 0 );
	bool active;
	
	ck_assert( num != 0 );
	
	active = alu_get_active( alu, num );
	
	ck_assert( num != 0 );
	ck_assert( active == true );
	
	alu_rem_reg_node( alu, &num );
	
	ck_assert( num == 0 );
}
END_TEST

START_TEST( test_alu_get_reg_nodes )
{
	ck_assert( alu_upto(alu) > 0 );
	
	int ret;
	uint_t nodes[REG_COUNT] = {0}, n;
	
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
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
	
	alu_rem_reg_nodes( alu, nodes, REG_COUNT );
	
	for ( n = 0; n < REG_COUNT; ++n )
	{
		ck_assert( nodes[n] == 0 );
	}
}
END_TEST

START_TEST( test_alu_reg_set_raw )
{
	ck_assert( alu_upto( alu ) > 0 );
	
	uint_t num = alu_get_reg_node( alu, 0 );
	uintmax_t val = _i, got;
	alu_reg_t NUM;
	bool active = alu_get_active( alu, num );
	
	ck_assert( num != 0 );
	ck_assert( active == true );
	
	alu_reg_init_unsigned( alu, NUM, num );
	NUM.upto = bitsof(uintmax_t);
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );

	alu_reg_set_raw( alu, NUM, &val, sizeof(uintmax_t), 0 );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	
	alu_reg_get_raw( alu, NUM, &got, sizeof(uintmax_t) );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	
	ck_assert_msg
	(
		val == got
		, "Expected %ju, Got %ju"
		, val
		, got
	);
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_uint_set_raw )
{
	uint_t num = alu_get_reg_node( alu, 0 );
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
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_reg_end_bit )
{
	uint_t num = alu_get_reg_node( alu, 0 );
	size_t bit = 0;
	void *N;
	alu_reg_t NUM;
	alu_bit_t n, v;
	
	ck_assert( num != 0 );
	
	alu_reg_init_unsigned( alu, NUM, num );
	/* Ensure any realloc occurs BEFORE we get our pointer - prevents it
	 * changing during the following loop */
	alu_set_raw( alu, num, 0, 0 );
	
	N = alu_data( alu, num );
	
	while ( bit < bitsof(uintmax_t) )
	{	
		v = alu_bit( N, bit++ );
		
		alu_set_raw( alu, num, v.mask, 0 );
		
		n = alu_reg_end_bit( alu, NUM );
		
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
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_bit )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t num = alu_get_reg_node( alu, 0 );
	uintmax_t *N;
	alu_bit_t n;
	
	ck_assert( num != 0 );
	
	N = (void*)alu_data( alu, num );
	
	n = alu_bit( N, _i );

	ck_assert( n.bit == (size_t)_i );
	ck_assert( n.seg == (size_t)(_i / bitsof(uintmax_t)) );
	ck_assert( n.pos == (size_t)(_i % bitsof(uintmax_t)) );
	ck_assert( n.ptr == (void*)(N + (_i / bitsof(uintmax_t))) );
	ck_assert( n.mask == 1uLL << (_i % bitsof(uintmax_t)) );
	
	alu_rem_reg_node( alu, &num );
}
END_TEST

START_TEST( test_alu_bit_inc )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t i = _i;
	alu_bit_t n, v;
	
	n = alu_bit( &i, i );
	v = alu_bit( &i, ++i );
	alu_bit_inc(&n);
	
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

START_TEST( test_alu_bit_dec )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t i = _i;
	alu_bit_t n, v;
	
	n = alu_bit( &i, i );
	v = alu_bit( &i, --i );
	alu_bit_dec(&n);
	
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

START_TEST( test_alu_vec_release )
{
	alu_vec_release( alu, 0 );
	
	ck_assert( alu->Nsize == 0 );
	ck_assert( alu->taken == 0 );
	ck_assert( alu->given == 0 );
	ck_assert( alu->block.block == NULL );
	ck_assert( alu->block.given == 0 );
	ck_assert( alu->block.taken == 0 );
	ck_assert( alu->block.fault == 0 );
}
END_TEST

START_TEST( test_alu_set_bit )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uintmax_t num[] = {0,0}, val[] = {0,0}, i = _i;
	
	if ( i >= bitsof(uintmax_t) )
		num[1] = 1uLL << _i % bitsof(uintmax_t);
	else
		num[0] = 1uLL << _i;
		
	alu_set_bit( val, _i, 1 );
	
	ck_assert_msg
	(
		memcmp( num, val, sizeof(uintmax_t) * 2 ) == 0
		, "Expected 0x%016jX%016jX, Got 0x%016jX%016jX"
		, num[0], num[1]
		, val[0], val[1]
	);
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

	tcase_add_test( tc_core, test_alu_setup_reg );
	tcase_add_test( tc_core, test_alu_get_reg_node );
	tcase_add_test( tc_core, test_alu_get_reg_nodes );
	
	tcase_add_loop_test( tc_core, test_alu_bit, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alu_set_bit, 0, (bitsof(uintmax_t) * 2) );
	tcase_add_loop_test( tc_core, test_alu_bit_inc, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alu_bit_dec, 1, ops_loop_until + 1 );

	tcase_add_loop_test( tc_core, test_alu_reg_set_raw, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alu_uint_set_raw, 0, ops_loop_until );

	tcase_add_test( tc_core, test_alu_reg_end_bit );
	tcase_add_loop_test( tc_core, test_alu_reg_cmp, 0, ops_loop_until );

	for ( f = 0; op1_str_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op1, 0, f * ops_loop_until );
	
	for ( f = 0; op2_str_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op2, 0, f * ops_loop_until );

	for ( f = 0; op3_str_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op3, 0, f * ops_loop_until );
	
	tcase_add_test( tc_core, test_alu_reg2str );
	tcase_add_test( tc_core, test_alu_vec_release );
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
