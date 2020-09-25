#include <check.h>
#include <alu.h>
#include <stdlib.h>

int wrChar( char32_t c, void *dst )
{
	alu_block_t *ALUSTR = dst;
	uint_t i = ALUSTR->taken;
	int ret = alu_block_expand( dst, ++(ALUSTR->taken) );
	char *alustr;
	
	alu_printf( "wrChar('%c',%p)", (char)c, dst );
	
	if ( ret != 0 )
	{
		--(ALUSTR->taken);
		return ret;
	}
	
	alustr = ALUSTR->block;
	alustr[i] = (char)c;
	return 0;
}
void flip( void *dst )
{
	alu_block_t *ALUSTR = dst;
	uint_t i, j;
	char c, *alustr = ALUSTR->block;
	
	alu_printf("alustr = '%s', taken = %zu", alustr, ALUSTR->taken );
	
	for ( i = 0, j = ALUSTR->taken - 1; i < j; ++i, --j )
	{
		c = alustr[i];
		alustr[i] = alustr[j];
		alustr[j] = c;
	}
}

int test_alu_uint_set_raw( size_t line, alu_t* alu, uint_t num, uintmax_t val )
{
	int ret;
	alu_reg_t NUM;
	void *data;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	ck_assert( NUM.node == num );
	ck_assert( NUM.upto == alu_bits_perN(alu) );
	ck_assert( NUM.from == 0 );
	ck_assert( NUM.mant == 0 );
	ck_assert( NUM.info == 0 );
	
	NUM.upto = bitsof(uintmax_t);
	
	ret = alu_reg_set_raw( alu, NUM, &val, sizeof(uintmax_t), 0 );
	
	if ( ret == 0 )
	{
		data = alu_data( alu, num );
		
		ck_assert_msg
		(
			memcmp( data, &val, sizeof(uintmax_t) ) == 0
			, "Line %zu, Expected %ju, Got %ju"
			, line
			, val
			, *((uintmax_t*)data)
		);
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

int test__alu_uint_op1
(
	size_t line
	, alu_t* alu
	, uint_t num
	, char *op
	, uintmax_t expect
	, func_alu_reg_op1_t op1
)
{
	int ret;
	alu_reg_t NUM;
	void *data;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	ck_assert( NUM.node == num );
	ck_assert( NUM.upto == alu_bits_perN(alu) );
	ck_assert( NUM.from == 0 );
	ck_assert( NUM.mant == 0 );
	ck_assert( NUM.info == 0 );
	
	NUM.upto = bitsof(uintmax_t);
	
	ret = op1( alu, NUM );
	
	if ( ret == 0 )
	{
		data = alu_data( alu, num );
		
		ck_assert_msg
		(
			memcmp( data, &expect, sizeof(uintmax_t) ) == 0
			, "Line %zu, %s = %ju, Got %ju"
			, line
			, op
			, expect
			, *((uintmax_t*)data)
		);
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

#define test_alu_uint_op1( alu, num, op, expect, op1 ) \
	test__alu_uint_op1( __LINE__, alu, num, op, (expect), op1 )

int test__alu_uint_op2
(
	size_t line
	, alu_t* alu
	, uint_t num
	, uint_t val
	, char *op
	, uintmax_t expect
	, func_alu_reg_op2_t op2
)
{
	int ret;
	alu_reg_t NUM, VAL;
	void *data;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	ck_assert( NUM.node == num );
	ck_assert( NUM.upto == alu_bits_perN(alu) );
	ck_assert( NUM.from == 0 );
	ck_assert( NUM.mant == 0 );
	ck_assert( NUM.info == 0 );
	
	alu_reg_init( alu, VAL, val, 0 );
	
	ck_assert( VAL.node == val );
	ck_assert( VAL.upto == alu_bits_perN(alu) );
	ck_assert( VAL.from == 0 );
	ck_assert( VAL.mant == 0 );
	ck_assert( VAL.info == 0 );
	
	NUM.upto = bitsof(uintmax_t);
	VAL.upto = bitsof(uintmax_t);
	
	ret = op2( alu, NUM, VAL );
	
	if ( ret == 0 || ret == ENODATA )
	{
		data = alu_data( alu, num );
		
		ck_assert_msg
		(
			memcmp( data, &expect, sizeof(uintmax_t) ) == 0
			, "Line %zu, %s = %ju, Got %ju"
			, line
			, op
			, expect
			, *((uintmax_t*)data)
		);
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

#define test_alu_uint_op2( alu, num, val, op, expect, op2 ) \
	test__alu_uint_op2( __LINE__, alu, num, val, op, (expect), op2 )

int test__alu_uint_shift
(
	size_t line
	, alu_t* alu
	, uint_t num
	, uint_t val
	, uint_t tmp
	, char *op
	, uintmax_t expect
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	int ret;
	alu_reg_t NUM, VAL, TMP;
	void *data;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	ck_assert( NUM.node == num );
	ck_assert( NUM.upto == alu_bits_perN(alu) );
	ck_assert( NUM.from == 0 );
	ck_assert( NUM.mant == 0 );
	ck_assert( NUM.info == 0 );
	
	alu_reg_init( alu, VAL, val, 0 );
	
	ck_assert( VAL.node == val );
	ck_assert( VAL.upto == alu_bits_perN(alu) );
	ck_assert( VAL.from == 0 );
	ck_assert( VAL.mant == 0 );
	ck_assert( VAL.info == 0 );
	
	alu_reg_init( alu, TMP, tmp, 0 );
	
	ck_assert( TMP.node == tmp );
	ck_assert( TMP.upto == alu_bits_perN(alu) );
	ck_assert( TMP.from == 0 );
	ck_assert( TMP.mant == 0 );
	ck_assert( TMP.info == 0 );
	
	NUM.upto = bitsof(uintmax_t);
	VAL.upto = bitsof(uintmax_t);
	TMP.upto = bitsof(uintmax_t);
	
	ret = shift( alu, NUM, VAL, TMP, _shift );
	
	if ( ret == 0 )
	{
		data = alu_data( alu, num );
		
		ck_assert_msg
		(
			memcmp( data, &expect, sizeof(uintmax_t) ) == 0
			, "Line %zu, %s = %ju, Got %ju"
			, line
			, op
			, expect
			, *((uintmax_t*)data)
		);
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

#define test_alu_uint_shift( alu, num, val, tmp, op, expect, _shift, shift ) \
	test__alu_uint_shift( __LINE__, alu, num, val, tmp, op, (expect), _shift, shift )

#define REG_COUNT 3

START_TEST( test_alu_create )
{
	int ret, cmp, expect_cmp;
	alu_t _alu = {0}, *alu = &_alu;
	alu_uint_t num, val, tmp;
	alu_reg_t TMP;
	alu_bit_t n;
	alu_block_t ALUSTR = {0};
	alu_dst_t alu_dst = {0};
	//alu_src_t alu_src = {0};
	alu_base_t base = {0};
	char stdstr[bitsof(uint_t) * 2] = {0}, *alustr;
	uchar_t *data;
	uintmax_t i, want = bitsof(uintmax_t);
	uint_t nodes[REG_COUNT] = {0};
	
	puts( "Initialising ALU\n" );
	ret = alu_setup_reg( alu, want, 0, 0 );
	
	ck_assert( ret == 0 );
	ck_assert( alu_used( alu ) != 0 );
	ck_assert_msg
	(
		alu_upto( alu ) >= want
		, "Expected at least %ju, Got %u"
		, want
		, alu_upto(alu)
	);
	ck_assert( alu_valid( alu ) != NULL );
	ck_assert( alu_size_perN( alu ) >= sizeof(uintmax_t) );
	
	puts( "Getting registers\n" );
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	for ( i = 0; i < REG_COUNT; ++i )
	{
		ck_assert( nodes[i] != 0 );
	}
	
	num = nodes[0];
	
	test_alu_uint_set_raw( __LINE__, alu, num, want );
	
	val = nodes[1];
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		test_alu_uint_set_raw( __LINE__, alu, val, i );
	}
	
	tmp = nodes[2];
	alu_reg_init( alu, TMP, tmp, 0 );
	
	ck_assert( TMP.node == tmp );
	ck_assert( TMP.upto == alu_bits_perN(alu) );
	ck_assert( TMP.from == 0 );
	ck_assert( TMP.mant == 0 );
	ck_assert( TMP.info == 0 );
	
	TMP.upto = bitsof(uint_t);
	data = alu_data( alu, tmp );
	n = alu_bit_set_bit( (void*)data, 0 );
	
	for ( i = 0; i < bitsof(uintmax_t); ++i, alu_bit_inc(&n) )
	{
		ck_assert( n.bit == i );
		ck_assert( n.seg == (i / bitsof(uintmax_t)) );
		ck_assert( n.pos == (i % bitsof(uintmax_t)) );
		ck_assert( n.ptr == (void*)(data + (i / bitsof(uintmax_t))) );
		ck_assert( n.mask == 1uLL << (i % bitsof(uintmax_t)) );
	}
	
	while ( i > 0 )
	{
		--i;
		alu_bit_dec(&n);
		
		ck_assert( n.bit == i );
		ck_assert( n.seg == (i / bitsof(uintmax_t)) );
		ck_assert( n.pos == (i % bitsof(uintmax_t)) );
		ck_assert( n.ptr == (void*)(data + (i / bitsof(uintmax_t))) );
		ck_assert( n.mask == 1uLL << (i % bitsof(uintmax_t)) );
	}
	
	puts("Attempting comparison\n");
	
	/* alu_reg_cmp() can only return -1,0 or 1 for valid results, anything else
	 * is an error code from errno.h */
	expect_cmp = (want < i) ? -1 : ((want > i) ? 1 : 0);
	test_alu_uint_set_raw( __LINE__, alu, val, i );	
	test_alu_uint_set_raw( __LINE__, alu, num, want );
	cmp = alu_uint_cmp( alu, num, val );
	ck_assert_msg
	(
		cmp == expect_cmp
		, "%ju vs %ju == %i, Got %i"
		, want
		, i
		, expect_cmp
		, cmp
	);
	
	puts("Attempting math\n");
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "~%ju", i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, i );
		test_alu_uint_op1
		(
			alu
			, num
			, stdstr
			, ~i
			, alu_reg_not
		);
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju >> %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_shift
		(
			alu
			, num
			, val
			, tmp
			, stdstr
			, want >> i
			, alu_reg__shr
			, alu_reg__shift
		);
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju << %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
			
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_shift
		(
			alu
			, num
			, val
			, tmp
			, stdstr
			, want << i
			, alu_reg__shl
			, alu_reg__shift
		);
	}
		
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju ^ %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want ^ i, alu_reg_xor );
	}
		
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju | %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want | i, alu_reg__or );
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju & %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want & i, alu_reg_and );
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju + %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
			
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want + i, alu_reg_add );
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju - %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
			
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want - i, alu_reg_sub );
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju * %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
			
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2( alu, num, val, stdstr, want * i, alu_reg_mul );
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju / %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
			
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2
		(
			alu
			, num
			, val
			, stdstr
			, i ? want / i : 0
			, alu_reg_div
		);
	}
	
	for ( i = 0; i < bitsof(uintmax_t); ++i )
	{
		sprintf( stdstr, "%ju %% %ju", want, i );
		
		test_alu_uint_set_raw( __LINE__, alu, val, i );
		
		test_alu_uint_set_raw( __LINE__, alu, num, want );
		test_alu_uint_op2
		(
			alu
			, num
			, val
			, stdstr
			, i ? want % i : want
			, alu_reg_rem
		);
	}
		
	puts("Attempting to print to string\n");
	
	alu_dst.dst = &ALUSTR;
	alu_dst.next = wrChar;
	alu_dst.flip = flip;
	
	base.base = 10;
	base.digsep = '\'';
	sprintf( stdstr, "%ju", want );
	test_alu_uint_set_raw( __LINE__, alu, num, want );
	ret = alu_uint2str( alu, alu_dst, num, base );
	
	if ( ret != 0 )
	{
		alu_error(ret);
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
		
	alu_vec_release( alu, alu_size_perN(alu) );
}
END_TEST

/* Modified copy & paste of code given in check's guide */
Suite * alu_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("ALU");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_alu_create);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	
	puts( "Running unit tests under 'check' test suite\n" );

	s = alu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
