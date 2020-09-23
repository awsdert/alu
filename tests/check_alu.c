#include <check.h>
#include <alu.h>
#include <stdlib.h>

#define REG_COUNT 3

int wrChar( char32_t c, void *dst )
{
	alu_block_t *ALUSTR = dst;
	uint_t i = ALUSTR->taken;
	int ret = alu_block_expand( dst, ++(ALUSTR->taken) );
	char *alustr;
	
	printf( "wrChar('%c',%p)", (char)c, dst );
	
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
	
	for ( i = 0, j = ALUSTR->taken - 1; i < j; ++i, --j )
	{
		c = alustr[i];
		alustr[i] = alustr[j];
		alustr[j] = c;
	}
}

START_TEST( test_alu_create )
{
	int ret, cmp;
	alu_t _alu = {0}, *alu = &_alu;
	alu_uint_t num, val, tmp;
	alu_reg_t NUM, VAL, TMP;
	alu_block_t ALUSTR = {0};
	alu_dst_t alu_dst = {0};
	//alu_src_t alu_src = {0};
	alu_base_t base = {0};
	char stdstr[bitsof(uint_t)] = {0}, *alustr;
	void *data;
	uint_t i, want = 32, temp, nodes[REG_COUNT] = {0};
	
	puts( "Initialising ALU\n" );
	ret = alu_setup_reg( alu, want, 0, 0 );
	
	ck_assert( ret == 0 );
	ck_assert( alu_used( alu ) != 0 );
	ck_assert_msg
	(
		alu_upto( alu ) >= want
		, "Expected at least %u, Got %u"
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
	alu_reg_init( alu, NUM, num, 0 );
	
	ck_assert( NUM.node == num );
	ck_assert( NUM.upto == alu_bits_perN(alu) );
	ck_assert( NUM.from == 0 );
	ck_assert( NUM.mant == 0 );
	ck_assert( NUM.info == 0 );
	
	NUM.upto = bitsof(uint_t);
	alu_uint_set_raw( alu, num, want );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &want, sizeof(uint_t) ) == 0 );
	
	val = nodes[1];
	alu_reg_init( alu, VAL, val, 0 );
	
	ck_assert( VAL.node == val );
	ck_assert( VAL.upto == alu_bits_perN(alu) );
	ck_assert( VAL.from == 0 );
	ck_assert( VAL.mant == 0 );
	ck_assert( VAL.info == 0 );
	
	VAL.upto = bitsof(uint_t);
	alu_uint_set_raw( alu, val, i );
	data = alu_data( alu, val );
	ck_assert( memcmp( data, &i, sizeof(uint_t) ) == 0 );
	
	tmp = nodes[2];
	alu_reg_init( alu, TMP, tmp, 0 );
	
	ck_assert( TMP.node == tmp );
	ck_assert( TMP.upto == alu_bits_perN(alu) );
	ck_assert( TMP.from == 0 );
	ck_assert( TMP.mant == 0 );
	ck_assert( TMP.info == 0 );
	
	TMP.upto = bitsof(uint_t);
	
	puts("Attempting comparison\n");
	
	/* alu_reg_cmp() can only return -1,0 or 1 for valid results, anything else
	 * is an error code from errno.h */
	cmp = alu_uint_cmp( alu, num, val );
	ck_assert( cmp == 1 );
	
	puts("Attempting bitwise math\n");
	
	temp = want ^ i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_xor( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want >> i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_shr( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want << i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_shl( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want | i;
	alu_uint_set_raw( alu, num, want );
	alu_uint__or( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want & i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_and( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	puts("Attempting normal math\n");
	
	temp = want + i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_add( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want - i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_sub( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	temp = want * i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_mul( alu, num, val );
	data = alu_data( alu, num );
	ck_assert_msg
	(
		memcmp( data, &temp, sizeof(uint_t) ) == 0
		, "%u * %u == %u, Got %u"
		, want
		, i
		, temp
		, *((uint_t*)data)
	);
	
	temp = want / i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_div( alu, num, val );
	data = alu_data( alu, num );
	ck_assert_msg
	(
		memcmp( data, &temp, sizeof(uint_t) ) == 0
		, "%u / %u == %u, Got %u"
		, want
		, i
		, temp
		, *((uint_t*)data)
	);
	
	temp = want % i;
	alu_uint_set_raw( alu, num, want );
	alu_uint_rem( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &temp, sizeof(uint_t) ) == 0 );
	
	puts("Attempting to print to string\n");
	
	alu_dst.dst = &ALUSTR;
	alu_dst.next = wrChar;
	alu_dst.flip = flip;
	
	base.base = 10;
	sprintf( stdstr, "%u", want );
	alu_uint_set_raw( alu, num, want );
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
