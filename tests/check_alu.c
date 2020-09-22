#include <check.h>
#include <alu.h>
#include <stdlib.h>

#define REG_COUNT 3

START_TEST( test_alu_create )
{
	int ret;
	alu_t _alu = {0}, *alu = &_alu;
	alu_uint_t num, val, tmp;
	void *data;
	uint_t i, want = 32, nodes[REG_COUNT] = {0};
	
	puts( "Initialising ALU" );
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
	
	puts( "Getting registers" );
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	for ( i = 0; i < REG_COUNT; ++i )
	{
		ck_assert( nodes[i] != 0 );
	}
	
	num = nodes[0];
	
	alu_uint_set_raw( alu, num, want );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &want, sizeof(uint_t) ) == 0 );
	
	val = nodes[1];
	
	alu_uint_set_raw( alu, val, i );
	data = alu_data( alu, val );
	ck_assert( memcmp( data, &i, sizeof(uint_t) ) == 0 );
	
	tmp = nodes[2];
	
	want <<= i;
	alu_uint_shl( alu, num, val );
	data = alu_data( alu, num );
	ck_assert( memcmp( data, &want, sizeof(uint_t) ) == 0 );
	
	alu_vec_release( alu, alu_size_perN(alu) );
}
END_TEST

/* Modified copy & paste of code given in check's guide */
Suite * alu_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Money");

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
	
	puts( "Running unit tests under 'check' test suite" );

	s = alu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
