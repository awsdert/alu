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
uintmax_t op3_ret_rol( uintmax_t a, uintmax_t b )
{
	b %= bitsof(uintmax_t);
	return unic_rol( a, b );
}
const char* op3_str_ror() { return ">>>"; }
uintmax_t op3_ret_ror( uintmax_t a, uintmax_t b )
{
	b %= bitsof(uintmax_t);
	return unic_ror( a, b );
}

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

func_alur_op1_t op1_reg = NULL, op1_reg_array[] =
{
	alur_not
	, alur_inc
	, alur_dec
	, NULL
};

func_op3_str op3_str = NULL
, op3_str_shift_array[] =
{
	op3_str_shl
	, op3_str_shr
	, NULL
}
, op3_str_rotate_array[] =
{
	op3_str_rol
	, op3_str_ror
	, NULL
};

func_op3_ret op3_ret = NULL
, op3_ret_shift_array[] =
{
	op3_ret_shl
	, op3_ret_shr
	, NULL
}
, op3_ret_rotate_array[] =
{
	op3_ret_rol
	, op3_ret_ror
	, NULL
};

func_alur__shift_t op3_reg_shift = NULL, op3_reg_shift_array[] =
{
	alur__shl
	, alur__shr
	, NULL
};

func_alur__rotate_t op3_reg_rotate = NULL, op3_reg_rotate_array[] =
{
	alur__rol
	, alur__ror
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

func_alur_op2_t op2_reg = NULL, op2_reg_array[] =
{
	alur_and
	, alur__or
	, alur_xor
	, alur_add
	, alur_mul
	, alur_sub
	, alur_div
	, alur_rem
	, NULL
};

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

START_TEST( test_alur_cmp )
{
	int ret, expect;
	uint_t nodes[REG_COUNT], num, val;
	alur_t NUM, VAL;
	intmax_t cmp_n = ops_loop_until, cmp_v = _i, got_n, got_v;
	cmp_n  -= _i;
	bool active;
	
	ret = alur_get_nodes( alu, nodes, REG_COUNT, 0 );
	
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
	
	alur_init_unsigned( alu, NUM, num );
	alur_init_unsigned( alu, VAL, val );
	
	active = alu_get_active( alu, num );
	ck_assert( active == true );
	active = alu_get_active( alu, val );
	ck_assert( active == true );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
		
	/* alur_cmp() can only return -1,0 or 1 for valid results, anything
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
	
	ret = alur_cmp( alu, NUM, VAL );
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
	
	alur_rem_nodes( alu, nodes, REG_COUNT );
	
	--cmp_n;
	++cmp_v;
}
END_TEST

START_TEST( test_alu_op1 )
{
	uint_t num = alur_get_node( alu, 0 );
	alur_t NUM;
	uintmax_t
		op1_f = _i / ops_loop_until
		, op1_n = _i % ops_loop_until
		, got, got_n;
	
	ck_assert( num != 0 );
	
	alur_init_unsigned( alu, NUM, num );
	
	NUM.upto = bitsof(uintmax_t);
	
	op1_str = op1_str_array[op1_f];
	op1_ret = op1_ret_array[op1_f];
	op1_reg = op1_reg_array[op1_f];
	
	if ( op1_str )
	{	
		uintmax_t expect = op1_ret(op1_n);
		
		alur_set_raw( alu, NUM, &op1_n, sizeof(uintmax_t), 0 );
		alur_get_raw( alu, NUM, &got_n, sizeof(uintmax_t), 0 );
		op1_reg( alu, NUM );
		alur_get_raw( alu, NUM, &got, sizeof(uintmax_t), 0 );
		
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
	
	alur_rem_node( alu, &num );
}
END_TEST

START_TEST( test_alu_op3_shift )
{
	int ret;
	uintmax_t
		op3_f = _i / ops_loop_until
		, op3_v = _i % ops_loop_until
		, got, got_n, got_v;
	uint_t nodes[REG_COUNT] = {0}, num, val, tmp;
	alur_t NUM, VAL;
	
	ret = alur_get_nodes( alu, nodes, REG_COUNT, 0 );
	
	num = nodes[0];
	val = nodes[1];
	tmp = nodes[2];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val != 0 );
	ck_assert( tmp != 0 );
	
	alur_init_unsigned( alu, NUM, num );
	alur_init_unsigned( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
	
	op3_str = op3_str_shift_array[op3_f];
	op3_ret = op3_ret_shift_array[op3_f];
	op3_reg_shift = op3_reg_shift_array[op3_f];
	
	if ( op3_str )
	{
		uintmax_t op3_n = 0xDEADC0DE, expect = op3_ret( op3_n, op3_v );
		
		alu_uint_set_raw( alu, num, op3_n );
		alu_uint_get_raw( alu, num, &got_n );
		
		alu_uint_set_raw( alu, val, op3_v );
		alu_uint_get_raw( alu, val, &got_v );
		
		alur__shift( alu, NUM, VAL, op3_reg_shift );
		alur_get_raw( alu, NUM, &got, sizeof(uintmax_t), 0 );
		
#if 0
		if ( expect != got )
		{
			alur_print( "VAL", alu, VAL, 1, 1, 0 );
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
	
	alur_rem_nodes( alu, nodes, REG_COUNT );
}
END_TEST

START_TEST( test_alup__rol_int2int )
{
	ushort_t src_num = 0xC0DE, src_val = _i
		, dst_num = src_num, dst_val = src_val % bitsof(ushort_t)
		, got_num = src_num, got_val = src_val;
	alup_t _GOT_NUM;
	
	alup_init_unsigned( _GOT_NUM, &got_num, sizeof(ushort_t) );
	
	dst_num = unic_rol( dst_num, dst_val );
	alup__rol_int2int( _GOT_NUM, got_val );
	
	if ( got_num != dst_num )
	{
		alup_t _DST_NUM, _SRC_NUM;
		
		alup_init_unsigned( _SRC_NUM, &src_num, sizeof(ushort_t) );
		alup_init_unsigned( _DST_NUM, &dst_num, sizeof(ushort_t) );
		
		alu_printf( "src_val = %u", src_val );
		alup_print( _SRC_NUM, 0, 1 );
		alup_print( _DST_NUM, 0, 1 );
		alup_print( _GOT_NUM, 0, 1 );
	}
	
	ck_assert( got_num == dst_num );
}
END_TEST

START_TEST( test_alup__ror_int2int )
{
	ushort_t src_num = 0xC0DE
		, val = _i  % bitsof(ushort_t)
		, dst_num = src_num
		, got_num = src_num;
	alup_t _GOT_NUM;
	
	alup_init_unsigned( _GOT_NUM, &got_num, sizeof(ushort_t) );
	
	dst_num = unic_ror( dst_num, val );
	alup__ror_int2int( _GOT_NUM, _i );
	
	if ( got_num != dst_num )
	{
		alup_t _DST_NUM, _SRC_NUM;
		
		alup_init_unsigned( _SRC_NUM, &src_num, sizeof(ushort_t) );
		alup_init_unsigned( _DST_NUM, &dst_num, sizeof(ushort_t) );
		
		alu_printf( "_i = %u", _i );
		alup_print( _SRC_NUM, 0, 1 );
		alup_print( _DST_NUM, 0, 1 );
		alup_print( _GOT_NUM, 0, 1 );
	}
	
	ck_assert( got_num == dst_num );
}
END_TEST

START_TEST( test_alu_op3_rotate )
{
	int ret;
	uintmax_t
		op3_f = _i / ops_loop_until
		, op3_v = _i % ops_loop_until
		, got, got_n, got_v;
	uint_t nodes[REG_COUNT] = {0}, num, val, tmp;
	alur_t NUM, VAL;
	
	ret = alur_get_nodes( alu, nodes, REG_COUNT, 0 );
	
	num = nodes[0];
	val = nodes[1];
	tmp = nodes[2];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val != 0 );
	ck_assert( tmp != 0 );
	
	alur_init_unsigned( alu, NUM, num );
	alur_init_unsigned( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
	
	op3_str = op3_str_rotate_array[op3_f];
	op3_ret = op3_ret_rotate_array[op3_f];
	op3_reg_rotate = op3_reg_rotate_array[op3_f];
	
	if ( op3_str )
	{
		uintmax_t op3_n = 0xDEADC0DE, expect = op3_ret( op3_n, op3_v );
		
		alu_uint_set_raw( alu, num, op3_n );
		alu_uint_get_raw( alu, num, &got_n );
		
		alu_uint_set_raw( alu, val, op3_v );
		alu_uint_get_raw( alu, val, &got_v );
		
		alur__rotate( alu, NUM, VAL, tmp, op3_reg_rotate );
		alur_get_raw( alu, NUM, &got, sizeof(uintmax_t), 0 );
		
#if 0
		if ( expect != got )
		{
			alur_print( "VAL", alu, VAL, 1, 1, 0 );
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
	
	alur_rem_nodes( alu, nodes, REG_COUNT );
}
END_TEST

START_TEST( test_alu_op2 )
{
	int ret;
	uint_t nodes[REG_COUNT] = {0}, num, val;
	alur_t NUM, VAL;
	uintmax_t
		op2_f = _i / ops_loop_until
		, op2_v = _i % ops_loop_until
		, got, got_n, got_v;
	
	ret = alur_get_nodes( alu, nodes, REG_COUNT, 0 );
	
	num = nodes[0];
	val = nodes[1];
	
	ck_assert( ret == 0 );
	ck_assert( num != 0 );
	ck_assert( val != 0 );
	
	alur_init_unsigned( alu, NUM, num );
	alur_init_unsigned( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(uintmax_t);
	
	op2_str = op2_str_array[op2_f];
	op2_ret = op2_ret_array[op2_f];
	op2_reg = op2_reg_array[op2_f];
	
	if ( op2_str )
	{
		uintmax_t op2_n = 0xDEADC0DE, expect = op2_ret( op2_n, op2_v );

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
	
	alur_rem_nodes( alu, nodes, REG_COUNT );
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

START_TEST( test_alur_end_bit )
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
		
		n = alur_end_bit( alu, NUM );
		
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

START_TEST( test_alur_add_floating )
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
	
	ret = alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
	ck_assert( ret == 0 );
	
	ret = alur_set_raw( alu, VAL, &src_val, sizeof(double), VAL.info );
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
	
	src_num += src_val;
	(void)alur_add( alu, NUM, VAL );
	
	N = alur_data( alu, NUM );
	
	if ( memcmp( N, &src_num, sizeof(double) ) != 0 )
	{
		alur_t EXP, MAN;
		
		alur_init_exponent( NUM, EXP );
		alur_init_mantissa( NUM, MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, "
			"(EXP.upto = %zu) - (EXP.from = %zu) = %zu, "
			"(MAN.upto = %zu) - (MAN.from = %zu) = %zu"
			, _num
			, _val
			, EXP.upto, EXP.from, EXP.upto - EXP.from
			, MAN.upto, MAN.from, MAN.upto - MAN.from
		);
		
		alur_print( alu, NUM, 0, 1 );
		(void)alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
		alur_print( alu, NUM, 0, 1 );
		
		ck_assert( 1 == 0 );
	}
	
	alur_rem_nodes( alu, nodes, 2 );
}
END_TEST

START_TEST( test_alur_add_floating_randomized )
{
	ck_assert( alu_upto(alu) > 0 );
	
	uint_t nodes[2], num, val;
	uint_t seed = time(NULL);
	ullong_t _num = _i, _val = 1;
	int ret = alur_get_nodes( alu, nodes, 2, 0 );
	double src_num, src_val, got_num, got_val;
	alur_t NUM, VAL;
	void *N, *V;
	
	ck_assert( ret == 0 );
	
	num = nodes[0];
	val = nodes[1];
	
	for ( int i = 0; i < _i; ++i ) _num = rand_r(&seed);
	src_num = _num;
	
	for ( int i = 0; i < _i; ++i ) _val = rand_r(&seed);
	src_val = _val;
	
	alur_init_floating( alu, NUM, num );
	alur_init_floating( alu, VAL, val );
	
	NUM.upto = VAL.upto = bitsof(double);
	
	/* Set values */
	
	ret = alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
	ck_assert( ret == 0 );
	
	ret = alur_set_raw( alu, VAL, &src_val, sizeof(double), VAL.info );
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
	
	src_num += src_val;
	(void)alur_add( alu, NUM, VAL );
	
	N = alur_data( alu, NUM );
	
	if ( memcmp( N, &src_num, sizeof(double) ) != 0 )
	{
		alur_t EXP, MAN;
		alup_t _NUM, _VAL;
		
		alur_init_exponent( NUM, EXP );
		alur_init_mantissa( NUM, MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, "
			"(EXP.upto = %zu) - (EXP.from = %zu) = %zu, "
			"(MAN.upto = %zu) - (MAN.from = %zu) = %zu"
			, _num
			, _val
			, EXP.upto, EXP.from, EXP.upto - EXP.from
			, MAN.upto, MAN.from, MAN.upto - MAN.from
		);
		
		alur_print( alu, NUM, 0, 1 );
		(void)alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
		alur_print( alu, NUM, 0, 1 );
		
		src_num = _num;
		(void)alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
		
		alur_print( alu, VAL, 0, 1 );
		alur_print( alu, NUM, 0, 1 );
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		alup_match_exponents( _NUM.data, _VAL.data, sizeof(double) );
		
		alur_print( alu, VAL, 0, 1 );
		alur_print( alu, NUM, 0, 1 );
		
		ck_assert( 1 == 0 );
	}
	
	alur_rem_nodes( alu, nodes, 2 );
}
END_TEST

START_TEST( test_alup_sub_floating )
{
	ck_assert( alu_upto(alu) > 0 );
	
	ullong_t _num = _i, _val = 1;
	double
		src_num = _num
		, src_val = _val
		, got_num = _num
		, got_val = _val
		, tmp_num
		, tmp_val;
	alup_t _NUM, _VAL;
	
	alup_init_floating( _NUM, &got_num, sizeof(double) );
	alup_init_floating( _VAL, &got_val, sizeof(double) );
	
	src_num -= src_val;
	(void)alup__sub( _NUM, _VAL, &tmp_num, &tmp_val );
	
	if ( memcmp( &got_num, &src_num, sizeof(double) ) != 0 )
	{
		alup_t _EXP, _MAN;
		
		alup_init_exponent( _NUM, _EXP );
		alup_init_mantissa( _NUM, _MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, "
			"(_EXP.upto = %zu) - (_EXP.from = %zu) = %zu, "
			"(_MAN.upto = %zu) - (_MAN.from = %zu) = %zu"
			, _num
			, _val
			, _EXP.upto, _EXP.from, _EXP.upto - _EXP.from
			, _MAN.upto, _MAN.from, _MAN.upto - _MAN.from
		);
		
		alup_print( _NUM, 0, 1 );
		got_num = src_num;
		alup_print( _NUM, 0, 1 );
		
		got_num = _num;
		
		alup_print( _VAL, 0, 1 );
		alup_print( _NUM, 0, 1 );
		
		alup_match_exponents( _NUM.data, _VAL.data, sizeof(double) );
		
		alup_print( _VAL, 0, 1 );
		alup_print( _NUM, 0, 1 );
		
		ck_assert( 1 == 0 );
	}
}
END_TEST

START_TEST( test_alur_sub_floating )
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
	
	ret = alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
	ck_assert( ret == 0 );
	
	ret = alur_set_raw( alu, VAL, &src_val, sizeof(double), VAL.info );
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
	
	src_num -= src_val;
	(void)alur_sub( alu, NUM, VAL );
	
	N = alur_data( alu, NUM );
	
	if ( memcmp( N, &src_num, sizeof(double) ) != 0 )
	{
		alur_t EXP, MAN;
		alup_t _NUM, _VAL;
		
		alur_init_exponent( NUM, EXP );
		alur_init_mantissa( NUM, MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, "
			"(EXP.upto = %zu) - (EXP.from = %zu) = %zu, "
			"(MAN.upto = %zu) - (MAN.from = %zu) = %zu"
			, _num
			, _val
			, EXP.upto, EXP.from, EXP.upto - EXP.from
			, MAN.upto, MAN.from, MAN.upto - MAN.from
		);
		
		alur_print( alu, NUM, 0, 1 );
		(void)alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
		alur_print( alu, NUM, 0, 1 );
		
		src_num = _num;
		(void)alur_set_raw( alu, NUM, &src_num, sizeof(double), NUM.info );
		
		alur_print( alu, VAL, 0, 1 );
		alur_print( alu, NUM, 0, 1 );
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		alup_match_exponents( _NUM.data, _VAL.data, sizeof(double) );
		
		alur_print( alu, VAL, 0, 1 );
		alur_print( alu, NUM, 0, 1 );
		
		ck_assert( 1 == 0 );
	}
	
	alur_rem_nodes( alu, nodes, 2 );
}
END_TEST

START_TEST( test_alup_mul_floating )
{
	ck_assert( alu_upto(alu) > 0 );
	
	ullong_t _num = _i, _val = 1;
	double
		src_num = _num
		, src_val = _val
		, got_num = _num
		, got_val = _val
		, tmp_num
		, tmp_val;
	alup_t _GOT_NUM, _GOT_VAL;
	
	alup_init_floating( _GOT_NUM, &got_num, sizeof(double) );
	alup_init_floating( _GOT_VAL, &got_val, sizeof(double) );
	
	src_num *= src_val;
	(void)alup__mul( _GOT_NUM, _GOT_VAL, &tmp_num, &tmp_val );
	
	if ( memcmp( &got_num, &src_num, sizeof(double) ) != 0 )
	{
		alup_t _EXP, _MAN, _SRC_NUM, _NUM;
		
		alup_init_unsigned( _NUM, &_num, sizeof(ullong_t) );
		alup_init_floating( _SRC_NUM, &src_num, sizeof(double) );
		alup_init_exponent( _GOT_NUM, _EXP );
		alup_init_mantissa( _GOT_NUM, _MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, "
			"(_EXP.upto = %zu) - (_EXP.from = %zu) = %zu, "
			"(_MAN.upto = %zu) - (_MAN.from = %zu) = %zu"
			, _num
			, _val
			, _EXP.upto, _EXP.from, _EXP.upto - _EXP.from
			, _MAN.upto, _MAN.from, _MAN.upto - _MAN.from
		);
		
		alup_print( _GOT_NUM, 0, 1 );
		alup_print( _SRC_NUM, 0, 1 );
		alup_print( _GOT_VAL, 0, 1 );
		alup_print( _NUM, 0, 1 );
	}
	
	ck_assert( memcmp( &got_num, &src_num, sizeof(double) ) == 0 );
}
END_TEST

START_TEST( test_alup_div_floating )
{
	ck_assert( alu_upto(alu) > 0 );
	
	ullong_t _num = _i, _val = 3;
	double tmp_num, tmp_val
		, src_num = _num
		, src_val = _val
		, got_num = _num
		, got_val = _val
		, expect = _num
		, result = _num;
	alup_t _RESULT, _GOT_VAL;
	
	alup_init_floating( _RESULT, &result, sizeof(double) );
	alup_init_floating( _GOT_VAL, &got_val, sizeof(double) );
	
	expect /= src_val;
	(void)alup__div( _RESULT, _GOT_VAL, &tmp_num, &tmp_val );
	
	if ( memcmp( &result, &expect, sizeof(double) ) != 0 )
	{
		alup_t _EXP, _MAN, _EXPECT, _GOT_NUM, _SRC_NUM, _SRC_VAL;
		
		alup_init_floating( _EXPECT, &expect, sizeof(double) );
		alup_init_floating( _SRC_NUM, &src_num, sizeof(double) );
		alup_init_floating( _SRC_VAL, &src_val, sizeof(double) );
		alup_init_floating( _GOT_NUM, &got_num, sizeof(double) );
		alup_init_exponent( _GOT_NUM, _EXP );
		alup_init_mantissa( _GOT_NUM, _MAN );
		
		alu_printf
		(
			"_num = %llu, _val = %llu, exp length = %zu, man length = %zu"
			, _num
			, _val
			, _EXP.upto - _EXP.from
			, _MAN.upto - _MAN.from
		);
		
		alup_print( _EXPECT, 0, 1 );
		alup_print( _RESULT, 0, 1 );
#if 0
		alup_print( _GOT_NUM, 0, 1 );
		alup_print( _SRC_NUM, 0, 1 );
		alup_print( _GOT_VAL, 0, 1 );
		alup_print( _SRC_VAL, 0, 1 );
#endif
	}
	
	ck_assert( memcmp( &result, &expect, sizeof(double) ) == 0 );
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
	
	tcase_add_loop_test( tc_core, test_alub, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alub_set, 0, (bitsof(uintmax_t) * 2) );
	tcase_add_loop_test( tc_core, test_alub_inc, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alub_dec, 1, ops_loop_until + 1 );

	tcase_add_loop_test( tc_core, test_alur_set_raw, 0, ops_loop_until );
	tcase_add_loop_test( tc_core, test_alu_uint_set_raw, 0, ops_loop_until );

	tcase_add_test( tc_core, test_alur_end_bit );
	tcase_add_loop_test( tc_core, test_alur_cmp, 0, ops_loop_until );

	for ( f = 0; op1_str_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op1, 0, f * ops_loop_until );
	
	for ( f = 0; op2_str_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op2, 0, f * ops_loop_until );

	for ( f = 0; op3_str_shift_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op3_shift, 0, f * ops_loop_until );
	
	for ( f = 0; op3_str_rotate_array[f]; ++f );
	tcase_add_loop_test( tc_core, test_alu_op3_rotate, 0, f * ops_loop_until );
	tcase_add_loop_test( tc_core, test_alup__rol_int2int, 1, bitsof(ushort_t)-1 );
	tcase_add_loop_test( tc_core, test_alup__ror_int2int, 1, bitsof(ushort_t)-1 );
	
	tcase_add_loop_test( tc_core, test_alur_set_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alur_add_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alur_add_floating_randomized, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_sub_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alur_sub_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_mul_floating, 0, DBL_MANT_DIG );
	tcase_add_loop_test( tc_core, test_alup_div_floating, 0, DBL_MANT_DIG );
	
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
